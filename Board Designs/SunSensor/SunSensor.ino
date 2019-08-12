#include <ESP8266WiFi.h>

#include <Wire.h>
#include "Adafruit_VEML6075.h"
Adafruit_VEML6075 uv = Adafruit_VEML6075();

#include <Adafruit_NeoPixel.h>
#define PIN        D4
#define NUMPIXELS 12
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const int numReadings = 12.;
int readings[numReadings];
int readIndex = 0;
int total = 0;
int average = 0;

int PixelLocation = 0;

int ModUvIndex = 0;

int MeasurementTimer = 0;

void setup() {
  WiFi.forceSleepBegin();
  delay(100);
  Serial.begin(115200);
  pixels.begin();
  delay(100);
  pixels.clear();
  if (! uv.begin()) {
    pixels.setPixelColor(1, pixels.Color(200 , 0, 0));
    Serial.println("Failed to communicate with VEML6075 sensor, check wiring?");
    while (1) {
      delay(100);
    }
  }
  Serial.println("Found VEML6075 sensor");
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
  Serial.println("Program Begin");
  pixels.setPixelColor(1, pixels.Color(0 , 200, 0));
  delay(1000);
}

void loop() {
  MeasurementTimer++;
  if(MeasurementTimer == 1000){
    if (average = 0) {
      pixels.clear();
    }
    ModUvIndex = (uv.readUVI());
    total = total - readings[readIndex];
    readings[readIndex] = ModUvIndex;
    total = total + readings[readIndex];
    readIndex = readIndex + 1;
    if (readIndex >= numReadings) {
      readIndex = 0;
    }
    average = total / numReadings;
    Serial.print("Avg - "); Serial.println(average);
    Serial.print("Actual - "); Serial.println(uv.readUVI());
    Serial.print("Mod - "); Serial.println(ModUvIndex);
    for (int i = 0; i < numReadings; i++) {
      Serial.print(readings[i]); Serial.print(" - ");
    }
    Serial.println("");
    Serial.println("- ---------------");
    for (; PixelLocation < average; PixelLocation++) {
      pixels.setPixelColor(PixelLocation, pixels.Color(200 , 0, 0));
      pixels.show();
    }
    for (; PixelLocation >= average; PixelLocation--) {
      pixels.setPixelColor(PixelLocation, pixels.Color(0, 0, 0));
      pixels.show();
    }
    MeasurementTimer = 0;
  }
  delay(1);
}
