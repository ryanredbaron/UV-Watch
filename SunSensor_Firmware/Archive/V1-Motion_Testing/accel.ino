int X = 0;
int Y = 0;
int Z = 0;
int T = 0;

extern uint16_t aOrient;
extern uint8_t  gotAccel;
extern uint16_t aShake;
extern uint16_t notMoving;

// MXC4005 Registers

#define ACCELaddress 0x15        // I2C address

#define INT_SRC0  0x00
#define INT_CLR0  0x00
#define INT_SRC1  0x01
#define INT_CLR1  0x01
#define STATUSREG 0x02
#define XOUT      0x03
#define XOUT_U    0x03
#define XOUT_L    0x04
#define YOUT      0x05
#define YOUT_U    0x05
#define YOUT_L    0x06
#define ZOUT      0x07
#define ZOUT_U    0x07
#define ZOUT_L    0x08
#define TOUT      0x09
#define INT_MASK0 0x0A
#define INT_MASK1 0x0B
#define DETECTION 0x0C
#define CONTROL   0x0D
#define DEVICE_ID 0x0E
#define VERSION   0x0F

// 0x0C - DETECTION register
  //  D7    D6      D5      D4      D3      D2    D1     D0
  // SMM  SHTH[2] SHTH[1] SHTH[0] SHC[1]  SHC[0] ORC[1] ORC[0]
  //
  // SHM = 0 - second shake must exceed SHTH[2:0] with opposite sign and number of shakes set by SHC[1:0]
  //     = 1 - second shake must have opposite sign and number of shakes set by SHC[1:0]
  // SHTH - shake threshold, 000=0.25g ... 111=2.0g in 0.25g steps
  // SHC - X/Y shake reading count (100Hz) - 00=8, 01=16, 10=32, 11=64
  // ORC - orientation valid count (100Hz) - 00=16, 01=32, 10=64, 11=128

#define SHM 0b10000000

////////////////////////////////////
// Low level I2C read/write routines
////////////////////////////////////

// Write a register select to accelerometer

uint8_t wrAccelReg(uint8_t reg) {
  TinyI2C.start(ACCELaddress, I2C_WRITE);
  TinyI2C.write(reg);
  TinyI2C.stop();
  return 1;
}

// Write a register select and one data bye to accelerometer

uint8_t wrAccelRegData(uint8_t reg, uint8_t dat) {
  TinyI2C.start(ACCELaddress, I2C_WRITE);
  TinyI2C.write(reg);
  TinyI2C.write(dat);
  TinyI2C.stop();
  return 1;
}

// Read accel value (16 bits) from accelerometer

uint16_t readAccel16() {
  uint8_t rdbuffer[2] = {0x55,0x55};

  TinyI2C.start(ACCELaddress, 2);
  rdbuffer[0] = TinyI2C.read();
  rdbuffer[1] = TinyI2C.read();
  TinyI2C.stop();
  
  return ( (rdbuffer[0] << 4) | (rdbuffer[1] >> 4));
}

// Read accel value (8 bits) from accelerometer

uint8_t readAccel8() {
   uint8_t rd;

  TinyI2C.start(ACCELaddress, 1);
  rd = TinyI2C.read();
  TinyI2C.stop();
  
  return rd;
}

////////////////////////////////////
// MXC4005 Read Register Routines
////////////////////////////////////

// 0x00 - INT_SRC0 - shake and change in orientation 
  //  D7    D6      D5      D4      D3      D2     D1     D0
  // CHORZ CHORXY   0       0      SHYM    SHYP   SHXM   SHXP
  //
  // CHORZ - change in Z axis orientation
  // CHORXY - change in the XY plane detected
  // SHxp - shake detected in X+, X-, Y+, Y-

#define CHORZ   0b10000000
#define CHORXY  0b01000000
#define SHYM    0b00001000
#define SHYP    0b00000100
#define SHXM    0b00000010
#define SHXP    0b00000001

uint8_t accelINT_SRC0() {
  wrAccelReg(INT_SRC0);           
  uint8_t ret = readAccel8();   
//  wrAccelRegData(INT_SRC0, CHORZ+CHORXY+SHYM+SHYP+SHXM+SHXP);   // Reset interrupt flags
  return ret; 
}

// 0x01 - INT_SRC1 -  Data ready interrupt sources, tilt & orientation status 
  //     D7        D6     D5      D4      D3      D2     D1     D0
  // TILT/ORZ[1] ORZ[0] ORXY[1] ORXY[0]    0       0     0     DRDY
  //
  // TILT/ORZ[1] - X/Y signal too small to reliably tell orientation
  // ORZ[0] - Z orientation +/-  
  // ORXY - 00=+X, 01=+Y, 10=-X, 11=-Y
  // DRDY - data ready in XOUT, YOUT, ZOUT, TOUT registers

uint8_t accelINT_SRC1() {
  wrAccelReg(INT_SRC1);           
  return readAccel8();    
}

