//  UV Watch V1
//
//  CPU is ATtiny1614: https://ww1.microchip.com/downloads/en/DeviceDoc/ATtiny1614-16-17-DataSheet-DS40002204A.pdf
//  UV Sensors is GUVA-S12SD: http://www.geni-uv.com/download/products/GUVA-S12SD.pdf
//  Alt. #1 UV Sensor is VEML-6075: https://media.digikey.com/pdf/Data%20Sheets/Vishay%20Semiconductors/VEML6075.pdf
//  Alt. #2 UV Sensor is VEML-6070: https://cdn-learn.adafruit.com/assets/assets/000/032/482/original/veml6070.pdf
//  Accelerometer is MXC4005XC: https://media.digikey.com/pdf/Data%20Sheets/MEMSIC%20PDFs/MXC400xXC_Rev.B_4-24-15.pdf
//

// Arduino config:
//
// Uses this : https://github.com/SpenceKonde/megaTinyCore
// Installation: https://github.com/SpenceKonde/megaTinyCore/blob/master/Installation.md
//
// After install:
//
// Tools -> Board: ATtiny3224/1624/1614/...... (UV uses 1614)
//          Chip:  Attiny1614
//          Clock: 16 MHz internal
//          .....
//          Startup Time: 64ms
//          .....
//          Programmer: SerialUPDI - 230400 baud
//
// Uses TinyI2C Library:
//
//  From here: https://github.com/technoblogy/tiny-i2c
//  Download .ZIP and use the Sketch->Include Library->Add .ZIP Library

#include <tinyNeoPixel.h>
#include "TinyI2CMaster.h"

// GUVA-S12SD sensor attached to this pin - voltage is 4.3 x diode photocurrent
// Adafruit says voltage/0.1V = UV index

#define UV_PIN    PIN_PA2

// Which pin on the Arduino is connected to the NeoPixels?
#define NEO_PIN   PIN_PA3

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      12

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
tinyNeoPixel pixels = tinyNeoPixel(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);

#define I2C_WRITE 0

int delayval = 100;

extern int8_t accelInit();
extern void accelRead();
extern bool orientDisplay();

// Accel values
uint16_t aOrient;
uint8_t  gotAccel = 0;
uint16_t aShake;
uint16_t notMoving = 0;
uint8_t lefthanded = 0;

// Globals
#define ON  255
uint8_t red = 0;
uint8_t green = 0;
uint8_t blue = ON;

uint32_t counts;
uint32_t voltage;
uint8_t nled;

// From here (and others): https://arduino.steamedu123.com/entry/GUVA-S12SD-Ultraviolet-Light-Sensor
uint32_t UVindex[] = { 50, 227, 318, 408, 503, 606, 696, 795, 881, 976, 1079, 1170, 0 };


void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.println("UV Watch");
  Serial.println();
  
  TinyI2C.init();     // Not in the library desc.!! Only in examples...
  pixels.begin();     // Initializes tinyNeoPixel library.
  pixels.setBrightness(25);
  gotAccel = accelInit();
}

void loop() {

  // Read the UV level and scale it
  // 1024 counts = 3300 mV
  // 1 count = 3300 / 1024 mV
  // Multiply counts by (3300 / 1024) to get voltage in mV
  
  counts = analogRead(UV_PIN);  // 1024 counts = 3300 mV (3.3V)
  voltage = counts * 3300;
  voltage /= 1024;              // 0 .. 3300

  // Look up the UV index
  for(nled = 0; voltage > UVindex[nled]; nled++) {
    if (nled >= NUMPIXELS) break;
  }
  if (nled >= NUMPIXELS) nled = 11;

  // Show the UV level
  pixels.setPixelColor(nled, pixels.Color(red,green,blue)); 
  pixels.show();
  delay(100);
  Serial.println(lefthanded);

  // Clear all the LEDs for next measurement
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor( i, pixels.Color(0, 0, 0) );
    delay(1);   // MUST HAVE A SMALL DELAY WITH LOOPED NEOPIXEL COMMANDS !!!???
  }
  pixels.show();

  // Change the LED color depending on orientation
  orientDisplay();
  if(lefthanded) {
    green = 0;
    red = ON;
    blue = 0;
  }
  else {
    red = 0;
    green = ON;
    blue = 0;
  }
}
