//------------------------------------------------------------------------------
//  Written by:     Dave Yonkers
//  Created :       02/01/2019
//  Modified By:    Dave Yonkers
//  Last Modified:  02/01/2019 10:00 PM
//  Version:        0.0
//  Purpose:        avoid high inrush current for DC motors
//  Description:    use PWM to soft start a motor and read values off of CAN
//------------------------------------------------------------------------------


#include <FlexCAN.h>
#include <kinetis_flexcan.h>

// define message type
static CAN_message_t rxmsg;

// initialize some variables
unsigned long SendTimer20Hz = 0;
unsigned long SendTimer10Hz = 0;



// initialize variables for testing
int FANL_minPWM         = 50;
int FANL_maxPWM         = 255;
int FANL_livePWM        = 100;
int FANL_powerPercent   = 0;
int FANL_incrementPWM   = 1;

// variable to keep track ot the LED
bool LED_on = false;



void setup() {

  // set the pins as inputs or outputs
  pinMode(13, OUTPUT);

  // begin CAN0
  Can0.begin(1000000);


}





void loop() {
  // put your main code here, to run repeatedly:


  // continuously read can
  CAN_READ();


  if ( millis() - SendTimer20Hz >= 500 )
  {
    SendTimer20Hz = millis();
    //Serial.println("testing the first timer");

    //CAN_READ();
    FANL_livePWM = softPower(FANL_powerPercent, FANL_livePWM, FANL_minPWM, FANL_maxPWM, FANL_incrementPWM);
    Serial.print("      CAN0 recieved FanL percent: "); Serial.println(FANL_powerPercent);
    Serial.print("SOFT_START returned livePWM rate: "); Serial.println(FANL_livePWM);
    Serial.println("");
  }

  if ( millis() - SendTimer10Hz >= 250 )
  {
    SendTimer10Hz = millis();

    if      ( LED_on == false ){ digitalWrite(13, HIGH); LED_on = true; }
    else if ( LED_on == true  ){ digitalWrite(13, LOW);  LED_on = false; }
  }


}

//------------------------------------------------------------------------------
//          FUNCTIONS
//------------------------------------------------------------------------------

int softPower(int powerPercent, int livePWM, int minPWM, int maxPWM, int incrementPWM)
{
//------------------------------------------------------------------------------
// when given a percentage of power, live PWM, minimum allowed PWM,
// and the desired PWM increment per cycle, SOFT_POWER will return a PWM
// value +/- the increment desired
//------------------------------------------------------------------------------

  // first convert the power percent to a PWM value
  int targetPWM = map(powerPercent, 0, 100, 0, 255);
  Serial.print("                      Target PWM: "); Serial.println(targetPWM);

  // make sure that the target is above the minimum PWM, otherwise do nothing
  if ( targetPWM >= minPWM )
  {

    // if the live PWM is less than the minimum PWM, set it to the minimum
    if ( livePWM <= minPWM )
    {
      livePWM = minPWM;
    }

    // live PWM is less than the target PWM
    if ( livePWM < targetPWM )
    {
      livePWM += incrementPWM;

      // be sure that the new PDM does not exceed the maximum!
      if ( livePWM > maxPWM )
      {
        livePWM = maxPWM;
      }
    }

    // live PWM is more than the target PWM
    else if ( livePWM > targetPWM)
    {
      livePWM -= incrementPWM;

      // if the increments fuck up because of weird readings, make sure that the livePWM
      // does not slip below the minumim. This is because we're in the (targetPWM >= minPWM) if statement
      // so we still want to motors to run, even if it's at the minimum PWM permitted. NOTE: notice that this
      // if statement alreacy exists, but it's important to repeat it after we reduce the PWM, because otherwise
      // we might end up sending a dead current to the motors.
      if ( livePWM < minPWM )
      {
        livePWM = minPWM;
      }


    }


  }
  // if the target PWM is below the minimum, set the actual PWM to 0
  else
  {
    livePWM = 0;
  }

  return livePWM;
}


//Can Input Conversion-----------------------------------------

void CAN_READ()
{
  // make sure there's a message available to read
  //Serial.println("attempting to read");
  if (Can0.available())
  {
    // print test to see if the teensy saw a message
    //Serial.println("CAN0 available for reading");

    // read the can message on bus -
    Can0.read(rxmsg);
    //Serial.println("CAN0 was read!");

    uint8_t dumpLen = sizeof(rxmsg);
    int rxByte[dumpLen];
    uint8_t *gua = (uint8_t *)&rxmsg;
    uint8_t working;

    //Serial.println(dumpLen);
    for ( int i = 0 ; i < dumpLen ; i++  )
    {
      working = *gua++;
      rxByte[i] = working; //Serial.print(rxByte[i]);
    }

    int rxID = rxByte[1] * 256 + rxByte[0];
    int rxDataLen = rxByte[5];
    int rxData[rxDataLen];

    // Serial.print(rxDataLen); <------ uncomment to view the disastrous datalenth
    // I changed "i < rxDataLen" to "i < 8" because the rxDataLen was incorrect, resulting in an rxData array length.
    // all of our messages are 8 bytes long, so I just set it to 8. -Dave
    for (int i = 0; i < 8; i++)
    {
      //Serial.print("completed loop: "); Serial.println(i);
      rxData[i] = rxByte[i + 8];
    }

    //****************

    switch(rxID)
    {
      // ID 0x101
      case 257:
        FANL_powerPercent = rxData[0] + rxData[1] * 256;
        break;
    }

  }
}
