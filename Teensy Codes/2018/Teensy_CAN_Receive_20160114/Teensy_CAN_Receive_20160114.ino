#include <FlexCAN.h>

static CAN_message_t rxmsg;
// -------------------------------------------------------------
void setup(void)
{
  Can0.begin(1000000);
  pinMode(13, OUTPUT);
  digitalWrite(13, 1);
  delay(1000);
  digitalWrite(13, 0);
  Serial.println("CAN Receive Example");
}
// -------------------------------------------------------------
void loop(void)
{

  

  if ( Can0.read(rxmsg) )
  {
    Serial.println("\n*********");
    Serial.println(rxmsg.id);
    Serial.println(rxmsg.len);
    uint8_t dumpLen = sizeof(rxmsg);
    Serial.println(dumpLen);
    int rxByte[dumpLen];
    uint8_t *gua = (uint8_t *)&rxmsg;
    uint8_t working;

    for( int i=0 ; i<dumpLen ; i++  )
    {
      working = *gua++;
      rxByte[i] = (working);
      Serial.print(rxByte[i]);
      Serial.print("  ");
    }

    int rxID = rxByte[1]*256 + rxByte[0];
    Serial.print("ID: ");
    Serial.println(rxID);

    int rxDataLen = rxByte[7];
    Serial.print("Data Length: ");
    Serial.println(rxDataLen);

    int rxData[rxDataLen];
    Serial.print("Data: ");
    for(int i=0;i<rxDataLen;i++)
    {
      rxData[i]=rxByte[i+8];
      Serial.print(rxData[i]);
      Serial.print(" ");
    }
    Serial.println();
    Serial.print("Data: ");
    for(int i=0;i<rxmsg.len;i++)
    {
      rxData[i]=rxmsg.buf[i];
      Serial.print(rxData[i]);
      Serial.print(" ");
    }
  } //******** receive CAN message




}
