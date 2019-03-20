/* **************************************


 * *************************************/

//include and initialize CAN
#include <FlexCAN.h>
FlexCAN CANbus(1000000);
static CAN_message_t msg;

//initialize the arrays to store the analog inputs
int ANAVolt[21], SensVal[21], ANAread[1];

//initialize the timer variables
unsigned long SendTimmer1000Hz = 0;
unsigned long  SendTimmer500Hz = 0, SendTimmer200Hz = 0, SendTimmer100Hz = 0;
unsigned long   SendTimmer50Hz = 0,  SendTimmer20Hz = 0,  SendTimmer10Hz = 0;

// -------------------------------------------------------------
void setup(void) 
{
  CANbus.begin();
  analogReadResolution(13);
  pinMode(13, OUTPUT);
  digitalWrite(13, 1);
  delay(1000);
  digitalWrite(13, 0);

  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);

}
// -------------------------------------------------------------

void loop(void)
{

  // Read from Analog voltage pins
  READ_ANA();             
  
  CAN_DECODE();

  SENS_CAL();

  // convert from 3.3v to 5v scall 1.2K and 2.2K resistor volt divider
  //ANAVoltCalibration();   

  //unsigned long currentTime = millis();

  if ( millis() - SendTimmer1000Hz >=  1 ) 
  {
    SendTimmer1000Hz = millis();
    CANMUX(); // get sensor value to Can message posision and send
  }
  
}




// ************Functions************

static void READ_ANA() 
{
  //---- Read analog pin ----------
    
  ANAread[0] = analogRead(A9); 
  
  for (int i = 0; i < 1; i++) {
    ANAVolt[i] = ANAread[i] *  33000 / 8191 ;
    Serial.print("\nANAVolt[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.print(ANAVolt[i]);
  }

}

/*
static void ANAVoltCalibration() {
  for (int i = 0; i < 5; i++) {
    ANAVolt[i] = ANAVolt[i] / 2200.0000 * 3400.0000 ;
    //Serial.print("\nANAVolt[");
    //Serial.print(i);
    //Serial.print("]: ");
    //Serial.print(ANAVolt[i]);
  }
}*/

static void SENS_CAL()
{

  // ***** sensor calibration *****                       //factor, min rate, name
  //SensVal[0]  = ( ANAVolt[ 0] -  4880 ) / 40000.0000 * 10000; //  0.01, 100Hz, Fan Current
  //SensVal[1]  = ( ANAVolt[ 1] -  4880 ) / 40000.0000 * 10000; //  0.01, 100Hz, WP Current
  //SensVal[2]  = ( ANAVolt[ 2] -  4950 ) / 40000.0000 * 10000; //  0.01, 100Hz, Input Current
  //SensVal[3]  =  ANAVolt[3] / 1000.000 * 49.000 * 1.01785   ; //  0.01, 100Hz, UB_PDM
  //SensVal[4]  = 0;
  
}

static void CANMUX()
{

  if ( millis() - SendTimmer500Hz >=  2 )
  {
    SendTimmer500Hz = millis();
    msg.buf[0] = SensVal[0];
    msg.buf[1] = SensVal[0] >> 8; //
    msg.buf[2] = SensVal[1];
    msg.buf[3] = SensVal[1] >> 8; //
    msg.buf[4] = SensVal[2];
    msg.buf[5] = SensVal[2] >> 8; //
    msg.buf[6] = SensVal[3];
    msg.buf[7] = SensVal[3] >> 8; //
    CAN_DATA_SEND(0x50, 8); // 500Hz




    if ( millis() - SendTimmer10Hz >=  100 ) 
    {
      SendTimmer10Hz = millis();
      //display the SensVal in current sending cycle
      Serial.print("\n*******");
      for (int i = 0; i < 1; i++) {
        Serial.print("\nSensVal[");
        Serial.print(i);
        Serial.print("]: ");
        Serial.print(SensVal[i]);

        /*/displaying the sent message buffer
          Serial.print("\n*****SendStart*****");
          for ( int i = 8; i >= 0; i-- ) {
          Serial.print("\nmsg.buf[");
          Serial.print(i);
          Serial.print("]: ");
          Serial.print(msg.buf[i]);
          }
          Serial.print("\n*****SendEND*****\n");
          //*/
      }
    }
  }
}

static void CAN_DATA_SEND(int id, int len) 
{
  msg.len = len;  //CAN message length unit: Byte (8 bits)
  msg.id = id; //CAN ID
  CANbus.write(msg);  // this is send the static
}

static void CAN_DECODE()
{

  if ( CANbus.read(rxmsg) )
  {
    uint8_t dumpLen = sizeof(rxmsg);
    int rxByte[dumpLen];
    uint8_t *gua = (uint8_t *)&rxmsg;
    uint8_t working;

    for ( int i = 0 ; i < dumpLen ; i++  )
    {
      working = *gua++;
      rxByte[i] = working; //Serial.print(rxByte[i]);
    }

    int rxID = rxByte[1] * 256 + rxByte[0];
    int rxDataLen = rxByte[5];
    int rxData[rxDataLen];
    for (int i = 0; i < rxDataLen; i++)
    {
      rxData[i] = rxByte[i + 8];
    }

    //****************

    if (rxID == 51) //ID 0x101 contains RPM, Throttle Pos
    {      
      //waterPump = rxData[0] + rxData[1] * 255;
      //waterPump  = rxData[2] + rxData[3] * 255;      
      // Throttle = Throttle / 10;
      //TC  = rxData[4] * 255      + rxData[5];
      //BP  = rxData[6] * 255      + rxData[7];
    }

  }
}
