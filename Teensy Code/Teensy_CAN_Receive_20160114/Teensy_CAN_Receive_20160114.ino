#include <FlexCAN.h>
FlexCAN CANbus(1000000);
static CAN_message_t rxmsg;
// -------------------------------------------------------------
void setup(void)
{
  CANbus.begin();
  pinMode(13, OUTPUT);
  digitalWrite(13, 1);
  delay(1000);
  digitalWrite(13, 0);
  Serial.println("CAN Receive Example");
}
// -------------------------------------------------------------
void loop(void)
{
  if ( CANbus.read(rxmsg) ) {
    Serial.println("\n*********");
    uint8_t dumpLen = sizeof(rxmsg);
    int rxByte[dumpLen];
    uint8_t *gua = (uint8_t *)&rxmsg;
    uint8_t working;
    for( int i=0 ; i<dumpLen ; i++  ){
      working = *gua++;
      rxByte[i] = working,HEX; //Serial.print(rxByte[i]);
    }
    int rxID = rxByte[1]*256 + rxByte[0];
    Serial.print("ID: "); 
    Serial.println(rxID);
    int rxDataLen = rxByte[5];
    Serial.print("Data Length: "); 
    Serial.println(rxDataLen);
    int rxData[rxDataLen];
    Serial.print("Data: ");
    for(int i=0;i<rxDataLen;i++){
      rxData[i]=rxByte[i+8];
      Serial.print(rxData[i]);
      Serial.print(" ");
    }
  } //******** receive CAN message
  
  
  
  
}




