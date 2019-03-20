//------------------------------------------------------------------------------
//  Written by:     Dave Yonkers
//  Created:        02/09/2019
//  Modified By:    Dave Yonkers
//  Last Modified:  02/09/2019 2:00 AM
//  Version:        0.0
//  Purpose:        Send test values in messages over can
//  Description:    read teensy inputs and send them over can
//------------------------------------------------------------------------------

#include <FlexCAN.h>
#include <kinetis_flexcan.h>

// define message type
static CAN_message_t msg;

// initialize a timer variable to not spam the bus
unsigned long messageTimer = 0;
unsigned long ledTimer = 0;
unsigned long valueTimer = 0;
unsigned long LEDTimer40Hz = 0;

// input values
int input1 = 0;
int input2 = 0;
int input3 = 0;
int input4 = 0;

// output values
int output1 = 0;
int output2 = 0;
int output3 = 0;
int output4 = 0;

// temp array
int temp[] = {0, 851, 935, 950, 1000};

// voltage array
int voltage[] = {80, 110, 113, 119, 120};

// counter
int arrayPos = 0;

// set the pins for the teensy
int led_pin = 13;
bool LED_on = true;

void setup() {
  Can0.begin(1000000);
  //Can1.begin(1000000);
  Serial.begin(9600);

  // pinmodes
  pinMode(13, OUTPUT); // led
  //pinMode(##, INPUT); //input1
  //pinMode(##, INPUT); //input2
  //pinMode(##, INPUT); //input3
  //pinMode(##, INPUT); //input4

}

void loop() {
  //input1 = analogRead(input1_pin);
  //input2 = analogRead(input2_pin);

  if ( millis() - LEDTimer40Hz >= 25)
  {
    LEDTimer40Hz = millis();
    //analogWrite(A7, 255);

    if      ( LED_on == false ){ digitalWrite(13, HIGH); LED_on = true; }
    else if ( LED_on == true ){ digitalWrite(13, LOW); LED_on = false; }
  }


  if ( millis() - valueTimer >= 20000 )
  {
    valueTimer = millis();

    output1 = temp[arrayPos];
    output2 = voltage[arrayPos];

    Serial.print("output1: "); Serial.println(output1);
    Serial.print("output2: "); Serial.println(output2);
    Serial.println();

    // increment by 1
    arrayPos++;

    // if greater than 4, reset to the 0th position in the array
    if ( arrayPos > 4 )
    {
      arrayPos = 0;
    }

  }


  if ( millis() - messageTimer >= 10 )
  {
    messageTimer = millis();

    // message data set up to mimick engine temp (output1) from M400
    // except buf 6&7 are normally Fuel Pressure, but here they are used as voltage

    msg.buf[0] = 4; // multiplexor ID
    msg.buf[1] = 0;
    msg.buf[2] = 0;
    msg.buf[3] = 0;
    msg.buf[4] = output1 >> 8;
    msg.buf[5] = output1;
    msg.buf[6] = output2 >> 8;
    msg.buf[7] = output2;
    CAN_DATA_SEND(0x5EF, 8, 0);
  }

}


static void CAN_DATA_SEND(int id, int len, int busNo)
{
  msg.len = len;  //CAN message length unit: Byte (8 bits)
  msg.id = id; //CAN ID

  // if (busNo == 1)
  // {
  //   Can1.write(msg);  // this is send the static
  // }
  if (busNo == 0)
  {
    Can0.write(msg);  // this is send the static
  }

}