// 0x02 - Status register
  //  D7    D6      D5      D4      D3      D2     D1       D0
  //   0    0       0      ORD   ORIZ[1] ORIZ[0] ORIXY[1] ORIXY[0]
  //
  // ORD - 1 when device ready to use
  // ORIZ - unfiltered ORIZ (in INT_SRC1) (not normally used??)
  // ORIXY - unfiltered ORXY (in INT_SRC1) (not normally used??)

#define ORD 0b00010000
#define sORIZ    0b00001100
#define sORIZ1   0b00001000
#define sORIZ0   0b00000100

#define sORIXY   0b00000011
#define sORIXY1  0b00000010
#define sORIXY0  0b00000001

#define sPOS_Z_NO_TILT 0b00000000
#define sNEG_Z_NO_TILT 0b00000100
#define sPOS_Z_TILT    0b00001000
#define sNEG_Z_TILT    0b00001100

#define sPOS_X  0b00000000
#define sPOS_Y  0b00000001
#define sNEG_X  0b00000010
#define sNEG_Y  0b00000011

uint8_t accelSTATUS() {
  wrAccelReg(STATUSREG);   // Select STATUS register
  return readAccel8();     // Read status
}

// Read and return the raw axis counts (X,Y,Z)
  // 12 bit signed value (2s complement) in 2 consecutive 8 bit registers
  // 8 MSBs in the lower-numbered register, 4 LSBs in upper nibble of higher register

int accelXYZ(uint8_t axis) {
  wrAccelReg(axis);
  int ax = readAccel8();
  ax <<= 8;                 // Shift MSBs all the way to the left of the 16 bit int
  wrAccelReg(axis + 1);
  int axl = readAccel8();
  ax += axl;               // Make image of full 12 bit register
  ax >>= 4;                 // sHIFT out the low 4 bits, and sign extend
  return ax;
}

////////////////////////////////////
// General Purpose Routines
////////////////////////////////////

// See if MXC4005XC accelerometer is present and initialize

bool accelBegin(bool shakeMode, uint8_t shakeThreshold, uint8_t shakeCount, uint8_t orientationCount) {
  uint8_t id = 0;

  // Check we can access the device over I2C
  TinyI2C.start(ACCELaddress, 0);
  TinyI2C.write(DEVICE_ID);                // Select ID register
  TinyI2C.restart(ACCELaddress, 1);
  id = TinyI2C.read();                // Read ID (shold be 0x02)
  TinyI2C.stop();

  if (id != 0x02 && id != 0x03) return 0;           // ID is hardwired to 0x02 or 0x03??, return fail

  // Configure he DETECTION register - shake mode, thresholds, filtering  
  uint8_t sconfig = shakeMode;
  sconfig <<= 3;
  sconfig += shakeThreshold;
  sconfig <<=2;
  sconfig += shakeCount;
  sconfig <<=2;
  sconfig += orientationCount;
  wrAccelRegData(DETECTION, sconfig);     
   
  // Wait for the device to be ready (calibration OTP memory loaded)
  // This appears to only be set after a movement????
//  id = 0;
//  while((id & ORD) == 0) {      // Wait for ORD bit to be set to one
//    Serial.println("Not ready");
//    wrAccelReg(STATUSREG);         // Select STATUS register
//    id = readAccel16();             // Read status
//  }

  return 1;  
}

// Simple getters for the raw axis values

int accelX() {
  return accelXYZ(XOUT);
}

int accelY() {
  return accelXYZ(YOUT);
}

int accelZ() {
  return accelXYZ(ZOUT);
}

// Return orientation (axis signs effectively)

#define bX 0b00001000
#define bY 0b00000100
#define bZ 0b00000010
#define bT 0b00000001

uint8_t accelXYZT() {
  uint8_t xyzt = 0;
  uint8_t stat = accelSTATUS();     // Read status register

  if ( (stat & sORIZ) == sPOS_Z_NO_TILT) { Z = 1; T = 0;}         
  if ( (stat & sORIZ) == sNEG_Z_NO_TILT) { Z = 0; T = 0;}
  if ( (stat & sORIZ) == sPOS_Z_TILT) { Z = 1; T = 1;}
  if ( (stat & sORIZ) == sNEG_Z_TILT) { Z = 0; T = 1;}
//  if(Z) Serial.print(" +Z ");
//  else  Serial.print(" -Z ");
//  if(T) Serial.print(" +T ");
//  else  Serial.print(" -T ");

  if ( (stat & sORIXY) == sPOS_X) X = 1;          // +X
  if ( (stat & sORIXY) == sNEG_X) X = 0;          // -X
//  if(X) Serial.print(" +X ");
//  else  Serial.print(" -X ");

  if ( (stat & sORIXY) == sPOS_Y) Y = 1;          // +HY
  if ( (stat & sORIXY) == sNEG_Y) Y = 0;          // -Y
//  if(Y) Serial.print(" +Y ");
//  else  Serial.print(" -Y ");
//
//  if (X && Z && T) Serial.print(" Raise ");
//  else             Serial.print("       ");
//  
//  Serial.print("\r");
  if(X) xyzt += bX;
  if(Y) xyzt += bY;
  if(Z) xyzt += bZ;
  if(T) xyzt += bT;

  return xyzt;
}

