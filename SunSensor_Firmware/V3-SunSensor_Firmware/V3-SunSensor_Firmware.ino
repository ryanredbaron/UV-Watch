#include <tinyNeoPixel.h>
#include "TinyI2CMaster.h"

//------------------RH Variables-------------
//y = -267.48x + 3913.6
//Given "x" UV level, "y" returns seconds needed to burn, we measure time spent in "x" and calc a "% burned"
//(Time Spent at UV level)/(-267.48*(UV level)+3913.6) + ((Time Spent at UV level)/(-267.48*(UV level)+3913.6))*ginger index
//1 = ginger. Increase to burn faster, decrease to be a beautiful tan person
//default is 1
float GingerIndex = 1;
//Input SPF used
//default is 30
float SPFIndex = 30;
//How long you expect your sunscreen to last in seconds
//default is 3600
int SunScreenDurationSeconds = 3600;
//How fast you want to recover from sunburn
//Percent decay per second, as a decimal
//Default = 0.005
float BurnDecay = 0.005;
//Mode control, what we want to start on
//default is 1
int WatchModeSelect = 1;
//LED Brightness
int LEDBrigthness = 10;

//------------------Mark UV-------------
// Adafruit says voltage/0.1V = UV index
// GUVA-S12SD sensor attached to this pin - voltage is 4.3 x diode photocurrent
#define UV_PIN PIN_PA2
float counts;
float voltage;

//------------------Mark Neo-------------
// NeoPixel Pin
#define NEO_PIN PIN_PA3
//Count of NeoPixels
#define NUMPIXELS 12
//Setup Neopixels
tinyNeoPixel pixels = tinyNeoPixel(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);
#define ON 255
uint8_t red = 0;
uint8_t green = 0;
uint8_t blue = ON;

//------------------RH Neo-------------
int RedLEDTimer = 0;
int GreenLEDTimer = 0;
int BlueLEDTimer = 0;
float TotalLEDs = NUMPIXELS;

//------------------Mark Accel-------------
//Initialize accel, setting up
extern int8_t accelInit();
//Run in accel.ino, not used here
extern void accelRead();
//changes global variable ZeroAtTwelve BOOL (int) 0-1
extern bool orientDisplay();
//Setup comms
#define I2C_WRITE 0
// Accel values
uint16_t aOrient;
//Check if accel is available
uint8_t gotAccel = 0;
uint16_t aShake;
uint16_t notMoving = 0;
uint8_t ZeroAtTwelve = 0;

//------------------RH Algo-------------
//we assume the person isn't wearing sunscreen (BOOOOO!)
bool SunScreenApplied = false;
//Variables to help calculate
int SunScreenTTBTimer = SunScreenDurationSeconds;
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
float CurrentReading = 0;

//MS to loop for delay.
//**Do not edit (dire consequences)**
int LoopDelay = 10;

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.println("UV Watch");
  Serial.println();

  TinyI2C.init();

  pixels.begin();
  pixels.setBrightness(LEDBrigthness);

  gotAccel = accelInit();
}

int SensorCaptureTimer = 0;
int SensorCaptureTrigger = 10;

int DisplayTimer = 0;
int DisplayTimerTrigger = 100;

int MoCapTimer = 0;
int MoCapTimerTrigger = 1;

void loop() {
  SensorCaptureTimer++;
  DisplayTimer++;
  MoCapTimer++;

  //-----------------------------Motion CAPTURE---------------------------------------
  if (MoCapTimer >= MoCapTimerTrigger) {
    orientDisplay();
    if (ZeroAtTwelve) {
      WatchModeSelect = 1;
    } else {
      WatchModeSelect = 2;
    }
    MoCapTimer = 0;
  }
  //----------------------------------------------------------------------------------



  //-----------------------------SENSOR CAPTURE---------------------------------------
  if (SensorCaptureTimer >= SensorCaptureTrigger) {
    counts = analogRead(UV_PIN);
    voltage = counts * 3300 * 5;
    voltage /= 1024;
    CurrentReading = ((voltage - 108) / 97);
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
    SensorCaptureTimer = 0;
  }
  //--------------------------------------------------------------------



  //-------------------------------DISPLAY-------------------------------------
  if (DisplayTimer >= DisplayTimerTrigger) {
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
    switch (WatchModeSelect) {
      //Percent burend display
      case 1:
        //Checking to make sure we aren't burnt!
        if (PercentBurned < 100) {
          //One LED at a time. Showing how burnt
          for (float PixelLocation = 0; PixelLocation < TotalLEDs; PixelLocation++) {
            if (PixelLocation == int(TotalLEDs * (PercentBurned / 100)) && PercentBurned > 0) {
              RedLEDTimer = 255 * (PixelLocation / TotalLEDs);
              GreenLEDTimer = 255 - RedLEDTimer;
              pixels.setPixelColor(PixelLocation, pixels.Color(RedLEDTimer, GreenLEDTimer, 0));
              if (SunScreenApplied) {
                for (int i = 0; i < (SunScreenTTBTimer / 900) + 1; i++) {
                  PixelLocation++;
                  if (PixelLocation < 12) {
                    pixels.setPixelColor(PixelLocation, pixels.Color(0, 0, 255));
                  } else {
                    pixels.setPixelColor(PixelLocation - 12, pixels.Color(0, 0, 255));
                  }
                }
              }
            } else {
              pixels.setPixelColor(PixelLocation, pixels.Color(0, 0, 0));
            }
          }
          pixels.show();
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
        break;
      //UV display
      case 2:
        for (float PixelLocation = 0; PixelLocation != TotalLEDs; PixelLocation++) {
          if (PixelLocation <= UVaverage && UVaverage != 0) {
            RedLEDTimer = 255 * (PixelLocation / TotalLEDs);
            GreenLEDTimer = 255 - RedLEDTimer;
            pixels.setPixelColor(PixelLocation, pixels.Color(RedLEDTimer, 0, GreenLEDTimer));
          } else {
            pixels.setPixelColor(PixelLocation, pixels.Color(0, 0, 0));
          }
          pixels.show();
        }
        break;
      default:
        // statements
        break;
    }
    DisplayTimer = 0;
  }
  //--------------------------------------------------------------------



  delay(LoopDelay);
}