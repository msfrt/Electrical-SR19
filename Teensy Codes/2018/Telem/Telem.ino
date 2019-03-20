
#include <FlexCAN.h>
#include <kinetis_flexcan.h>
FlexCAN CANbus(1000000);
static CAN_message_t rxmsg;
//static CAN_message_t msg;


int RPM = 0;

//initialize timer---------------------------
unsigned long prev_time = 0;
unsigned long time;


//---------------------------------------------------------------------------------------------------
//
//SETUP
//
//---------------------------------------------------------------------------------------------------
void setup() {
  //initialize communications----------------
  Serial1.begin(9600);
  CANbus.begin();
  pinMode(13, OUTPUT);
  digitalWrite(13, 1);
  delay(1000);
  digitalWrite(13, 0);
}
//-------------------------------------------------------------------
//
//MAIN
//
//-------------------------------------------------------------------
void CAN_decode()
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

    if (rxID == 257) //ID 0x101 contains RPM, Throttle Pos
    {      
      RPM = rxData[0] * 255      + rxData[1];
      //Throttle  = rxData[2] * 255      + rxData[3];
      // Throttle = Throttle / 10;
      //TC  = rxData[4] * 255      + rxData[5];
      //BP  = rxData[6] * 255      + rxData[7];
    }

  }
  Serial1.println(RPM);
}


void loop()
{
  //CAN_decode();
  if (Serial1.available()){
    Serial.print(Serial1.read());
  }
}

