#include <ESP8266WiFi.h>

#include <Wire.h>
#include "Adafruit_VEML6075.h"
Adafruit_VEML6075 uv = Adafruit_VEML6075();

#include <Adafruit_NeoPixel.h>
#define PIN D4
#define NUMPIXELS 12
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int LEDBrightnessMax = 25;
float PixelLocation = 0;

float TotalLEDs = 12;

int MeasurementTimer = 0;
int DisplayTimer = 0;
float CurrentReading = 0;

const int UVnumReadings = 10;
float UVreadings[UVnumReadings];  // the readings from the analog input
int UVreadIndex = 0;              // the index of the current reading
float UVtotal = 0;                // the running total
float UVaverage = 0;              // the average

float PercentBurned = 0;

void setup() {
  WiFi.forceSleepBegin();
  delay(100);
  Serial.begin(115200);
  pixels.begin();
  delay(100);
  pixels.clear();
  if (!uv.begin()) {
    pixels.setPixelColor(1, pixels.Color(0, 0, 50));
    Serial.println("Failed to communicate with VEML6075 sensor, check wiring?");
    while (1) {
      delay(100);
    }
  }
  Serial.println("Found VEML6075 sensor");
  Serial.println("Program Begin");
  pixels.setPixelColor(1, pixels.Color(0, 50, 0));
  pixels.show();
  delay(1000);
}

void loop() {
  MeasurementTimer++;
  DisplayTimer++;

  if (MeasurementTimer == 10) {
    CurrentReading = uv.readUVI();
    UVtotal = UVtotal - UVreadings[UVreadIndex];
    UVreadings[UVreadIndex] = CurrentReading;
    UVtotal = UVtotal + UVreadings[UVreadIndex];
    UVreadIndex++;
    MeasurementTimer = 0;
    if (UVreadIndex >= UVnumReadings) {
      UVreadIndex = 0;
    }
  }

  if (DisplayTimer == 100) {
    UVaverage = UVtotal / UVnumReadings;
    //y = -267.48x + 3913.6
    //Given "x" UV level, "y" returns seconds needed to burn
    //We measure time spent in "x" and calc a "% burned"
    //(Time Spent at UV level)/(-267.48*(UV level)+3913.6)
    PercentBurned = PercentBurned + (1) / (-267.48 * (UVaverage) + 3913.6);

    Serial.print("Actual - ");
    Serial.println(CurrentReading);
    Serial.print("Average - ");
    Serial.println(UVaverage);
    Serial.print("% burned - ");
    Serial.println(PercentBurned);
    Serial.println("----------------");

    /*
    for (; PixelLocation < MaxUV; PixelLocation++) {
      RedLEDTimer = LEDBrightnessMax + (LEDBrightnessMax * (PixelLocation / TotalLEDs));
      GreenLEDTimer = LEDBrightnessMax - (LEDBrightnessMax * (PixelLocation / TotalLEDs));
      pixels.setPixelColor(PixelLocation, pixels.Color(RedLEDTimer , GreenLEDTimer, 0));
      pixels.show(); 
    }
    */
    DisplayTimer = 0;
  }
  delay(10);
}