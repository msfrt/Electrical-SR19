/*
 * Written by:    Dave Yonkers
 * Created :      01/29/2019
 * Modified By:   Dave Yonkers
 * Last Modified: 01/31/2019 7:45 PM
 * Version:       0.0
 * Purpose:       CAN test
 * Description:   Tryna to send some messages less than a byte
 */

//include and initialize CAN
#include <FlexCAN.h>
FlexCAN CANbus(1000000);
static CAN_message_t msg;
static CAN_message_t rxmsg;

//initialize the arrays to store the analog inputs
int ANAVolt[21], SensVal[21], ANAread[1];

//initialize the timer variables
unsigned long SendTimer10000Hz = 0;
unsigned long SendTimer1000Hz = 0;
unsigned long SendTimer500Hz  = 0; 
unsigned long SendTimer200Hz  = 0;
unsigned long SendTimer100Hz  = 0;
unsigned long SendTimer50Hz   = 0;
unsigned long SendTimer20Hz   = 0;
unsigned long SendTimer10Hz   = 0;

// set an array of 8bit values
uint8_t Byte[] = {0, 0, 0, 0, 0, 0, 0, 0}; // each 0 is stored in binary as 00000000

// initialize variables to store reads
int RPMRead = 0;

// initiallize sensor read totals
int RPMReadTotal = 0;

// initiallize sensor read counters
int RPMReadCounter = 0;

// initiallize sensor max to the smallest value for int
int RPMReadMax = -2147483648;

// initiallize sensor min to the largest value for int
int RPMReadMin = 2147483648;

// initiallize sensor averages
int RPMReadAvg = 0;

// initiallize a counter
uint8_t counter = 0;


//************* Bit testing values ************************************


// enter a value 0-31
uint8_t value_one = 18;    // 00010010

// enter a value 0-3
uint8_t value_two = 2;     // 00000010

// enter a value 0-1
uint8_t value_three = 1;   // 00000001

// this should set Byte[0] equal to 10010101, or 149
//Byte[0] = (value_one << 3) | (value_two << 1) | (value_three);


//************* End Bit testing values ********************************


void setup(void) 
{
  CANbus.begin();
  analogReadResolution(13);
  pinMode(13, OUTPUT);
  digitalWrite(13, 1);
  delay(1000);
  digitalWrite(13, 0);

  // ***********for bit testing**********************************
  // this should set Byte[0] equal to 10010101, or 149
  Byte[0] = (value_one << 3) | (value_two << 1) | (value_three);
}
// -------------------------------------------------------------

void loop(void)
{

  if ( micros() - SendTimer10000Hz >  100 ) 
  {
    SendTimer10000Hz = micros();
    
    // read signal
    SIGREAD();
    
    // add to signal totals
    RPMReadTotal += RPMRead;
    
    // increment counters
    RPMReadCounter += 1;
    
    // see if sensor max
    if (RPMRead > RPMReadMax)
    {
      RPMReadMax = RPMRead;
    }
    
    // see if sensor min
    if (RPMRead < RPMReadMin)
    {
      RPMReadMin = RPMRead;
    }
    
    // check to see if 1ms (
    if ( millis() - SendTimer100Hz > 10 )
    {
      // calculate the avg
      RPMReadAvg = random(-100, 100);
      
      // send data
      CANMUX();
      
      // reset total, min, max, and count
      RPMReadTotal = 0;
      RPMReadMax = -2147483648;
      RPMReadMin = 2147483648;
      RPMReadCounter = 0;
    }


  }

  
}



// ************Functions************

static void SIGREAD()
{
  RPMRead = 1;
}

static void CANMUX()
{

  if ( millis() - SendTimer500Hz >=  2 )
  {
    SendTimer500Hz = millis();
    
    // each msg.buf[] sends one byte (8-bits)
    msg.buf[0] = (counter << 4);
    msg.buf[1] = RPMReadAvg;
    msg.buf[2] = RPMReadAvg >> 8;
    msg.buf[3] = Byte[3];
    msg.buf[4] = Byte[4];
    msg.buf[5] = Byte[5];
    msg.buf[6] = Byte[6];
    msg.buf[7] = Byte[7];
    CAN_DATA_SEND(0x0, 8); // 500Hz

    // each msg.buf[] sends one byte (8-bits)
    msg.buf[0] = (1 << 6) | (1);
    msg.buf[1] = Byte[1];
    msg.buf[2] = Byte[2];
    msg.buf[3] = Byte[3];
    msg.buf[4] = Byte[4];
    msg.buf[5] = Byte[5];
    msg.buf[6] = Byte[6];
    msg.buf[7] = Byte[7];
    CAN_DATA_SEND(0x348, 8); // 500Hz

    if ( counter < 15)
    {
      counter += 1;
    } 
    
    else 
    {
      counter = 0;
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
