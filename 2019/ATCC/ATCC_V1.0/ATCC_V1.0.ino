//------------------------------------------------------------------------------
//
//       ██████╗  ██████╗      ██████╗ ██████╗ ███████╗███████╗███╗   ██╗
//      ██╔════╝ ██╔═══██╗    ██╔════╝ ██╔══██╗██╔════╝██╔════╝████╗  ██║
//      ██║  ███╗██║   ██║    ██║  ███╗██████╔╝█████╗  █████╗  ██╔██╗ ██║
//      ██║   ██║██║   ██║    ██║   ██║██╔══██╗██╔══╝  ██╔══╝  ██║╚██╗██║
//      ╚██████╔╝╚██████╔╝    ╚██████╔╝██║  ██║███████╗███████╗██║ ╚████║
//       ╚═════╝  ╚═════╝      ╚═════╝ ╚═╝  ╚═╝╚══════╝╚══════╝╚═╝  ╚═══╝
//
//------------------------------------------------------------------------------
//  Written by:     Dave Yonkers & Nicholas Kopec
//  Created:        03/04/2019
//  Modified By:    Dave Yonkers
//  Last Modified:  03/04/2019 6:45 AM Ireland time ;;;;;;))) cheeky
//  Version:        1.0
//  Purpose:        Send some sexts over a couple of twisted wires
//  Description:    Analog-to-CAN converter code
//
//  Last change:    Dave started to write the code.
//
//------------------------------------------------------------------------------


// IMPORTANT!!!
// before uploading, update the "ATCC" constant to the correct module
// possible values:   0 : FRONT
//                    1 : REAR
const int ATCC = 0;


//------------------------------------------------------------------------------
//
//                  CAN Bus Initialization
//
//------------------------------------------------------------------------------

// CAN Bus 0 is the critical bus - recieve messages only
// CAN Bus 1 is the non critical bus - send and recieve

// include and initialize CAN ----------------------------------

// if CAN is not working make sure that FlexCAN is not
// installed. (Check the onenote for instructions)
#include <FlexCAN.h>
#include <kinetis_flexcan.h>

// define message type
static CAN_message_t msg;
static CAN_message_t rxmsg;

//------------------------------------------------------------------------------
//
//             Timer Initialization
//
//------------------------------------------------------------------------------

// initialize the CAN send timer variables
unsigned long SendTimer1000Hz   = 0;
unsigned long SendTimer500Hz    = 0;
unsigned long SendTimer200Hz    = 0;
unsigned long SendTimer100Hz    = 0;
unsigned long SendTimer50Hz     = 0;
unsigned long SendTimer20Hz     = 0;
unsigned long SendTimer10Hz     = 0;
unsigned long SendTimer1Hz      = 0;

// initialize the Sensor reading timer variables
unsigned long SensTimer10000Hz  = 0;
unsigned long SensTimer2000Hz   = 0;
unsigned long SensTimer1000Hz   = 0;
unsigned long SensTimer500Hz    = 0;
unsigned long SensTimer200Hz    = 0;
unsigned long SensTimer100Hz    = 0;
unsigned long SensTimer50Hz     = 0;
unsigned long SensTimer20Hz     = 0;
unsigned long SensTimer10Hz     = 0;
unsigned long SensTimer1Hz      = 0;

// initialize a timer and variable for the LED
unsigned long LEDTimer          = 0;
bool LED_on                     = false;


// structure for sensor reading
typedef struct
{

  // used to tell the status of the module
  uint8_t deviceStatus;

  int sensReadVal = 0;
  int sensCalcVal = 0;
  int sensMax     = 0;
  int sensMin     = 0;
  int sensCount   = 0;

} sensor;


// front ATCC
sensor FR_DAMPER_POS, FL_DAMPER_POS, TRACK_TEMP, FR_BRAKE_PRESSURE;
sensor FL_BRAKE_PRESSURE, RR_BRAKE_PRESSURE, RL_BRAKE_PRESSURE;
sensor WATER_TEMP_BETWEEN_RADS, FR_ROTOR_TEMP, FL_ROTOR_TEMP;

// rear ATCC
sensor RR_DAMPER_POS, RL_DAMPER_POS, RIGHT_RAD_TEMP, LEFT_RAD_TEMP;
sensor RR_ROTOR_TEMP, LR_ROTOR_TEMP;

// variables for message counter
int messageCount100Hz = 0;







