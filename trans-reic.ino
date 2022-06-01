#include <RF22.h>
#include <RF22Router.h>

#define MY_ADDRESS 12
#define NODE_ADDRESS 6
#define DESTINATION_ADDRESS 6
#define RED 8
#define GREEN 9


#include "Wire.h" // This library allows you to communicate with I2C devices.
#include "DHT.h"
#define dhttype DHT11 

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
const uint16_t AccelScaleFactor = 5000;
int dhtpin=5;

DHT dht(dhtpin, dhttype);

int16_t accelerometer_z; // variables for accelerometer raw data
int16_t temperature; // variables for temperature data

int u0;
int a0 = 0;
int tmp = 0;
int Tmp = 0;
int counter = 0;
int average = 0;
int totalA = 0;
int received_value = 0;

char tmp_str[7]; // temporary variable used in convert function

char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}

long randNumber;
int max_delay = 300;
int eventInterval = 5000;
unsigned long t = 0;
RF22Router rf22(MY_ADDRESS);
int c = 0;
int num = 1;

void setup() {
  Serial.begin(9600); 
  Initialize();
  pinMode(RED,OUTPUT);
  pinMode(GREEN,OUTPUT);
  dht.begin();
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Tmp = millis();
  t = millis();
}

void loop() {
  unsigned long tmp = millis();
  unsigned long Tmp;
  double Az,T,v;

  if(received_value < 50){
    digitalWrite(GREEN, HIGH);
    digitalWrite(RED, LOW);
  }
  else{
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
  }
  
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers

  float hum=(double)dht.readHumidity(); //reading humidity
  float temp=(double)dht.readTemperature(); //reading temperature in Celsius
  
  if(isnan(hum) || isnan(temp)){
    Serial.println("DHT sensor read failure!!");
    return;
  }
 
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  Wire.read()<<8;
  Wire.read()<<8;
  accelerometer_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  temperature = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  Wire.read()<<8;
  Wire.read()<<8;
  Wire.read()<<8;
  
  if(millis() - Tmp > 10){
    Az = (double)accelerometer_z/AccelScaleFactor;
    T = (double)temperature/340+36.53; //temperature formula
    
    totalA+=Az;
    
    counter++;
    if(counter==10){
      average=(double)totalA/10;
      counter=0;
      totalA=0;
    }
    Tmp=millis();
  }
  
  v=(double)u0+(average+a0)/2*0.01; //v0=0 and a0=0 in the beginning
  u0=v; // given the previous v value to v0 for the next loop
  a0=average;

  if(c==0){
      uint8_t buf[RF22_ROUTER_MAX_MESSAGE_LEN];
      char incoming[RF22_ROUTER_MAX_MESSAGE_LEN];
      memset(buf, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
      memset(incoming, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
      uint8_t len = sizeof(buf);
      uint8_t from; 

      if (rf22.recvfromAck(buf, &len, &from)){
        buf[RF22_ROUTER_MAX_MESSAGE_LEN - 1] = '\0';
        memcpy(incoming, buf, RF22_ROUTER_MAX_MESSAGE_LEN);
        Serial.print("got request from : ");
        Serial.println(from, DEC);
        received_value = atoi((char*)incoming);
        Serial.println(received_value);
        c = 1;
        t = millis();
      }
 }
 else{  
      char data_read[RF22_ROUTER_MAX_MESSAGE_LEN];
      uint8_t data_send[RF22_ROUTER_MAX_MESSAGE_LEN];
      memset(data_read, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
      memset(data_send, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
      sprintf(data_read, "%d %d %d %d %d", num, (int)T, (int)v, (int)temp, (int)hum);
      data_read[RF22_ROUTER_MAX_MESSAGE_LEN - 1] = '\0';
      memcpy(data_send, data_read, RF22_ROUTER_MAX_MESSAGE_LEN);
     
      if (rf22.sendtoWait(data_send, sizeof(data_send), DESTINATION_ADDRESS) != RF22_ROUTER_ERROR_NONE)
      {
          randNumber=random(200,max_delay);
          delay(randNumber);
      }
      else
      {
          Serial.print(T);
          Serial.print("°C ");  
          Serial.print(v);
          Serial.print(" ");
          Serial.print(temp);
          Serial.print("°C ");
          Serial.print(hum);
          Serial.println("% ");
      }
      if(tmp - t > eventInterval){
        c = 0;
      }
    }     
 }


void Initialize(){
  rf22.addRouteTo(NODE_ADDRESS,NODE_ADDRESS);
  if (!rf22.init())
    Serial.println("RF22 init failed");
      // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
  if (!rf22.setFrequency(431.0))
    Serial.println("setFrequency Fail");
  rf22.setTxPower(RF22_TXPOW_20DBM);
      //1,2,5,8,11,14,17,20 DBM
  rf22.setModemConfig(RF22::GFSK_Rb125Fd125);
      //modulation
}