////////////////////////////////////
// Experimental and Deprecated Stuffs
////////////////////////////////////

void getShake() {
  uint8_t stat = accelINT_SRC0();
  
  //  D7    D6      D5      D4      D3      D2     D1     D0
  // CHORZ CHORXY   0       0      SHYM    SHYP   SHXM   SHXP

  if(stat & SHYM) Serial.print(" SHYM ");
  else            Serial.print("      ");
  if(stat & SHYP) Serial.print(" SHYP ");
  else            Serial.print("      ");
  if(stat & SHXM) Serial.print(" SHXM ");
  else            Serial.print("      ");
  if(stat & SHXP) Serial.print(" SHXP ");
  else            Serial.print("      ");

//  Serial.print("\r");

  if(stat & 0x0F) wrAccelRegData(INT_SRC0, CHORZ+CHORXY+SHYM+SHYP+SHXM+SHXP);
}

// Read accelerometer

void readStatus() {

  uint8_t stat = accelSTATUS();     // Read status register

  if ( (stat & sORIZ) == sPOS_Z_NO_TILT) { Z = 1; T = 0;}         
  if ( (stat & sORIZ) == sNEG_Z_NO_TILT) { Z = 0; T = 0;}
  if ( (stat & sORIZ) == sPOS_Z_TILT) { Z = 1; T = 1;}
  if ( (stat & sORIZ) == sNEG_Z_TILT) { Z = 0; T = 1;}
  if(Z) Serial.print(" +Z ");
  else  Serial.print(" -Z ");
  if(T) Serial.print(" +T ");
  else  Serial.print(" -T ");

  if ( (stat & sORIXY) == sPOS_X) X = 1;          // +X
  if ( (stat & sORIXY) == sNEG_X) X = 0;          // -X
  if(X) Serial.print(" +X ");
  else  Serial.print(" -X ");

  if ( (stat & sORIXY) == sPOS_Y) Y = 1;          // +HY
  if ( (stat & sORIXY) == sNEG_Y) Y = 0;          // -Y
  if(Y) Serial.print(" +Y ");
  else  Serial.print(" -Y ");

  if (X && Z && T) Serial.print(" Raise ");
  else             Serial.print("       ");
  
  Serial.print("\r");
}

// Read accelerometer

void readINT_SRC1() {
  
  uint8_t stat = accelINT_SRC1();

  if( (stat & sORIZ) == 0x40) Serial.println("ORZ ");
  else                        Serial.print("....");

  stat >>= 4;     // Shift so bit positions match the STATUS register positions

  if ( (stat & sORIZ) == sPOS_Z_NO_TILT) { Z = 1; T = 0;}         
  if ( (stat & sORIZ) == sNEG_Z_NO_TILT) { Z = 0; T = 0;}
  if ( (stat & sORIZ) == sPOS_Z_TILT) { Z = 1; T = 1;}
  if ( (stat & sORIZ) == sNEG_Z_TILT) { Z = 0; T = 1;}
  if(Z) Serial.print(" +Z ");
  else  Serial.print(" -Z ");
  if(T) Serial.print(" +T ");
  else  Serial.print(" -T ");

  if ( (stat & sORIXY) == sPOS_X) X = 1;          // +X
  if ( (stat & sORIXY) == sNEG_X) X = 0;          // -X
  if(X) Serial.print(" +X ");
  else  Serial.print(" -X ");

  if ( (stat & sORIXY) == sPOS_Y) Y = 1;          // +HY
  if ( (stat & sORIXY) == sNEG_Y) Y = 0;          // -Y
  if(Y) Serial.print(" +Y ");
  else  Serial.print(" -Y ");

  if (X && Z && T) Serial.print(" Raise ");
  else             Serial.print("       ");
  
  Serial.print("\r");
}

// Read accelerometer

void accelRead() {
  // 0x02 - Status register
  //  D7    D6      D5      D4      D3      D2     D1       D0
  //   0    0       0      ORD   ORIZ[1] ORIZ[0] ORIXY[1] ORIXY[0]
  //
  // ORD - 1 when device ready to use
  // ORIZ - unfiltered ORIZ (in INT_SRC1) (not normally used??)
  // ORIXY - unfiltered ORXY (in INT_SRC1) (not normally used??)

  wrAccelReg(STATUSREG);     // Select STATUS register
  aOrient = readAccel16();    // Read status

//  wrAccelReg(0x00);          // Select INT_SRC0 interrupt source register
//  aShake = readAccel16();     // Read 
//  wrAccelRegData(0x00, 0xCF);   // Clear change and shake interrupt bits     
}

bool orientDisplay() {

  accelRead();            // Get STATUS to aOrient and INT_SRC0 to aShake

  // ORIXY[1] ORIXY[0] Orientation
  //   0       0        +X
  //   0       1        +Y
  //   1       0        -X
  //   1       1        -Y
  
  if ( (aOrient & 0x30) == 0x00) lefthanded = 0;          // +X
  if ( (aOrient & 0x30) == 0x20) lefthanded = 1;          // -X

  return( (aShake & 0xC00) == 0);           // 0 == No change in XY or Z, != 0 if XY or Z changed
}
