#include <ESP8266WiFi.h>

#include <Wire.h>
#include "Adafruit_VEML6075.h"
Adafruit_VEML6075 uv = Adafruit_VEML6075();

#include <Adafruit_NeoPixel.h>
#define PIN        D4
#define NUMPIXELS 12
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int LEDBrightnessMax = 50
;
float PixelLocation = 0;

int RedLEDTimer = 0;
int GreenLEDTimer = 0;
float TotalLEDs = 12;

int MeasurementTimer = 0;
int CurrentReadingInt = 0;
int MaxUV = 0;
//In S = Decay
//(5 second delay * 12 segements) = 60 seconds to decay (1 min)
// ^^^^^^^^^^^^^ Multiply by "X" to get minute values
//(75 second delay * 12 segements) = 900 seconds to decay (15 min)
int MaxUVTimerLimit = 50;
int MaxUVTimer = 0;

void setup() {
  WiFi.forceSleepBegin();
  delay(100);
  Serial.begin(115200);
  pixels.begin();
  delay(100);
  pixels.clear();
  if (! uv.begin()) {
    pixels.setPixelColor(1, pixels.Color(0 , 0, 50));
    Serial.println("Failed to communicate with VEML6075 sensor, check wiring?");
    while (1) {
      delay(100);
    }
  }
  Serial.println("Found VEML6075 sensor");
  Serial.println("Program Begin");
  pixels.setPixelColor(1, pixels.Color(0 , 50, 0));
  delay(1000);
}

void loop() {
  MeasurementTimer++;
  CurrentReadingInt = uv.readUVI() + 0.5;
  if (CurrentReadingInt > MaxUV) {
    MaxUV = CurrentReadingInt;
    if (MaxUV > 12) {
      MaxUV = 12;
    }
  }
  if (MeasurementTimer == 100) {
    Serial.print("Actual - "); Serial.println(uv.readUVI());
    Serial.print("Actual Int - "); Serial.println(CurrentReadingInt);
    Serial.print("Max - "); Serial.println(MaxUV);
    Serial.println("----------------");
    if (MaxUV < 1) {
      pixels.clear();
    }
    for (; PixelLocation < MaxUV; PixelLocation++) {
      RedLEDTimer = LEDBrightnessMax + (LEDBrightnessMax * (PixelLocation / TotalLEDs));
      GreenLEDTimer = LEDBrightnessMax - (LEDBrightnessMax * (PixelLocation / TotalLEDs));
      pixels.setPixelColor(PixelLocation, pixels.Color(RedLEDTimer , GreenLEDTimer, 0));
      pixels.show(); 
    }
    for (; PixelLocation >= MaxUV; PixelLocation--) {
      pixels.setPixelColor(PixelLocation, pixels.Color(0, 0, 0));
      pixels.show();
    }
    MaxUVTimer++;
    if (MaxUVTimer == MaxUVTimerLimit) {
      MaxUV--;
      MaxUVTimer = 0;
    }
    MeasurementTimer = 0;
  }
  delay(10);
}
