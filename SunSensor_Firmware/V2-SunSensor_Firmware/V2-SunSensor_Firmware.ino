/*
Lolin WEMOS d1 mini clone
*/
//y = -267.48x + 3913.6
//Given "x" UV level, "y" returns seconds needed to burn
//We measure time spent in "x" and calc a "% burned"
//(Time Spent at UV level)/(-267.48*(UV level)+3913.6) + ((Time Spent at UV level)/(-267.48*(UV level)+3913.6))*ginger index
//Increase to burn faster, decrease to be a beautiful tan person
//  -     = less ginger
//  1     =  ginger
//  +     =  more ginger
//default is 1
float GingerIndex = 10;
//Example: If you burn in 10 minutes SPF 30 will give you 300 minutes of burning protection from UVB rays.
//Input SPF used
//default is 30
float SPFIndex = 30;
//How long you expect your sunscreen to last in seconds
//default is 3600
int SunScreenDurationSeconds = 10;
//How fast you want to recover from sunburn
//Percent decay per second, as a decimal
//Default = .01
float BurnDecay = 1;
//we assume the person isn't wearing sunscreen (BOOOOO!)
bool SunScreenApplied = false;
//Ticks down for every second spent in the sun
int SunScreenTTBTimer = SunScreenDurationSeconds;

//Variables to help calculate
float PercentBurned = 0;
float SecondsToBurn = 0;
float PercentAddedToBurn = 0;

//Average UV stuffs.
const int UVnumReadings = 10;
float UVreadings[UVnumReadings];
int UVreadIndex = 0;
float UVtotal = 0;
float UVaverage = 0;
int SecondsInSun = 0;

//UV sensor control
#include <Wire.h>
#include "Adafruit_VEML6075.h"
Adafruit_VEML6075 uv = Adafruit_VEML6075();
float CurrentReading = 0;

//NeoPixel control
#include <Adafruit_NeoPixel.h>
#define PIN D4
#define NUMPIXELS 12
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
int LEDBrigthness = 5;
int RedLEDTimer = 0;
int GreenLEDTimer = 0;
float TotalLEDs = 12;

//Timing control
int MeasurementTimer = 0;
int DisplayTimer = 0;

void setup() {
  Serial.begin(115200);
  pixels.begin();
  pixels.setBrightness(LEDBrigthness);
  pixels.show();
  pixels.show();
  delay(100);
  pixels.clear();
  if (!uv.begin()) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 50));
    pixels.setPixelColor(3, pixels.Color(0, 0, 50));
    pixels.setPixelColor(6, pixels.Color(0, 0, 50));
    pixels.setPixelColor(9, pixels.Color(0, 0, 50));
    Serial.println("Failed to communicate with VEML6075 sensor, check wiring?");
    while (1) {
      delay(5000);
    }
  }
  Serial.println("-----------------------------------------");
  Serial.println("Found VEML6075 sensor");
  Serial.println("Program Begin");
  Serial.println("-----------------------------------------");
  pixels.setPixelColor(0, pixels.Color(0, 50, 0));
  pixels.setPixelColor(3, pixels.Color(0, 50, 0));
  pixels.setPixelColor(6, pixels.Color(0, 50, 0));
  pixels.setPixelColor(9, pixels.Color(0, 50, 0));
  pixels.show();
  delay(3000);
  pixels.clear();
  pixels.show();
}

