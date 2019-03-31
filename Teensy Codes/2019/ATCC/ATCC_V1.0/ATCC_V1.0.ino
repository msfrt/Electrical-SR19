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
//  Written by:     Dave Yonkers
//  Created:        03/04/2019
//  Version:        1.0
//  Purpose:        Send some sexts over a couple of twisted wires
//  Description:    Analog-to-CAN converter code
//------------------------------------------------------------------------------


// IMPORTANT!!!
// before uploading, update the "ATCC" constant to the correct module
// possible values:   0 : FRONT
//                    1 : REAR
#define ATCC = 1


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
  int sensAvg     = 0;
  int sensMax     = 0;
  int sensMin     = 0;
  int sensCount   = 0;
  int outVals[3];
  

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

// define the pins for each sensor - front ATCC
#define FR_DAMPER_POS_pin A0
#define FL_DAMPER_POS_pin A1
#define TRACK_TEMP_pin A2
#define SWA_pin A3
#define RR_BRAKE_PRESSURE_pin A4
#define RL_BRAKE_PRESSURE_pin A5
#define FR_ROTOR_TEMP_pin A12
#define FR_BRAKE_PRESSURE_pin A13
#define FL_ROTOR_TEMP_pin A14
#define FL_BRAKE_PRESSURE_pin A15
#define WATER_TEMP_BETWEEN_RADS_pin A19

// set the analog read resolution in bits (10 bits yeild an input 0-1023, etc.)
// initialize a variable for a calculation of the maximum read value from pins
const int read_resolution_bits = 10; // <--- modify this one
      int read_resolution      = 0;




