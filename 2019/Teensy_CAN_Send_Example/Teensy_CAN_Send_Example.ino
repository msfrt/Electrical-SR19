



/* **************************************
 *        Teensy Can Send testing
 * needs Teensy and can circuit
 * Date: Nov 24
 * Purpose: CAN Data send
 * Author: Jacky
 * Description:
 *   
 * *************************************/

#include <FlexCAN.h>
#include <kinetis_flexcan.h>

int led = 13;
//FlexCAN CANbus(1000000);
static CAN_message_t msg,rxmsg;
//static uint8_t hex[17] = "0123456789abcdef";
// -------------------------------------------------------------
void setup(void)
{
  Can0.begin(1000000);
  pinMode(led, OUTPUT);
  digitalWrite(led, 1);
  delay(1000);
  Serial.print("Gua - Send\n\n");
}// -------------------------------------------------------------
void loop(void)
{
    Serial.print("\n*****SendStart*****");
    msg.len = 8;  //CAN message length unit: Byte (8 bits) 
    msg.id = 0x0; //CAN ID
    
    //writing values in to the buffer
      msg.buf[0]=0;
      msg.buf[1]=1;
      msg.buf[2]=2;
      msg.buf[3]=3;
      msg.buf[4]=4;
      msg.buf[5]=5;
      msg.buf[6]=6;
      msg.buf[7]=7;
      
      //displaying the sent message buffer
    for( int i=8; i>=0; i-- ) {
      Serial.print("\nmsg.buf[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.print(msg.buf[i]);  
    }
    Can0.write(msg);  // this is send the static  
    
    Serial.print("\n*****SendEND*****\n");
    
    digitalWrite(led, 1);
    delay(50);
    digitalWrite(led, 0);
    delay(50);
    
}
