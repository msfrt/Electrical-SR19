
#include <FlexCAN.h>
FlexCAN CANbus(1000000);
static CAN_message_t rxmsg;
//static CAN_message_t msg;

int VIN[22];
int VIN_count = 0;
int VIN_total = 0;
int waterPump = 0;
int waterPump1 = 0;
int fan = 0;

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
  Serial.begin(9600);
  CANbus.begin();
  pinMode(A9, INPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
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

    if (rxID == 51) //ID 0x101 contains RPM, Throttle Pos
    {      
      //waterPump = rxData[0] + rxData[1] * 255;
      waterPump  = rxData[2] + rxData[3] * 255;      
      // Throttle = Throttle / 10;
      //TC  = rxData[4] * 255      + rxData[5];
      //BP  = rxData[6] * 255      + rxData[7];
    }

  }
}


void loop()
{
  time = millis();
  CAN_decode();
  waterPump1 = map(waterPump, 600, 1000, 0 , 255);
  analogWrite(5, waterPump1);
  analogWrite(6, waterPump1);

  if(time - prev_time > 1000){
  //Serial.println(waterPump1);
  //Serial.print("----");
  //Serial.println(analogRead(A9));
  prev_time = time;
  }
  if(VIN_count < 20){
    VIN[VIN_count] = analogRead(A9);
    VIN_count += 1;
  }
  else{
    VIN_total = 0;
    for(int i=0; i<20; i++){
       VIN_total += VIN[i];
    }
    Serial.println(VIN_total / VIN_count);
    VIN_count = 0;
  }
}
