// (c) Michael Schoeffler 2017, http://www.mschoeffler.de

#include "Wire.h" // This library allows you to communicate with I2C devices.

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
const uint16_t AccelScaleFactor = 10000;

int16_t accelerometer_z; // variables for accelerometer raw data
int16_t temperature; // variables for temperature data

int u0;
int a0 = 0;
int tmp = 0;
int counter = 0;
int average = 0;
int totalA = 0;

char tmp_str[7]; // temporary variable used in convert function

char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  tmp = millis();
}

void loop() {
  double Az,T,v;
  
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
  
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  accelerometer_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  temperature = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  
  if(millis() - tmp > 10){
    Az = (double)accelerometer_z/AccelScaleFactor;
    T = (double)temperature/340+36.53; //temperature formula
    
    totalA+=Az;
    
    counter++;
    if(counter==10){
      average=(double)totalA/10;
      Serial.println(average);
      counter=0;
      totalA=0;
    }
    tmp=millis();
  }
  v=u0+(average+a0)/2*0.01; //v0=0 and a0=0 in the beginning
  u0=v; // given the previous v value to v0 for the next loop
  a0=average;
  //Serial.println(Az);
}
