#include <Arduino.h>
#include <U8g2lib.h>
#include <map>
#include <STM32FreeRTOS.h>
#include <ES_CAN.h>
#include <knobs.h>

// Constants
const uint32_t interval = 100; // Display update interval

// Pin definitions
// Row select and enable
const int RA0_PIN = D3;
const int RA1_PIN = D6;
const int RA2_PIN = D12;
const int REN_PIN = A5;

// Matrix input and output
const int C0_PIN = A2;
const int C1_PIN = D9;
const int C2_PIN = A6;
const int C3_PIN = D1;
const int OUT_PIN = D11;

// Audio analogue out
const int OUTL_PIN = A4;
const int OUTR_PIN = A3;

// Joystick analogue in
const int JOYY_PIN = A0;
const int JOYX_PIN = A1;

// Output multiplexer bits
const int DEN_BIT = 3;
const int DRST_BIT = 4;
const int HKOW_BIT = 5;
const int HKOE_BIT = 6;

// knobs
Knobs knob2(2);
Knobs knob3(3);
Knobs knob1(1);

// Phase step sizes
//const int32_t stepSizes[13] = {0, 51076057, 54113197, 57330936, 60740010,
//                               64351799, 68178356, 72232452, 76527617,
//                               81078186, 85899346, 91007187, 96418756};

const int16_t stepSizes[13] = {0, 779,	826,	875,	927,	982,	1040,	1102,	1168,	1237,	1311,	1389,	1471};

static int16_t phaseAcc[3] = {0};

