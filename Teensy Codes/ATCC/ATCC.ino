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

  int pin           = 0;
  int readVal       = 0;
  int readMax       = -2147483647; // minimum number possible
  int readMin       =  2147483647; // maximum number possible
  int readAvg       = 0;
  int count         = 0;
  int zeroMVolt10   = 0; // in mV*10
  int mV10unit      = 0; // in mV*10 (mV per unit, ex. 30 could be 30mV/degree C)
  double scaleFact  = 0; // like in the DBC (.1, .01, .001 etc.)
  int z1            = 390000; // z1 & z2 are for the voltage divider (units are ohms)
  int z2            = 100000; // check the wikipedia page for a diagram en.wikipedia.org/wiki/Voltage_divider
                              // if you want to change these per sensor, do it in calibration in the setup loop


} sensor;


// front ATCC
sensor FR_DAMPER_POS, FL_DAMPER_POS, TRACK_TEMP, FR_BRAKE_PRESSURE;
sensor FL_BRAKE_PRESSURE, RR_BRAKE_PRESSURE, RL_BRAKE_PRESSURE;
sensor WATER_TEMP_BETWEEN_RADS, FR_ROTOR_TEMP, FL_ROTOR_TEMP;

// rear ATCC
sensor RR_DAMPER_POS, RL_DAMPER_POS, RIGHT_RAD_TEMP, LEFT_RAD_TEMP;
sensor RR_ROTOR_TEMP, RL_ROTOR_TEMP;

// variables for message counter
int messageCount100Hz = 0;

// set the analog read resolution in bits (10 bits yeild an input 0-1023, etc.)
// initialize a variable for a calculation of the readMaximum read value from pins
const int read_resolution_bits = 10; // <--- modify this one
      int read_resolution      = 0;

// teensy voltage in mV * 10
const int teensy_voltage_mV10 = 33000;




