// Accel Test

// Accelerometer routines (in separate tab)

//bool accelBegin(bool shakeMode, uint8_t shakeThreshold, uint8_t shakeCount, uint8_t orientationCount);
//int accelX();         Get raw X value
//int accelY();         Get raw Y value
//int accelZ();         Get raw Z value
//uint8_t accelXYZT();  Get orientation (directions and signs 

#include <tinyNeoPixel.h>
#include "TinyI2CMaster.h"

// Which pin on the Arduino is connected to the NeoPixels?
#define NEO_PIN   PIN_PA3

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      12
tinyNeoPixel pixels = tinyNeoPixel(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);

#define I2C_WRITE 0

// Deprecated (will be going away but still working)
extern void accelRead();
extern bool orientDisplay();
extern void getShake();
// Deprecated Accel global values (will be going away but still working)
uint16_t aOrient;
uint16_t aShake;
uint16_t notMoving = 0;

// Globals

uint8_t  gotAccel = 0;
uint8_t lefthanded = 0;

#define ON  255
uint8_t red = 0;
uint8_t green = 0;
uint8_t blue = ON;

uint8_t nled;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println("UV Watch");
  Serial.println();
  
  TinyI2C.init();     // Not in the library desc.!! Only in examples...
  pixels.begin();     // Initializes tinyNeoPixel library.
  pixels.setBrightness(25);
  
  // Don't worry about initialization values now (don't affect basic operation)
  // bool shakeMode, uint8_t shakeThreshold, uint8_t shakeCount, uint8_t orientationCount
  gotAccel = accelBegin(0,0,0,0);
  
  if (gotAccel) Serial.println("MXC4005 found...");
  else Serial.println("MXC4005 not found...");
  Serial.println();
}

void loop() {

#define Xbit  0b00001000
#define Ybit  0b00000100
#define Zbit  0b00000010
#define Tbit  0b00000001

  uint8_t xyzt = accelXYZT();
   
  Serial.print("X:");
  Serial.print(accelX());
  Serial.print(",");
  Serial.print("Y:");
  Serial.print(accelY());
  Serial.print(",");
  Serial.print("Z:");
  Serial.println(accelZ());

  nled = 6;
  pixels.setPixelColor(nled, pixels.Color(red,green,blue)); 
  pixels.show();
  delay(100);

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
