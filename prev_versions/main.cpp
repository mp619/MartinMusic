#include <Arduino.h>
#include <U8g2lib.h>
#include <iostream>
#include <string>
using namespace std;

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

// key stepsize based on frequency:
// f_sample == 22k, S = 2^32* f/f_sample, A = 440Hz = 9th key
// 2^-12 = frequency diff between adjacent notes
//const int32_t stepSizes [11] = {...};

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
  delayMicroseconds(2);
  digitalWrite(REN_PIN, LOW);
}

// for now set row is as set out mux bit
void setRow(uint8_t rowIdx)
{
  digitalWrite(REN_PIN, LOW);
  digitalWrite(RA0_PIN, rowIdx & 0x01);
  digitalWrite(RA1_PIN, rowIdx & 0x02);
  digitalWrite(RA2_PIN, rowIdx & 0x04);
  digitalWrite(REN_PIN, HIGH);
}

//*************basic functions***********//
uint8_t readCols(int row)
{
  // set automatically what row enabled
  setRow(row);
  uint8_t value = 0X0F;
  // if key pressed, =0, therefore preset to 1 and and it to ge 0 when pressed
  // concatenation of the 4 bits into a byte (0000+4 vakues of the keys)
  value = value & (digitalRead(C3_PIN) * 8 | digitalRead(C2_PIN) * 4 | digitalRead(C1_PIN) * 2 | digitalRead(C0_PIN) * 1);
  return value;
}

void setup()
{
  // put your setup code here, to run once:

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

  // Initialise UART
  Serial.begin(9600);
  Serial.println("Hello World");
}

void loop()
{
  // put your main code here, to run repeatedly:
  static uint32_t next = millis();
  static uint32_t count = 0;
  // store reading of each row in matrix
  uint8_t keyArray[7];

  if (millis() > next)
  {
    next += interval;

    for (int i = 0; i < 3; i++)
    {
      setRow(i);
      delayMicroseconds(3);
      keyArray[i] = readCols(i);
    }

    //***DISPLAY***//
    // Update display
    u8g2.clearBuffer();                 // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    // u8g2.drawStr(2,10, value );  // write something to the internal memory
    u8g2.setCursor(2, 20);
    u8g2.print(keyArray[0], HEX);
    u8g2.setCursor(10, 20);
    u8g2.print(keyArray[1], HEX);
    u8g2.setCursor(18, 20);
    u8g2.print(keyArray[2], HEX);
    u8g2.sendBuffer(); // transfer internal memory to the display

    // Toggle LED
    digitalToggle(LED_BUILTIN);
  }
}
