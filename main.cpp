#include <Arduino.h>
#include <U8g2lib.h>
#include <map>
#include <STM32FreeRTOS.h>

//Constants
  const uint32_t interval = 100; //Display update interval

//Pin definitions
  //Row select and enable
  const int RA0_PIN = D3;
  const int RA1_PIN = D6;
  const int RA2_PIN = D12;
  const int REN_PIN = A5;

  //Matrix input and output
  const int C0_PIN = A2;
  const int C1_PIN = D9;
  const int C2_PIN = A6;
  const int C3_PIN = D1;
  const int OUT_PIN = D11;

  //Audio analogue out
  const int OUTL_PIN = A4;
  const int OUTR_PIN = A3;

  //Joystick analogue in
  const int JOYY_PIN = A0;
  const int JOYX_PIN = A1;

  //Output multiplexer bits
  const int DEN_BIT = 3;
  const int DRST_BIT = 4;
  const int HKOW_BIT = 5;
  const int HKOE_BIT = 6;

  // Phase step sizes
  const int32_t stepSizes [3][4] = {{51076057,	54113197,	57330936,	60740010},
                                    {64351799,	68178356,	72232452,	76527617},
                                    {81078186,	85899346,	91007187,	96418756}};

 //Key struct
  struct Key{
    char letter;
    bool hash;
  };

  static int32_t phaseAcc = 0;

  //Key map
  std::map<volatile int,Key> mapofKeys = {
    {0, {'N', false}},
    {1, {'C',false}},
    {2, {'C',true}},
    {3, {'D',false}},
    {4, {'D',true}},
    {5, {'E',false}},
    {6, {'F',false}},
    {7, {'F',true}},
    {8, {'G',false}},
    {9, {'G',true}},
    {10, {'A',false}},
    {11, {'A',true}},
    {12, {'B',false}}
  };

  // Current Step
  volatile int32_t currentStepSize = 0;

  //keyMatric
  volatile uint8_t keyArray[7];
  volatile int idxKey;

//Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);

//Function to set outputs using key matrix
void setOutMuxBit(const uint8_t bitIdx, const bool value) {
      digitalWrite(REN_PIN,LOW);
      digitalWrite(RA0_PIN, bitIdx & 0x01);
      digitalWrite(RA1_PIN, bitIdx & 0x02);
      digitalWrite(RA2_PIN, bitIdx & 0x04);
      digitalWrite(OUT_PIN,value);
      digitalWrite(REN_PIN,HIGH);
      delayMicroseconds(2);
      digitalWrite(REN_PIN,LOW);
}

//Read Matrix
uint8_t readCols(int row){
  uint8_t value = 0x0F;
  setOutMuxBit(row, true);
  value = value & (digitalRead(C3_PIN)*8 | digitalRead(C2_PIN)*4 | digitalRead(C1_PIN)*2 | digitalRead(C0_PIN)*1);
  return value;
}

void sampleISR(){
  phaseAcc += currentStepSize;
  int32_t Vout = phaseAcc >> 24;
  analogWrite(OUTR_PIN, Vout + 128);
  //Serial.println(Vout);
}

void scanKeysTask(void * pvParameters){
  const TickType_t xFrequency = 50/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (1){
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    //ReadCol
    setOutMuxBit(0x01, true);
    int32_t localCurrentStepSize = 0;
    idxKey = 0;
    for(int i = 0; i < 3; i++){
      keyArray[i] = readCols(i);
      int j = 0;
      for(uint8_t idx = 1; idx < 9; idx = idx*2){
        if((keyArray[i] & idx) == 0){
          localCurrentStepSize =  stepSizes[i][j];
          idxKey = i*4 + j + 1;
        }
        j++;
      }
    }
    //Serial.println(keyArray[0]);
    __atomic_store_n(&currentStepSize, localCurrentStepSize, __ATOMIC_RELAXED);
    //vTaskDelay(1);
  }
}

void displayUpdateTask(void * pvParameters){
  const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (1){
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    //Serial.print("Hello");
    //Update display
    //Serial.println(keyArray[0]);
    u8g2.clearBuffer();         // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    u8g2.setCursor(2,10);
    u8g2.print(keyArray[0],HEX);
    u8g2.setCursor(10,10);
    u8g2.print(keyArray[1],HEX);
    u8g2.setCursor(18,10);
    u8g2.print(keyArray[2],HEX);
    u8g2.setCursor(2,20);

    //Print key press
    if(mapofKeys.find(idxKey)->second.letter != 'N'){
      u8g2.print(mapofKeys.find(idxKey)->second.letter);
      u8g2.setCursor(10,20);
      if(mapofKeys.find(idxKey)->second.hash){
        u8g2.print('#');
      }   
    }
    u8g2.sendBuffer();          // transfer internal memory to the display
    //Toggle LED
    digitalToggle(LED_BUILTIN);
    //vTaskDelay(1);
  }
}

void setup() {
  // put your setup code here, to run once:

  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *sampleTimer = new HardwareTimer(Instance);
  sampleTimer->setOverflow(22000, HERTZ_FORMAT);
  sampleTimer->attachInterrupt(sampleISR);
  sampleTimer->resume();

  //Set pin directions
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

  //Initialise display
  setOutMuxBit(DRST_BIT, LOW);  //Assert display logic reset
  delayMicroseconds(2);
  setOutMuxBit(DRST_BIT, HIGH);  //Release display logic reset
  u8g2.begin();
  setOutMuxBit(DEN_BIT, HIGH);  //Enable display power supply

  //Threading
  TaskHandle_t scanKeysHandle = NULL;
  xTaskCreate(scanKeysTask, /* Function that implements the task */
              "scanKeys", /* Text name for the task */
              64, /* Stack size in words, not bytes */
              NULL, /* Parameter passed into the task */
              2, /* Task priority */
              &scanKeysHandle ); /* Pointer to store the task handle */
  
  TaskHandle_t displayUpdateHandle = NULL;
  xTaskCreate(displayUpdateTask, /* Function that implements the task */
            "displayUpdate", /* Text name for the task */
            256, /* Stack size in words, not bytes */
            NULL, /* Parameter passed into the task */
            1, /* Task priority */
            &displayUpdateHandle ); /* Pointer to store the task handle */

  //Initialise UART
  Serial.begin(9600);
  Serial.println("Hello World");
  vTaskStartScheduler();
}

void loop() {

}