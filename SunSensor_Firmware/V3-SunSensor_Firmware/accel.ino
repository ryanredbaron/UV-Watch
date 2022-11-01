extern uint16_t aOrient;
extern uint8_t  gotAccel;
extern uint16_t aShake;
extern uint16_t notMoving;

#define ACCELaddress 0x15        // I2C address

// Write a register select to accelerometer

uint8_t wrAccel(uint8_t reg) {
  TinyI2C.start(ACCELaddress, I2C_WRITE);
  TinyI2C.write(reg);
  TinyI2C.stop();
  return 1;
}

// Write a register select and data to accelerometer

uint8_t wrAccel2(uint8_t reg, uint8_t dat) {
  TinyI2C.start(ACCELaddress, I2C_WRITE);
  TinyI2C.write(reg);
  TinyI2C.write(dat);
  TinyI2C.stop();
  return 1;
}

// Read accel value (32 bits) from accelerometer

uint16_t rdAccel() {
   uint8_t rdbuffer[2] = {0x55,0x55};

  TinyI2C.start(ACCELaddress, 2);
  rdbuffer[0] = TinyI2C.read();
  rdbuffer[1] = TinyI2C.read();
  TinyI2C.stop();
  
  return ( (rdbuffer[0] << 4) | (rdbuffer[1] >> 4));
}

// See if MXC4005XC accelerometer is present and initialize

int8_t accelInit() {
  uint16_t id = 0;

  TinyI2C.start(ACCELaddress, 0);
  TinyI2C.write(0x0E);
  TinyI2C.restart(ACCELaddress, 1);
  id = TinyI2C.read();
  TinyI2C.stop();
  wrAccel2(0x0C, 0x80);      // Set shake mode
  return id;
  
//  wrAccel(0x0E);  
//  
//  TinyI2C.start(ACCELaddress, 1);
//  id = TinyI2C.read();
//
//  if (id != 0x02) {
//        return 0;
//  }
//
//  wrAccel2(0x0C, 0x80);      // Set shake mode
//  return 1;
}

// Read accelerometer

void accelRead() {
  wrAccel(0x02);
  aOrient = rdAccel();
  wrAccel(0x00);
  aShake = rdAccel();
  wrAccel2(0x00, 0xCF);   // Clear change and shake interrupt bits     
}

bool orientDisplay() {

  accelRead();

  // ORIXY[1] ORIXY[0] Orientation
  //   0       0        +X
  //   0       1        +Y
  //   1       0        -X
  //   1       1        -Y
  
  if ( (aOrient & 0x30) == 0x00) ZeroAtTwelve = 1;          // +X
  if ( (aOrient & 0x30) == 0x20) ZeroAtTwelve = 0;          // -X

  return( (aShake & 0xC00) == 0);           // 0 == No change in XY or Z
}