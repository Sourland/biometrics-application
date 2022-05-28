#include <RF22.h>
#include <RF22Router.h>

#define MY_ADDRESS 6
#define NODE_ADDRESS 12
#define DESTINATION_ADDRESS 12

int m = 0;
int Sit = 1;
int sensorVal = 10;
boolean successful_packet = false;
long randNumber;
int max_delay=300;
int eventInterval_r = 10000;
int eventInterval_t = 5000;
RF22Router rf22(MY_ADDRESS);
int c = 0;

void setup() {
  Serial.begin(9600);
  rf22.addRouteTo(DESTINATION_ADDRESS,DESTINATION_ADDRESS);
  if (!rf22.init())
    Serial.println("RF22 init failed");
    // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
  if (!rf22.setFrequency(431.0))
     Serial.println("setFrequency Fail");
  rf22.setTxPower(RF22_TXPOW_20DBM);
        //1,2,5,8,11,14,17,20 DBM
  rf22.setModemConfig(RF22::GFSK_Rb125Fd125);
  sensorVal = analogRead(A0);
  randomSeed(sensorVal);
}

void loop() {
  unsigned long t = millis();
  unsigned long tim = t;
  if(c==0){
    int S = 10;
    char data_read[RF22_ROUTER_MAX_MESSAGE_LEN];
    uint8_t data_send[RF22_ROUTER_MAX_MESSAGE_LEN];
    memset(data_read, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
    memset(data_send, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
    sprintf(data_read, "%d", S);
    data_read[RF22_ROUTER_MAX_MESSAGE_LEN - 1] = '\0';
    memcpy(data_send, data_read, RF22_ROUTER_MAX_MESSAGE_LEN);
  
    successful_packet = false;
    while (!successful_packet)
        {
          if (rf22.sendtoWait(data_send, sizeof(data_send), DESTINATION_ADDRESS) != RF22_ROUTER_ERROR_NONE)
          {
            Serial.println("sendtoWait failed");
            randNumber=random(200,max_delay);
            Serial.println(randNumber);
            delay(randNumber);
          }
          else
          {
            successful_packet = true;
            c = 1;
          }
      }
  }
  else{
    if(m==0){
      Serial.println("Emit");
      while(tim-t < eventInterval_r){
        tim = millis();
    
        uint8_t buf[RF22_ROUTER_MAX_MESSAGE_LEN];
        char incoming[RF22_ROUTER_MAX_MESSAGE_LEN];
        memset(buf, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
        memset(incoming, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
        uint8_t len = sizeof(buf);
        uint8_t from;
        int received_value = 0; 
        if (rf22.recvfromAck(buf, &len, &from)){
          buf[RF22_ROUTER_MAX_MESSAGE_LEN - 1] = '\0';
          memcpy(incoming, buf, RF22_ROUTER_MAX_MESSAGE_LEN);
          Serial.print("DATA FROM ATHLETE: ");
          Serial.println(from, DEC);
          const char s[3] = " ";
          char *token;
          token = strtok(incoming,s);
          int Athl = atoi(token);
          token = strtok(NULL,s);
          int Speed = atoi(token);
          token = strtok(NULL,s);
          int Temp = atoi(token);
          token = strtok(NULL,s);
          int Humid = atoi(token);
          token = strtok(NULL,s);
          
          Serial.println(Athl);
        } 
        m=1;
      }
    }
    else{
      Serial.println("Transm");  
      while(tim-t < eventInterval_t){
        tim = millis();
        
        char data_read[RF22_ROUTER_MAX_MESSAGE_LEN];
        uint8_t data_send[RF22_ROUTER_MAX_MESSAGE_LEN];
        memset(data_read, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
        memset(data_send, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
        sprintf(data_read, "%d", Sit);
        data_read[RF22_ROUTER_MAX_MESSAGE_LEN - 1] = '\0';
        memcpy(data_send, data_read, RF22_ROUTER_MAX_MESSAGE_LEN);
  
        successful_packet = false;
        while (!successful_packet & tim-t < eventInterval_t)
        {
          tim = millis();
          if (rf22.sendtoWait(data_send, sizeof(data_send), DESTINATION_ADDRESS) != RF22_ROUTER_ERROR_NONE)
          {
            Serial.println("sendtoWait failed");
            randNumber=random(200,max_delay);
            Serial.println(randNumber);
            delay(randNumber);
          }
          else
          {
            successful_packet = true;
          }
        }
      m=0;
      }
    }
  }
}