const static char *NOTES[13] = {" ", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

// Current Step
volatile int16_t currentStepSize[3] = {0};

// keyMatrix
volatile uint8_t keyArray[7];
volatile int idxKey[3] = {0};

// Global Queue
QueueHandle_t msgInQ;
QueueHandle_t msgOutQ;

// Global RX Message
volatile uint8_t RX_Message[8];

// Global handle
SemaphoreHandle_t keyArrayMutex;
SemaphoreHandle_t keyidxMutex;
SemaphoreHandle_t RX_MessageMutex;
SemaphoreHandle_t CAN_TX_Semaphore;

// Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);

// Function to set outputs using key matrix
void setOutMuxBit(const uint8_t bitIdx, const bool value)
{
  digitalWrite(REN_PIN, LOW);
  digitalWrite(RA0_PIN, bitIdx & 0x01);
  digitalWrite(RA1_PIN, bitIdx & 0x02);
  digitalWrite(RA2_PIN, bitIdx & 0x04);
  digitalWrite(OUT_PIN, value);
  digitalWrite(REN_PIN, HIGH);
  delayMicroseconds(15);
  digitalWrite(REN_PIN, LOW);
}

// Read Matrix
uint8_t readCols(int row)
{
  uint8_t value = 0x0F;
  setOutMuxBit(row, true);
  value = value & (digitalRead(C3_PIN) * 8 | digitalRead(C2_PIN) * 4 | digitalRead(C1_PIN) * 2 | digitalRead(C0_PIN) * 1);
  return value;
}

void sampleISR()
{
  int32_t Total = 0;
  for(int i = 0; i < 3; i++){
    phaseAcc[i] += currentStepSize[i];
    Total += phaseAcc[i]; 
  }
  if (Total > 65536){
    Total = true;
  }
  int32_t Vout = Total >> 8;

  Vout = Vout >> (8 - knob3.get_count() / 2);
  analogWrite(OUTR_PIN, Vout + 128);
  //Serial.println(Vout + 128);
}

void CAN_RX_ISR(void)
{
  uint8_t RX_Message_ISR[8];
  uint32_t ID;
  CAN_RX(ID, RX_Message_ISR);
  xQueueSendFromISR(msgInQ, RX_Message_ISR, NULL);
}

void scanKeysTask(void *pvParameters)
{
  uint8_t TX_Message[8] = {0};
  const TickType_t xFrequency = 50 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (1)
  {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    // ReadCol
    setOutMuxBit(0x01, true);
    int16_t localCurrentStepSize[3] = {0};
    for(int i = 0; i < 3; i++){
      idxKey[i] = 0;
    }
    xSemaphoreGive(keyidxMutex);

    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

    uint8_t count = 0;
    for (int i = 0; i < 3; i++)
    {
      keyArray[i] = readCols(i);
      int j = 0;
      for (uint8_t idx = 1; idx < 9; idx = idx * 2)
      {
        if ((keyArray[i] & idx) == 0)
        {
          idxKey[count] = i * 4 + j + 1;
          count++;
          if(count > 2){
            count = 2;
          }
        }
        j++;
      }
    }
    for(int i = 0; i < 3; i++){
      localCurrentStepSize[i] = stepSizes[idxKey[i]];
    }

    // read volume knobs
    keyArray[3] = readCols(3);
    keyArray[4] = readCols(4);
    knob3.decodeKnob(keyArray[3]);
    knob2.decodeKnob(keyArray[3]);
    knob1.decodeKnob(keyArray[4]);

    char keyState;
    //uint8_t activeKey = idxKey[0];
    uint8_t octave = knob2.get_count();
    int shift = octave - 4;
/*     if (shift == 0)
    {
      localCurrentStepSize = localCurrentStepSize;
    }
    else if (shift > 0)
    {
      localCurrentStepSize = localCurrentStepSize << shift;
    }
    else if (shift < 0)
    {
      localCurrentStepSize = localCurrentStepSize >> -shift;
    } */

    //Serial.println(localCurrentStepSize);

/*     if (activeKey == 0)
    {
      keyState = 'R';
      activeKey = TX_Message[2];
      TX_Message[0] = keyState;
      TX_Message[1] = octave;
      TX_Message[2] = activeKey;
    }
    else
    {
      keyState = 'P';
      TX_Message[0] = keyState;
      TX_Message[1] = octave;
      TX_Message[2] = activeKey;
    } */

    // Sendign CAN frame
    xQueueSend(msgOutQ, TX_Message, portMAX_DELAY);

    xSemaphoreGive(keyArrayMutex);
    // Serial.println(keyArray[0]);
    for(int i = 0; i < 3; i++){
      __atomic_store_n(&currentStepSize[i], localCurrentStepSize[i], __ATOMIC_RELAXED);
    }
    keyArray[5] = readCols(5);
    keyArray[6] = readCols(6);
  }
}

void displayUpdateTask(void *pvParameters)
{  
  const TickType_t xFrequency = 100 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint8_t y_1 = 10;
  uint8_t y_2 = 20;
  uint8_t y_3 = 30;

  uint8_t north = 45;
  uint8_t east = 95;
  uint8_t southeast = 95;
  uint8_t west = 5;
  uint8_t southwest  = 5;
  uint8_t south = 58;
  uint8_t centre = 60;
  while (1)
  {
    
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
      //Make frame

    u8g2.clearBuffer();                 // clear the internal memory

    u8g2.setFont(u8g2_font_5x7_tr); // choose a suitable font
    //Get Centre
    u8g2.setCursor(north, y_1);
    u8g2.print("Receiver");

    //Octave
    u8g2.setCursor(west, y_2);
    u8g2.print("Oct: ");
    u8g2.print(knob2.get_count());

    // volume print
    u8g2.setCursor(southwest, y_3);
    u8g2.print("Vol: ");
    u8g2.print(knob3.get_count(), DEC);
    
    //Key print
    u8g2.setCursor(east, y_2);
    u8g2.print("Key: ");
    u8g2.print(idxKey[0]);

    u8g2.setCursor(centre, y_2);
    u8g2.print('-');
    u8g2.print('-');

    //Print Position
    u8g2.setCursor(southeast, y_3);
    u8g2.print("Pos: ");
    u8g2.print(0);

    int _wave = 2;
    //Wave print
    u8g2.setCursor(south, y_3);
    if (_wave == 0)
    {
        u8g2.print("Tri");
    }
    else if (_wave  == 1)
    {
        u8g2.print("Squ");
    }
    else if (_wave == 2)
    {
        u8g2.print("Saw");
    }
    else if (_wave > 2)
    {
        u8g2.print("Sine");
    }
    u8g2.setCursor(west, y_1);
    u8g2.print("Single");     
                  // clear the internal memory
    //u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font

    
    u8g2.drawFrame(2,1,126,31);
    u8g2.sendBuffer(); // transfer internal memory to the display     // Toggle LED   
    digitalToggle(LED_BUILTIN);


  }
}

void decodeTask(void *pvParameters)
{
  int32_t localCurrentStepSize = 0;
  while (true)
  {
    uint8_t RX_Message_local[8];
    xQueueReceive(msgInQ, RX_Message_local, portMAX_DELAY);
    if ((char)RX_Message_local[0] == 'P')
    {
      localCurrentStepSize = (stepSizes[RX_Message_local[2]]);
    }
    else if ((char)RX_Message_local[0] == 'R')
    {
      localCurrentStepSize = 0;
    }
    //Serial.println(localCurrentStepSize);
    xSemaphoreTake(RX_MessageMutex, portMAX_DELAY);
    for (int i = 0; i < 8; i++)
    {
      __atomic_store_n(&RX_Message[i], RX_Message_local[i], __ATOMIC_RELAXED);
    }
    xSemaphoreGive(RX_MessageMutex);
  }
}

void CAN_TX_ISR(void)
{
  xSemaphoreGiveFromISR(CAN_TX_Semaphore, NULL);
}

void CAN_TX_Task(void *pvParameters)
{
  uint8_t msgOut[8];
  while (1)
  {
    xQueueReceive(msgOutQ, msgOut, portMAX_DELAY);
    xSemaphoreTake(CAN_TX_Semaphore, portMAX_DELAY);
    CAN_TX(0x123, msgOut);
  }
}

void setup()
{
  // put your setup code here, to run once:

  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *sampleTimer = new HardwareTimer(Instance);
  sampleTimer->setOverflow(22000, HERTZ_FORMAT);
  sampleTimer->attachInterrupt(sampleISR);
  sampleTimer->resume();

  msgInQ = xQueueCreate(36, 8);
  msgOutQ = xQueueCreate(36, 8);

  // Initialise CAN Hardware
  CAN_Init(true); // CAN_Init(true);
  setCANFilter(0x123, 0x7ff);
  CAN_RegisterRX_ISR(CAN_RX_ISR);
  CAN_RegisterTX_ISR(CAN_TX_ISR);
  CAN_Start();

  // Set pin directions
  pinMode(RA0_PIN, OUTPUT);
  pinMode(RA1_PIN, OUTPUT);
  pinMode(RA2_PIN, OUTPUT);
  pinMode(REN_PIN, OUTPUT);
  pinMode(OUT_PIN, OUTPUT);
  pinMode(OUTL_PIN, OUTPUT);
  pinMode(OUTR_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(C0_PIN, INPUT);
  pinMode(C1_PIN, INPUT);
  pinMode(C2_PIN, INPUT);
  pinMode(C3_PIN, INPUT);
  pinMode(JOYX_PIN, INPUT);
  pinMode(JOYY_PIN, INPUT);

  // Initialise display
  setOutMuxBit(DRST_BIT, LOW); // Assert display logic reset
  delayMicroseconds(2);
  setOutMuxBit(DRST_BIT, HIGH); // Release display logic reset
  u8g2.begin();
  setOutMuxBit(DEN_BIT, HIGH); // Enable display power supply

  // Threading
  TaskHandle_t scanKeysHandle = NULL;
  xTaskCreate(scanKeysTask,     /* Function that implements the task */
              "scanKeys",       /* Text name for the task */
              64,               /* Stack size in words, not bytes */
              NULL,             /* Parameter passed into the task */
              2,                /* Task priority */
              &scanKeysHandle); /* Pointer to store the task handle */

  TaskHandle_t displayUpdateHandle = NULL;
  xTaskCreate(displayUpdateTask,     /* Function that implements the task */
              "displayUpdate",       /* Text name for the task */
              256,                   /* Stack size in words, not bytes */
              NULL,                  /* Parameter passed into the task */
              1,                     /* Task priority */
              &displayUpdateHandle); /* Pointer to store the task handle */

  TaskHandle_t decodeHandle = NULL;
  xTaskCreate(decodeTask,   /* Function that implements the task */
              "decodeTask", /* Text name for the task */
              32,           /* Stack size in words, not bytes */
              NULL,         /* Parameter passed into the task */
              4,            /* Task priority */
              &decodeHandle);

  TaskHandle_t CAN_TX_Handle = NULL;
  xTaskCreate(CAN_TX_Task, /* Function that implements the task */
              "CAN_TX",    /* Text name for the task */
              32,          /* Stack size in words, not bytes */
              NULL,        /* Parameter passed into the task */
              3,           /* Task priority */
              &CAN_TX_Handle);

  // Initialise UART
  Serial.begin(9600);
  Serial.println("Hello World");

  keyArrayMutex = xSemaphoreCreateMutex();
  keyidxMutex = xSemaphoreCreateMutex();
  RX_MessageMutex = xSemaphoreCreateMutex();
  CAN_TX_Semaphore = xSemaphoreCreateCounting(3, 3);

  vTaskStartScheduler();
}

void loop()
{
}