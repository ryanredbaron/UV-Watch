# UV Watch
## Current MVP
 - OSHpark board to test firmware on, fits NEAR to watch size
 - Low power, 8 hours on battery
 - Low component count
 - BETA quality, make it work son!
 - Wearable daily, in comfort/usability/not too dorky
 - Tell time
 - Tell current/peak UV
 - Motion control
 
## Project List
1. Hardware
   - [ ] OshPark board design
   - [ ] Build/fit watch in case
   
2. Software
   - [ ] Integrate with new processor
   - [ ] Time control (accurate)
   - [ ] Docking procedure to update time and charge
   
## To-do list
### Firmware
### Hardware
- Custom hardware 
  - Test pads for charging/comms/programming
  - ATMega 32 u4
  - WS2812 LEDs in ring config
  - UV sensor (existing)
  - Battery??
  - Motion sensing

## Specs
   - Selected Sensor - VEML-6070
     - https://www.digikey.com/product-detail/en/vishay-semiconductor-opto-division/VEML6070/VEML6070CT-ND/5171286
     - http://www.vishay.com/product?docid=84277
     - https://www.adafruit.com/product/2899
   - NLA - (current Beta-WEMOS setup) - VEML-6075
     - https://www.digikey.com/product-detail/en/vishay-semiconductor-opto-division/VEML6075/VEML6075CT-ND/5878946
     - https://www.vishay.com/ppg?84304
     - https://www.adafruit.com/product/3964
# Wemos Setup
  - Wemos D1 Pins
    - 5V
      - NeoPixel "PWR", pwr in.
      - Adafruit VEML-6075, VIN in.
    - GND
      - NeoPixel "GND", gnd in. 
      - Adafruit VEML-6075, gnd in. 
    - D4
      - NeoPixel "In", data in.
    - D2
      - 
    - D1
      - 
 - Adafruit VEML-6075 Pins
    - VIN
    - 3Vo
    - GND
    - SCL
    - SDA
 - Adafruit NeoPixel 12 ring Pins	
   - 

	Glass Diameter 		= 36.75mm
	First Step Height	= 03.00mm
	First Step Diameter	= 40.55mm
	First Step Lip Width 	= 01.90mm
	
![alt text](https://github.com/ryanredbaron/UV-Watch/blob/master/Specifications/adafruit_products_image.png?raw=true)