void loop() {
  if (Serial.available()) {
    Serial.read();
    SunScreenApplied = true;
    SunScreenTTBTimer = SunScreenDurationSeconds;
  }
  MeasurementTimer++;
  DisplayTimer++;

  //-----------------Avg measurement control------------------
  //Every 10th of a second
  if (MeasurementTimer == 10) {
    CurrentReading = uv.readUVI();
    if (CurrentReading > 12) {
      CurrentReading = 12;
    }
    if (CurrentReading < 0.2) {
      CurrentReading = 0;
    }
    UVtotal = UVtotal - UVreadings[UVreadIndex];
    UVreadings[UVreadIndex] = CurrentReading;
    UVtotal = UVtotal + UVreadings[UVreadIndex];
    UVreadIndex++;
    if (UVreadIndex >= UVnumReadings) {
      UVreadIndex = 0;
    }
    MeasurementTimer = 0;
  }
  //-----------------Display control (LEDs)------------------
  //Every second
  if (DisplayTimer == 100) {
    if (SunScreenApplied == true) {
      SunScreenTTBTimer--;
      if (SunScreenTTBTimer == 0) {
        SunScreenApplied = false;
      }
    }
    UVaverage = UVtotal / UVnumReadings;
    if (UVaverage < 0.1) {
      //If we see no UV
      UVaverage = 0;
      if (PercentBurned > 0) {
        //Decay percent burned by .1%/Second when no UV
        PercentBurned = PercentBurned - BurnDecay;
      } else {
        //When back to zero, reset all. No burn.
        SecondsInSun = 0;
        pixels.clear();
      }
    } else {
      //If we see UV, start counting time in sun
      SecondsInSun++;
      //If sunscreen applied. Start counting down and dividing time to burn by SPFIndex
      if (SunScreenApplied == true) {
        //Math with sunscreen
        SecondsToBurn = (-267.48 * UVaverage + 3913.6) * SPFIndex;
      } else {
        //Math without sunscreen
        SecondsToBurn = -267.48 * UVaverage + 3913.6;
      }
      //Building the new "% burned".
      PercentAddedToBurn = (1 / SecondsToBurn) * 100;
      PercentBurned = PercentBurned + (PercentAddedToBurn * GingerIndex);
    }
    //Value catching. Dealing with float. Makes 100+ = 100
    if (PercentBurned > 100) {
      PercentBurned = 100;
    }
    //Value catching. Dealing with float. Makes 0- = 0
    if (PercentBurned < 0) {
      PercentBurned = 0;
    }
    //Testing/monitoring
    Serial.print("Average UV - ");
    Serial.println(UVaverage);
    Serial.print("% burned - ");
    Serial.println(PercentBurned);
    Serial.print("Seconds in sun - ");
    Serial.println(SecondsInSun);
    if (CurrentReading != 0) {
      Serial.print("Time to Burn (min)- ");
      Serial.println(((100 / PercentBurned) * SecondsInSun) / 60);
    }
    Serial.print("Sunscreen applied? - ");
    Serial.println(SunScreenApplied ? "Applied" : "None");
    if (SunScreenApplied) {
      Serial.print("Sunscreen left - ");
      Serial.println(SunScreenTTBTimer);
    }
    Serial.println("----------------");
    //Checking to make sure we aren't burnt!
    if (PercentBurned < 100) {
      /*FIll ring. Showing how burnt
      for (float PixelLocation = 0; PixelLocation < (TotalLEDs * (PercentBurned / 100)); PixelLocation++) {
        RedLEDTimer = 255 * (PixelLocation / TotalLEDs);
        GreenLEDTimer = 255 - RedLEDTimer;
        pixels.setPixelColor(PixelLocation, pixels.Color(RedLEDTimer, GreenLEDTimer, 0));
        pixels.show();
      }
      for (float PixelLocation = TotalLEDs; PixelLocation >= (TotalLEDs * (PercentBurned / 100)); PixelLocation--) {
        pixels.setPixelColor(PixelLocation, pixels.Color(0, 0, 0));
        pixels.show();
      }
      */
      //One LED at a time. Showing how burnt
      for (float PixelLocation = 0; PixelLocation < TotalLEDs; PixelLocation++) {
        if (PixelLocation == int(TotalLEDs * (PercentBurned / 100)) && PercentBurned > 0) {
          RedLEDTimer = 255 * (PixelLocation / TotalLEDs);
          GreenLEDTimer = 255 - RedLEDTimer;
          pixels.setPixelColor(PixelLocation, pixels.Color(RedLEDTimer, GreenLEDTimer, 0));
          if (SunScreenApplied) {
            PixelLocation++;
            pixels.setPixelColor(PixelLocation, pixels.Color(0, 0, 255));
          }
          pixels.show();
        } else {
          pixels.setPixelColor(PixelLocation, pixels.Color(0, 0, 0));
          pixels.show();
        }
      }
      //If we are burnt, we also burn our retinas by flashing bright, bright red
    } else {
      pixels.setBrightness(255);
      for (int PixelLocation = 0; PixelLocation < TotalLEDs; PixelLocation++) {
        pixels.setPixelColor(PixelLocation, pixels.Color(255, 0, 0));
        pixels.show();
      }
      pixels.setBrightness(LEDBrigthness);
      pixels.show();
    }
    DisplayTimer = 0;
  }
  delay(10);
}