void setup() {

  // start the CAN busses
  Can0.begin(1000000);

  Serial.begin(9600);

  // set the resolution to rad the inputs pins
  analogReadResolution(read_resolution_bits);

  // calculate the maximum binary input read value from the read resolution
  // (used for dynamic sensor conversion accuracy)
  read_resolution = pow(2, read_resolution_bits) - 1;



  switch (ATCC)
  {

    // FRONT ATCC
    case 0:
      // I took these from the "pinout" excel doc in the ATCC code folder, so it's probably wrong
      pinMode(13,                           OUTPUT); // onboard LED
      pinMode(FR_DAMPER_POS_pin,            INPUT);
      pinMode(FL_DAMPER_POS_pin,            INPUT);
      pinMode(TRACK_TEMP_pin,               INPUT);
      pinMode(SWA_pin,                      INPUT);
      pinMode(RR_BRAKE_PRESSURE_pin,        INPUT);
      pinMode(RL_BRAKE_PRESSURE_pin,        INPUT);
      pinMode(FR_ROTOR_TEMP_pin,            INPUT);
      pinMode(FR_BRAKE_PRESSURE_pin,        INPUT);
      pinMode(FL_ROTOR_TEMP_pin,            INPUT);
      pinMode(FL_BRAKE_PRESSURE_pin,        INPUT);
      pinMode(WATER_TEMP_BETWEEN_RADS_pin,  INPUT);


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
    sensor FR_DAMPER_POS, FL_DAMPER_POS, TRACK_TEMP, FR_BRAKE_PRESSURE;
    sensor FL_BRAKE_PRESSURE, RR_BRAKE_PRESSURE, RL_BRAKE_PRESSURE;
    sensor WATER_TEMP_BETWEEN_RADS, FR_ROTOR_TEMP, FL_ROTOR_TEMP, SWA;

    // rear ATCC
    sensor RR_DAMPER_POS, RL_DAMPER_POS, RIGHT_RAD_TEMP, LEFT_RAD_TEMP;
    sensor RR_ROTOR_TEMP, LR_ROTOR_TEMP;



    // front ATCC
    case 0:
      switch ( sensGroup ) {
        case 0:

          // read analog pins
          FR_DAMPER_POS.sensReadVal           = analogRead(FR_DAMPER_POS_pin);
          FL_DAMPER_POS.sensReadVal           = analogRead(FL_DAMPER_POS_pin);
          FR_BRAKE_PRESSURE.sensReadVal       = analogRead(FR_BRAKE_PRESSURE_pin);
          FL_BRAKE_PRESSURE.sensReadVal       = analogRead(FR_BRAKE_PRESSURE_pin);
          RR_BRAKE_PRESSURE.sensReadVal       = analogRead(RR_BRAKE_PRESSURE_pin);
          RL_BRAKE_PRESSURE.sensReadVal       = analogRead(RL_BRAKE_PRESSURE_pin);
          FR_ROTOR_TEMP.sensReadVal           = analogRead(FR_ROTOR_TEMP_pin);
          FL_ROTOR_TEMP.sensReadVal           = analogRead(FL_ROTOR_TEMP_pin);
          TRACK_TEMP.sensReadVal              = analogRead(TRACK_TEMP_pin);
          WATER_TEMP_BETWEEN_RADS.sensReadVal = analogRead(WATER_TEMP_BETWEEN_RADS_pin);
          SWA.sensReadVal                     = analogRead(SWA_pin);




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

          // FR damper pos
          if      ( FR_DAMPER_POS.sensReadVal < FR_DAMPER_POS.sensMin ){ FR_DAMPER_POS.sensMin = FR_DAMPER_POS.sensReadVal; }
          else if ( FR_DAMPER_POS.sensReadVal > FR_DAMPER_POS.sensMax ){ FR_DAMPER_POS.sensMax = FR_DAMPER_POS.sensReadVal; }
          FR_DAMPER_POS.sensAvg += FR_DAMPER_POS.sensReadVal;
          FR_DAMPER_POS.sensCount++;

          // FL damper pos
          if      ( FL_DAMPER_POS.sensReadVal < FL_DAMPER_POS.sensMin ){ FL_DAMPER_POS.sensMin = FL_DAMPER_POS.sensReadVal; }
          else if ( FL_DAMPER_POS.sensReadVal > FL_DAMPER_POS.sensMax ){ FL_DAMPER_POS.sensMax = FL_DAMPER_POS.sensReadVal; }
          FL_DAMPER_POS.sensAvg += FL_DAMPER_POS.sensReadVal;
          FL_DAMPER_POS.sensCount++;

          // FR brake pressure
          if      ( FR_BRAKE_PRESSURE.sensReadVal < FR_BRAKE_PRESSURE.sensMin ){ FR_BRAKE_PRESSURE.sensMin = FR_BRAKE_PRESSURE.sensReadVal; }
          else if ( FR_BRAKE_PRESSURE.sensReadVal > FR_BRAKE_PRESSURE.sensMax ){ FR_BRAKE_PRESSURE.sensMax = FR_BRAKE_PRESSURE.sensReadVal; }
          FR_BRAKE_PRESSURE.sensAvg += FR_BRAKE_PRESSURE.sensReadVal;
          FR_BRAKE_PRESSURE.sensCount++;

          // FL brake pressure
          if      ( FL_BRAKE_PRESSURE.sensReadVal < FL_BRAKE_PRESSURE.sensMin ){ FL_BRAKE_PRESSURE.sensMin = FL_BRAKE_PRESSURE.sensReadVal; }
          else if ( FL_BRAKE_PRESSURE.sensReadVal > FL_BRAKE_PRESSURE.sensMax ){ FL_BRAKE_PRESSURE.sensMax = FL_BRAKE_PRESSURE.sensReadVal; }
          FL_BRAKE_PRESSURE.sensAvg += FL_BRAKE_PRESSURE.sensReadVal;
          FL_BRAKE_PRESSURE.sensCount++;

          // RR brake pressure
          if      ( RR_BRAKE_PRESSURE.sensReadVal < RR_BRAKE_PRESSURE.sensMin ){ RR_BRAKE_PRESSURE.sensMin = RR_BRAKE_PRESSURE.sensReadVal; }
          else if ( RR_BRAKE_PRESSURE.sensReadVal > RR_BRAKE_PRESSURE.sensMax ){ RR_BRAKE_PRESSURE.sensMax = RR_BRAKE_PRESSURE.sensReadVal; }
          RR_BRAKE_PRESSURE.sensAvg += RR_BRAKE_PRESSURE.sensReadVal;
          RR_BRAKE_PRESSURE.sensCount++;

          // RL brake pressure
          if      ( RL_BRAKE_PRESSURE.sensReadVal < RL_BRAKE_PRESSURE.sensMin ){ RL_BRAKE_PRESSURE.sensMin = RL_BRAKE_PRESSURE.sensReadVal; }
          else if ( RL_BRAKE_PRESSURE.sensReadVal > RL_BRAKE_PRESSURE.sensMax ){ RL_BRAKE_PRESSURE.sensMax = RL_BRAKE_PRESSURE.sensReadVal; }
          RL_BRAKE_PRESSURE.sensAvg += RL_BRAKE_PRESSURE.sensReadVal;
          RL_BRAKE_PRESSURE.sensCount++;

          // RR rotor temp
          if      ( FR_ROTOR_TEMP.sensReadVal < FR_ROTOR_TEMP.sensMin ){ FR_ROTOR_TEMP.sensMin = FR_ROTOR_TEMP.sensReadVal; }
          else if ( FR_ROTOR_TEMP.sensReadVal > FR_ROTOR_TEMP.sensMax ){ FR_ROTOR_TEMP.sensMax = FR_ROTOR_TEMP.sensReadVal; }
          FR_ROTOR_TEMP.sensAvg += FR_ROTOR_TEMP.sensReadVal;
          FR_ROTOR_TEMP.sensCount++;

          // FL rotor temp
          if      ( FL_ROTOR_TEMP.sensReadVal < FL_ROTOR_TEMP.sensMin ){ FL_ROTOR_TEMP.sensMin = FL_ROTOR_TEMP.sensReadVal; }
          else if ( FL_ROTOR_TEMP.sensReadVal > FL_ROTOR_TEMP.sensMax ){ FL_ROTOR_TEMP.sensMax = FL_ROTOR_TEMP.sensReadVal; }
          FL_ROTOR_TEMP.sensAvg += FL_ROTOR_TEMP.sensReadVal;
          FL_ROTOR_TEMP.sensCount++;

          // track temp
          if      ( TRACK_TEMP.sensReadVal < TRACK_TEMP.sensMin ){ TRACK_TEMP.sensMin = TRACK_TEMP.sensReadVal; }
          else if ( TRACK_TEMP.sensReadVal > TRACK_TEMP.sensMax ){ TRACK_TEMP.sensMax = TRACK_TEMP.sensReadVal; }
          TRACK_TEMP.sensAvg += TRACK_TEMP.sensReadVal;
          TRACK_TEMP.sensCount++;

          // water temp between radiators
          if      ( WATER_TEMP_BETWEEN_RADS.sensReadVal < WATER_TEMP_BETWEEN_RADS.sensMin ){ WATER_TEMP_BETWEEN_RADS.sensMin = WATER_TEMP_BETWEEN_RADS.sensReadVal; }
          else if ( WATER_TEMP_BETWEEN_RADS.sensReadVal > WATER_TEMP_BETWEEN_RADS.sensMax ){ WATER_TEMP_BETWEEN_RADS.sensMax = WATER_TEMP_BETWEEN_RADS.sensReadVal; }
          WATER_TEMP_BETWEEN_RADS.sensAvg += WATER_TEMP_BETWEEN_RADS.sensReadVal;
          WATER_TEMP_BETWEEN_RADS.sensCount++;

          // steering wheel angle
          if      ( SWA.sensReadVal < SWA.sensMin ){ SWA.sensMin = SWA.sensReadVal; }
          else if ( SWA.sensReadVal > SWA.sensMax ){ SWA.sensMax = SWA.sensReadVal; }
          SWA.sensAvg += SWA.sensReadVal;
          SWA.sensCount++;


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

          // raw sensor input to teensy ( milivolts * 10 )
          SWA.
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