void setup() {

  // start the CAN busses
  Can0.begin(1000000);

  Serial.begin(9600);

  // set the resolution to read the input pins
  analogReadResolution(read_resolution_bits);

  // calculate the readMaximum binary input read value from the read resolution
  // (used for dynamic sensor conversion accuracy)
  read_resolution = pow(2, read_resolution_bits) - 1;



  switch (ATCC)
  {

    // FRONT ATCC
    case 0:

      // sensor calibration
      FR_DAMPER_POS.pin         = A0;
      FR_DAMPER_POS.zeroMVolt10 = 0; // mV * 10
      FR_DAMPER_POS.mV10unit    = 0; // mV * 10 (mV per unit)
      FR_DAMPER_POS.scaleFact   = 0.1;

      FL_DAMPER_POS.pin         = A1;
      FL_DAMPER_POS.zeroMVolt10 = 0; // mV * 10
      FL_DAMPER_POS.mV10unit    = 0; // mV * 10 (mV per unit)
      FL_DAMPER_POS.scaleFact   = 0.1;

      TRACK_TEMP.pin         = A2;
      TRACK_TEMP.zeroMVolt10 = 0; // mV * 10
      TRACK_TEMP.mV10unit    = 0; // mV * 10 (mV per unit)
      TRACK_TEMP.scaleFact   = 0.1;

      FR_BRAKE_PRESSURE.pin         = A13;
      FR_BRAKE_PRESSURE.zeroMVolt10 = 0; // mV * 10
      FR_BRAKE_PRESSURE.mV10unit    = 0; // mV * 10 (mV per unit)
      FR_BRAKE_PRESSURE.scaleFact   = 0.1;

      FL_BRAKE_PRESSURE.pin         = A15;
      FL_BRAKE_PRESSURE.zeroMVolt10 = 0; // mV * 10
      FL_BRAKE_PRESSURE.mV10unit    = 0; // mV * 10 (mV per unit)
      FL_BRAKE_PRESSURE.scaleFact   = 0.1;

      RR_BRAKE_PRESSURE.pin         = A4;
      RR_BRAKE_PRESSURE.zeroMVolt10 = 0; // mV * 10
      RR_BRAKE_PRESSURE.mV10unit    = 0; // mV * 10 (mV per unit)
      RR_BRAKE_PRESSURE.scaleFact   = 0.1;

      RL_BRAKE_PRESSURE.pin         = A5;
      RL_BRAKE_PRESSURE.zeroMVolt10 = 0; // mV * 10
      RL_BRAKE_PRESSURE.mV10unit    = 0; // mV * 10 (mV per unit)
      RL_BRAKE_PRESSURE.scaleFact   = 0.1;

      WATER_TEMP_BETWEEN_RADS.pin         = A19;
      WATER_TEMP_BETWEEN_RADS.zeroMVolt10 = 0; // mV * 10
      WATER_TEMP_BETWEEN_RADS.mV10unit    = 0; // mV * 10 (mV per unit)
      WATER_TEMP_BETWEEN_RADS.scaleFact   = 0.1;

      FR_ROTOR_TEMP.pin         = A12;
      FR_ROTOR_TEMP.zeroMVolt10 = 0; // mV * 10
      FR_ROTOR_TEMP.mV10unit    = 0; // mV * 10 (mV per unit)
      FR_ROTOR_TEMP.scaleFact   = 0.1;

      FL_ROTOR_TEMP.pin         = A14;
      FL_ROTOR_TEMP.zeroMVolt10 = 0; // mV * 10
      FL_ROTOR_TEMP.mV10unit    = 0; // mV * 10 (mV per unit)
      FL_ROTOR_TEMP.scaleFact   = 0.1;


      // assign the pins output or input
      pinMode(13,                           OUTPUT); // onboard LED
      pinMode(FR_DAMPER_POS.pin,            INPUT);
      pinMode(FL_DAMPER_POS.pin,            INPUT);
      pinMode(TRACK_TEMP.pin,               INPUT);
      pinMode(RR_BRAKE_PRESSURE.pin,        INPUT);
      pinMode(RL_BRAKE_PRESSURE.pin,        INPUT);
      pinMode(FR_ROTOR_TEMP.pin,            INPUT);
      pinMode(FR_BRAKE_PRESSURE.pin,        INPUT);
      pinMode(FL_ROTOR_TEMP.pin,            INPUT);
      pinMode(FL_BRAKE_PRESSURE.pin,        INPUT);
      pinMode(WATER_TEMP_BETWEEN_RADS.pin,  INPUT);

      break;


    // REAR ATCC
    case 1:

      // sensor calibration
      RR_DAMPER_POS.pin         = 0;
      RR_DAMPER_POS.zeroMVolt10 = 0; // mV * 10
      RR_DAMPER_POS.mV10unit    = 0; // mV * 10
      RR_DAMPER_POS.scaleFact   = 0.1;

      RL_DAMPER_POS.pin         = 0;
      RL_DAMPER_POS.zeroMVolt10 = 0; // mV * 10
      RL_DAMPER_POS.mV10unit    = 0; // mV * 10
      RL_DAMPER_POS.scaleFact   = 0.1;

      RIGHT_RAD_TEMP.pin         = 0;
      RIGHT_RAD_TEMP.zeroMVolt10 = 0; // mV * 10
      RIGHT_RAD_TEMP.mV10unit    = 0; // mV * 10
      RIGHT_RAD_TEMP.scaleFact   = 0.1;

      LEFT_RAD_TEMP.pin         = 0;
      LEFT_RAD_TEMP.zeroMVolt10 = 0; // mV * 10
      LEFT_RAD_TEMP.mV10unit    = 0; // mV * 10
      LEFT_RAD_TEMP.scaleFact   = 0.1;

      RR_ROTOR_TEMP.pin         = 0;
      RR_ROTOR_TEMP.zeroMVolt10 = 0; // mV * 10
      RR_ROTOR_TEMP.mV10unit    = 0; // mV * 10
      RR_ROTOR_TEMP.scaleFact   = 0.1;

      RL_ROTOR_TEMP.pin         = 0;
      RL_ROTOR_TEMP.zeroMVolt10 = 0; // mV * 10
      RL_ROTOR_TEMP.mV10unit    = 0; // mV * 10
      RL_ROTOR_TEMP.scaleFact   = 0.1;

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



  switch ( ATCC )
  {





    // main loop for front ATCC
    case 0:

      // read the sensors in this timer at 10,000 Hz
      if ( micros() - SensTimer1000Hz >= 1 )
      {
        SensTimer1000Hz = micros()

        // read the sensors
        analogReadSensor(FR_DAMPER_POS);
        analogReadSensor(FL_DAMPER_POS);
        analogReadSensor(TRACK_TEMP);
        analogReadSensor(FR_BRAKE_PRESSURE);
        analogReadSensor(FL_BRAKE_PRESSURE);
        analogReadSensor(RR_BRAKE_PRESSURE);
        analogReadSensor(RL_BRAKE_PRESSURE);
        analogReadSensor(WATER_TEMP_BETWEEN_RADS);
        analogReadSensor(FR_ROTOR_TEMP);
        analogReadSensor(FL_ROTOR_TEMP);

      }

      // continually launch the CalculateAndLaunchCAN function, as it
      // has indivdual timers built in.
      CalculateAndLaunchCAN();

      break;






    // main loop for rear ATCC
    case 1:



      break;





  }
}













void analogReadSensor( sensor SENSOR )
// calls the analogRead function for the specified sensor
// determines if the read values are mins or maxes; modifies globals
// adds the value to the average, adds one to the counter globals
{

  // read the sensor
  SENSOR.readVal = analogRead(SENSOR.pin);

  // determine if its a min or max
  if      ( SENSOR.readVal < SENSOR.readMin ){ SENSOR.readMin = SENSOR.readVal; }
  else if ( SENSOR.readVal > SENSOR.readMax ){ SENSOR.readMax = SENSOR.readVal; }

  // add to the average and the counter
  SENSOR.readAvg += SENSOR.readVal;
  SENSOR.count++;

}











int analogToSensorVal( sensor SENSOR )
// takes raw sensor data and turns them into the human-readable
// numbers to send over CAN
{
  // remember: calculate raw average before calling this function
  // remember: reset all sensor values after calling this function!!!


  // convert the analog inputs into the teeny voltage (mV*10)
  int sensMin = SENSOR.readMin * (teensy_voltage_mV10 / read_resolution);
  int sensMax = SENSOR.readMax * (teensy_voltage_mV10 / read_resolution);
  int sensAvg = SENSOR.readAvg * (teensy_voltage_mV10 / read_resolution);

  // convert the teensy voltage (3.3V) to sensor voltage (12V) by doing the opposite
  // operations of a voltage divider (in ohms). For reference: en.wikipedia.org/wiki/Voltage_divider
  sensMin = sensMin / SENSOR.z2 * (SENSOR.z2 + SENSOR.z1);
  sensMax = sensMax / SENSOR.z2 * (SENSOR.z2 + SENSOR.z1);
  sensAvg = sensAvg / SENSOR.z2 * (SENSOR.z2 + SENSOR.z1);


  // sensor calibration (convert 12v to CAN values)
  //                   zero volt of sens.    units per mV                   inverse of the scale factor.
  sensMin = (sensMin - SENSOR.zeroMVolt10) * ( 1.0000 / SENSOR.mV10unit ) * ( 1.0000 / SENSOR.scaleFact );
  sensMax = (sensMax - SENSOR.zeroMVolt10) * ( 1.0000 / SENSOR.mV10unit ) * ( 1.0000 / SENSOR.scaleFact );
  sensAvg = (sensAvg - SENSOR.zeroMVolt10) * ( 1.0000 / SENSOR.mV10unit ) * ( 1.0000 / SENSOR.scaleFact );

  // put the values into an array to return
  int returnVals = [sensMin, sensMax, sensAvg];
  return returnVals;

}






void resetSensor(sensor SENSOR)
// resets the sensor mins, maxs, avgs, and count
{
  SENSOR.readMin =  2147483647; // maximum number possible
  SENSOR.readMax = -2147483647; // minimum number possible
  SENSOR.readAvg = 0;
  SENSOR.count   = 0;
}









static void CalculateAndLaunchCAN()
// timers for every can message grouping
//  calculate readable values
//  put the values into their respective CAN array
//  call the send message function
{


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

   // create a static array that holds information returned by analogToSensorVal
   int MinMaxAvg[3];

   switch ( ATCC )
   {
    // front ATCC
    case 0:

      if ( millis() - SendTimer100Hz >= 10 )
      {
        SendTimer100Hz = millis();

        // add one for every cycle through. Reset after 14
        if ( messageCount100Hz < 15 ){messageCount100Hz++;}
        else {messageCount100Hz = 0;}


        // turn the raw numbers into the ones we can read over CAN.
        MinMaxAvg = analogToSensorVal(FR_DAMPER_POS);
        // put the results into a message buffer
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = MinMaxAvg[1]; // max
        msg.buf[2] = MinMaxAvg[1] >> 8;
        msg.buf[3] = MinMaxAvg[2]; // average
        msg.buf[4] = MinMaxAvg[2] >> 8;
        msg.buf[5] = MinMaxAvg[0]; // min
        msg.buf[6] = MinMaxAvg[0] >> 8;
        msg.buf[7] = 0;
        // send the message
        sendCAN(ID_GOES_HERE!, 8, 0);
        // reset the global variables
        resetSensor(FR_DAMPER_POS);


        MinMaxAvg = analogToSensorVal(FL_DAMPER_POS);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = MinMaxAvg[1]; // max
        msg.buf[2] = MinMaxAvg[1] >> 8;
        msg.buf[3] = MinMaxAvg[2]; // average
        msg.buf[4] = MinMaxAvg[2] >> 8;
        msg.buf[5] = MinMaxAvg[0]; // min
        msg.buf[6] = MinMaxAvg[0] >> 8;
        msg.buf[7] = 0;
        sendCAN(ID_GOES_HERE!, 8, 0);
        resetSensor(FL_DAMPER_POS);


        MinMaxAvg = analogToSensorVal(TRACK_TEMP);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = MinMaxAvg[1]; // max
        msg.buf[2] = MinMaxAvg[1] >> 8;
        msg.buf[3] = MinMaxAvg[2]; // average
        msg.buf[4] = MinMaxAvg[2] >> 8;
        msg.buf[5] = MinMaxAvg[0]; // min
        msg.buf[6] = MinMaxAvg[0] >> 8;
        msg.buf[7] = 0;
        sendCAN(ID_GOES_HERE!, 8, 0);
        resetSensor(TRACK_TEMP);


        MinMaxAvg = analogToSensorVal(FR_BRAKE_PRESSURE);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = MinMaxAvg[1]; // max
        msg.buf[2] = MinMaxAvg[1] >> 8;
        msg.buf[3] = MinMaxAvg[2]; // average
        msg.buf[4] = MinMaxAvg[2] >> 8;
        msg.buf[5] = MinMaxAvg[0]; // min
        msg.buf[6] = MinMaxAvg[0] >> 8;
        msg.buf[7] = 0;
        sendCAN(ID_GOES_HERE!, 8, 0);
        resetSensor(FR_BRAKE_PRESSURE);


        MinMaxAvg = analogToSensorVal(FL_BRAKE_PRESSURE);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = MinMaxAvg[1]; // max
        msg.buf[2] = MinMaxAvg[1] >> 8;
        msg.buf[3] = MinMaxAvg[2]; // average
        msg.buf[4] = MinMaxAvg[2] >> 8;
        msg.buf[5] = MinMaxAvg[0]; // min
        msg.buf[6] = MinMaxAvg[0] >> 8;
        msg.buf[7] = 0;
        sendCAN(ID_GOES_HERE!, 8, 0);
        resetSensor(FL_BRAKE_PRESSURE);


        MinMaxAvg = analogToSensorVal(RR_BRAKE_PRESSURE);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = MinMaxAvg[1]; // max
        msg.buf[2] = MinMaxAvg[1] >> 8;
        msg.buf[3] = MinMaxAvg[2]; // average
        msg.buf[4] = MinMaxAvg[2] >> 8;
        msg.buf[5] = MinMaxAvg[0]; // min
        msg.buf[6] = MinMaxAvg[0] >> 8;
        msg.buf[7] = 0;
        sendCAN(ID_GOES_HERE!, 8, 0);
        resetSensor(RR_BRAKE_PRESSURE);


        MinMaxAvg = analogToSensorVal(RL_BRAKE_PRESSURE);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = MinMaxAvg[1]; // max
        msg.buf[2] = MinMaxAvg[1] >> 8;
        msg.buf[3] = MinMaxAvg[2]; // average
        msg.buf[4] = MinMaxAvg[2] >> 8;
        msg.buf[5] = MinMaxAvg[0]; // min
        msg.buf[6] = MinMaxAvg[0] >> 8;
        msg.buf[7] = 0;
        sendCAN(ID_GOES_HERE!, 8, 0);
        resetSensor(RL_BRAKE_PRESSURE);


        MinMaxAvg = analogToSensorVal(WATER_TEMP_BETWEEN_RADS);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = MinMaxAvg[1]; // max
        msg.buf[2] = MinMaxAvg[1] >> 8;
        msg.buf[3] = MinMaxAvg[2]; // average
        msg.buf[4] = MinMaxAvg[2] >> 8;
        msg.buf[5] = MinMaxAvg[0]; // min
        msg.buf[6] = MinMaxAvg[0] >> 8;
        msg.buf[7] = 0;
        sendCAN(ID_GOES_HERE!, 8, 0);
        resetSensor(WATER_TEMP_BETWEEN_RADS);


        MinMaxAvg = analogToSensorVal(FR_ROTOR_TEMP);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = MinMaxAvg[1]; // max
        msg.buf[2] = MinMaxAvg[1] >> 8;
        msg.buf[3] = MinMaxAvg[2]; // average
        msg.buf[4] = MinMaxAvg[2] >> 8;
        msg.buf[5] = MinMaxAvg[0]; // min
        msg.buf[6] = MinMaxAvg[0] >> 8;
        msg.buf[7] = 0;
        sendCAN(ID_GOES_HERE!, 8, 0);
        resetSensor(FR_ROTOR_TEMP);

        MinMaxAvg = analogToSensorVal(FL_ROTOR_TEMP);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = MinMaxAvg[1]; // max
        msg.buf[2] = MinMaxAvg[1] >> 8;
        msg.buf[3] = MinMaxAvg[2]; // average
        msg.buf[4] = MinMaxAvg[2] >> 8;
        msg.buf[5] = MinMaxAvg[0]; // min
        msg.buf[6] = MinMaxAvg[0] >> 8;
        msg.buf[7] = 0;
        sendCAN(ID_GOES_HERE!, 8, 0);
        resetSensor(FL_ROTOR_TEMP);

      } // end 100Hz timer
      break;




    // rear ATCC
    case 1:
      break;
  }
}










void sendCAN(int id, int len, int busNo)
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
