#include <RF22.h>
#include <RF22Router.h>
#define MY_ADDRESS 6
#define NODE_ADDRESS 12
#define DESTINATION_ADDRESS 12

int c = 0;
int S = 0;
int N = 0;
int P = 0;
boolean successful_packet = false;
long randNumber;
int max_delay=300;
int eventInterval = 20000;
RF22Router rf22(MY_ADDRESS);


void setup() {
  Serial.begin(9600);
  Initialize();
}

void loop() {
  unsigned long t = millis();
  unsigned long tim = t;
  if(c==0){
    if(N > P){
      S = 100;    
    }
    else{
      S = 10;
    }
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
            N = 0;
            P = 0;
          }
      }
  }
  else{
      Serial.println("Recceive");
      while(tim-t < eventInterval){
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
          const char s[4] = " ";
          char *token;
          token = strtok(incoming,s);
          int Athl = atoi(token);
          token = strtok(NULL,s);
          int In_Temp = atoi(token);
          token = strtok(NULL,s);
          int Speed = atoi(token);
          token = strtok(NULL,s);
          int Temp = atoi(token);
          token = strtok(NULL,s);
          int Humid = atoi(token);
          Serial.println(Athl);
          Serial.println(In_Temp);
          Serial.println(Speed);
          Serial.println(Temp);
          Serial.println(Humid);
          
          if(In_Temp > 39  & Temp > 38 & Humid > 90){
            N++; 
          }
          else {
            P++;
          }
        } 
        c=0;
      }
    } 
}


void Initialize(){
  rf22.addRouteTo(DESTINATION_ADDRESS,DESTINATION_ADDRESS);
  if (!rf22.init())
    Serial.println("RF22 init failed");
    // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
  if (!rf22.setFrequency(431.0))
     Serial.println("setFrequency Fail");
  rf22.setTxPower(RF22_TXPOW_20DBM);
        //1,2,5,8,11,14,17,20 DBM
  rf22.setModemConfig(RF22::GFSK_Rb125Fd125);
}