void setup() {

  // start the CAN busses
  Can0.begin(1000000);

  Serial.begin(9600);

  // set the resolution of the analog input from 0-1023 (10bit) to 0-1891 (13bit)
  analogReadResolution(10);

  switch (ATCC)
  {

    // FRONT ATCC
    case 0:
      // I took these from the "pinout" excel doc in the ATCC code folder, so it's probably wrong
      pinMode(13, OUTPUT); // onboard LED
      pinMode(A0, INPUT); // FR damper position
      pinMode(A1, INPUT); // FL damper position
      pinMode(A2, INPUT); // track temp
      // pinMode(A3, INPUT); // SWA
      pinMode(A4, INPUT); // RR brake pressure
      pinMode(A5, INPUT); // RL brake pressure
      pinMode(A12, INPUT); // FR rotor temp
      pinMode(A13, INPUT); // FR brake pressure
      pinMode(A14, INPUT); // FL rotor temp
      pinMode(A15, INPUT); // FL brake pressure
      pinMODE(A19, INPUT); // water temp between rads


      break;


    // REAR ATCC
    case 1:
      pinMode(13, OUTPUT); // Onboard LED
      break;
  }

}









void loop() {
  // LED Blink - the important stuff
  if ( millis() - LEDTimer >= 100)
  {
    LEDTimer = millis();

    if      ( LED_on == false ){ digitalWrite(13, HIGH); LED_on = true; }
    else if ( LED_on == true ){ digitalWrite(13, LOW); LED_on = false; }
  }

}









static void ANA_READ( int sensGroup )
{

  //----------------------------------------------------------------------------
  // Reads the analog value of the pins ( 0 - 8191) and
  // assigns it to the correct SensVal
  //----------------------------------------------------------------------------
  switch (ATCC){



    // front ATCC
    case 0:
      switch ( sensGroup ) {
        case 0:
          // read analog pins
          break;
        break;
      }



    // rear ATCC
    case 1:
      switch ( sensGroup ) {
        case 0:
          // read analog pins
          break;
        break;
      }

  }
}









static void ANA_STORE( int sensGroup )
{

  //----------------------------------------------------------------------------
  // calls the read funcion
  // determines if the SenseVal is a min or max
  // adds the SenseVal to the average
  // adds one to the sensor counter
  //----------------------------------------------------------------------------
  switch(ATCC)
  {
    // front ATCC
    case 0:
      switch ( sensGroup ) {
        case 0:
          ANA_READ(sensGroup)
          // look at PDM code to copy min/max storage (if applicable)
          break;
        break;
      }


      // rear ATCC
      case 1:
        switch ( sensGroup ) {
          case 0:
            ANA_READ(sensGroup)
            // look at PDM code to copy min/max storage (if applicable)
            break;
          break;
        }
  }
}









static void ANA_TO_SENSORVAL(int sensGroup)
{
  switch( ATCC )
  {





    // Front ATCC
    case 0:
      switch(sensGroup)
      {
        case 0:
          // conversions will go here
          break;
      }
      break;






    // Rear ATCC
    case 1:
      switch(sensGroup)
      {
        case 0:
          // conversions will go here
          break;
      }
      break;





  }
}









static void CALC_SEND_CAN()
{

  //----------------------------------------------------------------------------
  // Timers for every CAN message
  //    calculate values and put them into CAN arrays
  //    call  send CAN function
  //    reset sensor min, max, avg, and count for future data calculations
  //----------------------------------------------------------------------------

  /*
   * Template
    msg.buf[0] = SensVal[0];
    msg.buf[1] = SensVal[0] >> 8; //
    msg.buf[2] = SensVal[1];
    msg.buf[3] = SensVal[1] >> 8; //
    msg.buf[4] = SensVal[2];
    msg.buf[5] = SensVal[2] >> 8; //
    msg.buf[6] = SensVal[3];
    msg.buf[7] = SensVal[3] >> 8; //
    CAN_DATA_SEND(0x50, 8, 0);
   *
   */


  if ( millis() - SendTimer100Hz >= 10 )
  {
    SendTimer100Hz = millis();

    if ( messageCount100Hz < 15 ){messageCount100Hz++;}
    else {messageCount100Hz = 0;}



    switch (ATCC)
    {





      // front ATCC
      case 0:
        ANA_TO_SENSORVAL(0);
        // look at PDM code for message buffer input structure
        // call the CAN_DATA_SEND function after writing to msg.buf
        break;





      // rear ATCC
      case 1:
        ANA_TO_SENSORVAL(0);
        // look at PDM code for message buffer input structure
        // call the CAN_DATA_SEND function after writing to msg.buf
        break;
    }

  }

}










static void CAN_DATA_SEND(int id, int len, int busNo)
{
  msg.len = len;  //CAN message length unit: Byte (8 bits)
  msg.id = id; //CAN ID

  switch(busNo)
  {
    case 0:
      Can0.write(msg);  // this is send the static
      break;
  }

}
