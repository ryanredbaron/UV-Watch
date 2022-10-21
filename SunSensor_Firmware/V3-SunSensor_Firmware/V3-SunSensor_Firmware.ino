#include <tinyNeoPixel.h>
#include "TinyI2CMaster.h"

// GUVA-S12SD sensor attached to this pin - voltage is 4.3 x diode photocurrent
// Adafruit says voltage/0.1V = UV index
#define UV_PIN PIN_PA2

// NeoPixel Pin
#define NEO_PIN PIN_PA3

//Count of NeoPixels
#define NUMPIXELS 12

//Setup Neopixels
tinyNeoPixel pixels = tinyNeoPixel(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);
#define I2C_WRITE 0

//MS to loop for delay.
//**Do not edit (dire consequences)**
int LoopDelay = 10;

//Initialize accel, setting up
extern int8_t accelInit();
//Run in accel.ino, not used here
extern void accelRead();
//changes global variable lefthanded BOOL (int) 0-1
extern bool orientDisplay();

// Accel values
uint16_t aOrient;
//Check if accel is available
uint8_t gotAccel = 0;
uint16_t aShake;
uint16_t notMoving = 0;
uint8_t lefthanded = 0;

// Globals
#define ON 255
uint8_t red = 0;
uint8_t green = 0;
uint8_t blue = ON;

float counts;
float voltage;
uint8_t nled;

float UVIndex = 0;

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.println("UV Watch");
  Serial.println();

  TinyI2C.init();
  // Initializes tinyNeoPixel library.
  pixels.begin();
  pixels.setBrightness(10);
  gotAccel = accelInit();
}

int SensorCaptureTimer = 0;
int SensorCaptureTrigger = 10;

int DisplayTimer = 0;
int DisplayTimerTrigger = 100;

void loop() {
  SensorCaptureTimer++;
  DisplayTimer++;

  //-----------------------------SENSOR CAPTURE---------------------------------------
  if (SensorCaptureTimer >= SensorCaptureTrigger) {
    counts = analogRead(UV_PIN);
    voltage = counts * 3300;
    voltage /= 1024;
    UVIndex = ((voltage - 104) / 98);
    if (UVIndex < 0) {
      UVIndex = 0;
    }
    if (UVIndex > 11) {
      UVIndex = 12;
    }
    SensorCaptureTimer = 0;
  }
  //--------------------------------------------------------------------

  //-------------------------------DISPLAY-------------------------------------
  if (DisplayTimer >= DisplayTimerTrigger) {
    //----Orientation----
    orientDisplay();
    if (lefthanded) {
      green = 0;
      red = ON;
      blue = 0;
    } else {
      red = 0;
      green = ON;
      blue = 0;
    }

    //----Serial Prints----
    Serial.println("");
    Serial.println("----------");
    Serial.print("Voltage ");
    Serial.println(voltage);
    Serial.println("");
    Serial.print("UV ");
    Serial.println(UVIndex);
    Serial.println("----------");
    Serial.println("");

    //----Neopixels----
    for (int i = 0; i < NUMPIXELS; i++) {
      if (int(UVIndex) == i) {
        pixels.setPixelColor(i, pixels.Color(red, green, blue));
      } else {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      }
      delay(1);
    }
    pixels.show();

    DisplayTimer = 0;
  }
  //--------------------------------------------------------------------

  delay(LoopDelay);
}