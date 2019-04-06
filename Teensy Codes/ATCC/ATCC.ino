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
//  Purpose:        Send some 1s and 0s over a couple of twisted wires
//  Description:    Analog-to-CAN converter code
//------------------------------------------------------------------------------


// IMPORTANT!!!
// before uploading, update the "ATCC" definition to the correct module
// possible values:   0 : FRONT
//                    1 : REAR
#define ATCC 0


// include and initialize CAN

// if CAN is not working make sure that FlexCAN is not
// installed. (Check the onenote for instructions)
#include <FlexCAN.h>
#include <kinetis_flexcan.h>

// define message type
static CAN_message_t msg;
static CAN_message_t rxmsg;


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
  int actualMax     = 0;
  int actualMin     = 0;
  int actualAvg     = 0;
  int count         = 0;
  int zeroMVolt10   = 0; // in mV*10
  int mV10unit      = 0; // in mV*10 (mV per unit, ex. 30 could be 30mV/degree C)
  double scaleFact  = 1; // like in the DBC (.1, .01, .001 etc.)
  double z1         = 39000.0000; // z1 & z2 are for the voltage divider (units are ohms)
  double z2         = 10000.0000; // check the wikipedia page for a diagram en.wikipedia.org/wiki/Voltage_divider
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
      FR_DAMPER_POS.zeroMVolt10 = 0; // mV*10
      FR_DAMPER_POS.mV10unit    = 0; // mV*10 per sensor unit
      FR_DAMPER_POS.scaleFact   = 0.1;

      FL_DAMPER_POS.pin         = A1;
      FL_DAMPER_POS.zeroMVolt10 = 0; // mV*10
      FL_DAMPER_POS.mV10unit    = 0; // mV*10 per sensor unit
      FL_DAMPER_POS.scaleFact   = 0.1;

      TRACK_TEMP.pin         = A2;
      TRACK_TEMP.zeroMVolt10 = 400; // mV*10
      TRACK_TEMP.mV10unit    = 300; // mV*10 per sensor unit
      TRACK_TEMP.scaleFact   = 0.1;

      FR_BRAKE_PRESSURE.pin         = A13;
      FR_BRAKE_PRESSURE.zeroMVolt10 = 0; // mV*10
      FR_BRAKE_PRESSURE.mV10unit    = 0; // mV*10 per sensor unit
      FR_BRAKE_PRESSURE.scaleFact   = 0.1;

      FL_BRAKE_PRESSURE.pin         = A15;
      FL_BRAKE_PRESSURE.zeroMVolt10 = 0; // mV*10
      FL_BRAKE_PRESSURE.mV10unit    = 0; // mV*10 per sensor unit
      FL_BRAKE_PRESSURE.scaleFact   = 0.1;

      RR_BRAKE_PRESSURE.pin         = A4;
      RR_BRAKE_PRESSURE.zeroMVolt10 = 0; // mV*10
      RR_BRAKE_PRESSURE.mV10unit    = 0; // mV*10 per sensor unit
      RR_BRAKE_PRESSURE.scaleFact   = 0.1;

      RL_BRAKE_PRESSURE.pin         = A5;
      RL_BRAKE_PRESSURE.zeroMVolt10 = 0; // mV*10
      RL_BRAKE_PRESSURE.mV10unit    = 0; // mV*10 per sensor unit
      RL_BRAKE_PRESSURE.scaleFact   = 0.1;

      WATER_TEMP_BETWEEN_RADS.pin         = A19;
      WATER_TEMP_BETWEEN_RADS.zeroMVolt10 = 0; // mV*10
      WATER_TEMP_BETWEEN_RADS.mV10unit    = 0; // mV*10 per sensor unit
      WATER_TEMP_BETWEEN_RADS.scaleFact   = 0.1;

      FR_ROTOR_TEMP.pin         = A12;
      FR_ROTOR_TEMP.zeroMVolt10 = 50; // mV*10
      FR_ROTOR_TEMP.mV10unit    = 5000; // mV*10 per sensor unit
      FR_ROTOR_TEMP.scaleFact   = 0.1;

      FL_ROTOR_TEMP.pin         = A14;
      FL_ROTOR_TEMP.zeroMVolt10 = 50; // mV*10
      FL_ROTOR_TEMP.mV10unit    = 5000; // mV*10 per sensor unit
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
      RR_DAMPER_POS.zeroMVolt10 = 0; // mV*10
      RR_DAMPER_POS.mV10unit    = 0; // mV*10 per sensor unit
      RR_DAMPER_POS.scaleFact   = 0.1;

      RL_DAMPER_POS.pin         = 0;
      RL_DAMPER_POS.zeroMVolt10 = 0; // mV*10
      RL_DAMPER_POS.mV10unit    = 0; // mV*10 per sensor unit
      RL_DAMPER_POS.scaleFact   = 0.1;

      RIGHT_RAD_TEMP.pin         = 0;
      RIGHT_RAD_TEMP.zeroMVolt10 = 0; // mV*10
      RIGHT_RAD_TEMP.mV10unit    = 0; // mV*10 per sensor unit
      RIGHT_RAD_TEMP.scaleFact   = 0.1;

      LEFT_RAD_TEMP.pin         = 0;
      LEFT_RAD_TEMP.zeroMVolt10 = 0; // mV*10
      LEFT_RAD_TEMP.mV10unit    = 0; // mV*10 per sensor unit
      LEFT_RAD_TEMP.scaleFact   = 0.1;

      RR_ROTOR_TEMP.pin         = 0;
      RR_ROTOR_TEMP.zeroMVolt10 = 0; // mV*10
      RR_ROTOR_TEMP.mV10unit    = 0; // mV*10 per sensor unit
      RR_ROTOR_TEMP.scaleFact   = 0.1;

      RL_ROTOR_TEMP.pin         = 0;
      RL_ROTOR_TEMP.zeroMVolt10 = 0; // mV*10
      RL_ROTOR_TEMP.mV10unit    = 0; // mV*10 per sensor unit
      RL_ROTOR_TEMP.scaleFact   = 0.1;

      pinMode(13, OUTPUT); // Onboard LED


      break;
  }

}













void loop() {



  // LED Blink - the important stuff
  if ( millis() - LEDTimer >= 75)
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
      if ( micros() - SensTimer1000Hz >= 10000 ) //was 1
      {
        SensTimer1000Hz = micros();

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

      // continually launch the calculateAndLaunchCAN function, as it
      // has indivdual timers built in.
      calculateAndLaunchCAN();

      break;






    // main loop for rear ATCC
    case 1:



      break;





  }
}













static void analogReadSensor( sensor &SENSOR )
// calls the analogRead function for the specified sensor
// determines if the read values are mins or maxes; modifies globals
// adds the value to the average, adds one to the counter globals
// note: the "&" allows us to operate on the original variable, rather than
// initializing new ones for this function's scope.
{

  // read the sensor
  SENSOR.readVal = analogRead(SENSOR.pin);

  // determine if its a min or max
  if      ( SENSOR.readVal < SENSOR.readMin ){ SENSOR.readMin = SENSOR.readVal; }
  else if ( SENSOR.readVal > SENSOR.readMax ){ SENSOR.readMax = SENSOR.readVal; }

  // add to the average and the counter
  SENSOR.readAvg = SENSOR.readAvg + SENSOR.readVal;
  SENSOR.count++;


}











void analogToSensorVal( sensor &SENSOR )
// takes raw sensor data and turns them into the human-readable
// numbers to send over CAN
{
  // divide the running total for average by the count, therefore calculating
  // the true raw average
  SENSOR.readAvg /= SENSOR.count;


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
  //                   zero volt of sens.    units per mV*10                inverse of the scale factor.
  sensMin = (sensMin - SENSOR.zeroMVolt10) * ( 1.0000 / SENSOR.mV10unit ) * ( 1.0000 / SENSOR.scaleFact );
  sensMax = (sensMax - SENSOR.zeroMVolt10) * ( 1.0000 / SENSOR.mV10unit ) * ( 1.0000 / SENSOR.scaleFact );
  sensAvg = (sensAvg - SENSOR.zeroMVolt10) * ( 1.0000 / SENSOR.mV10unit ) * ( 1.0000 / SENSOR.scaleFact );


  // put the final values into the sensor's structured variabels
  SENSOR.actualMin = sensMin;
  SENSOR.actualMax = sensMax;
  SENSOR.actualAvg = sensAvg;


  // calculations are done, so reset the sensor raw read values
  resetSensor(SENSOR);
}






void resetSensor( sensor &SENSOR )
// resets the sensor mins, maxs, avgs, and count
{
  SENSOR.readMin =  2147483647; // maximum number possible
  SENSOR.readMax = -2147483647; // minimum number possible
  SENSOR.readAvg = 0;
  SENSOR.count   = 0;
}









void calculateAndLaunchCAN()
// timers for every can message grouping
//  calculate readable values
//  put the values into their respective CAN array
//  call the send message function
{

  /*
   *
    Template
    msg.buf[0] = SensVal[0];
    msg.buf[1] = SensVal[0] >> 8;
    msg.buf[2] = SensVal[1];
    msg.buf[3] = SensVal[1] >> 8;
    msg.buf[4] = SensVal[2];
    msg.buf[5] = SensVal[2] >> 8;
    msg.buf[6] = SensVal[3];
    msg.buf[7] = SensVal[3] >> 8;
    CAN_DATA_SEND(0x50, 8, 0);
   *
   */


   switch ( ATCC )
   {
    // front ATCC
    case 0:

      if ( millis() - SendTimer100Hz >= 1000 ) // changed for testing
      {
        SendTimer100Hz = millis();

        // add one for every cycle through. Reset after 14
        if ( messageCount100Hz < 15 ){messageCount100Hz++;}
        else {messageCount100Hz = 0;}


        // turn the raw numbers into the ones we can read over CAN.
        analogToSensorVal(FR_DAMPER_POS);
        // put the results into a message buffer
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = FR_DAMPER_POS.actualMax;
        msg.buf[2] = FR_DAMPER_POS.actualMax >> 8;
        msg.buf[3] = FR_DAMPER_POS.actualAvg;
        msg.buf[4] = FR_DAMPER_POS.actualAvg >> 8;
        msg.buf[5] = FR_DAMPER_POS.actualMin;
        msg.buf[6] = FR_DAMPER_POS.actualMin >> 8;
        msg.buf[7] = 0;
        // send the message
        sendCAN(0x00, 8, 0);


        analogToSensorVal(FL_DAMPER_POS);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = FL_DAMPER_POS.actualMax;
        msg.buf[2] = FL_DAMPER_POS.actualMax >> 8;
        msg.buf[3] = FL_DAMPER_POS.actualAvg;
        msg.buf[4] = FL_DAMPER_POS.actualAvg >> 8;
        msg.buf[5] = FL_DAMPER_POS.actualMin;
        msg.buf[6] = FL_DAMPER_POS.actualMin >> 8;
        msg.buf[7] = 0;
        sendCAN(0x00, 8, 0);


        Serial.println();
        Serial.print("READ MAX: "); Serial.println(TRACK_TEMP.readMax);
        Serial.print("READ AVG: "); Serial.println(TRACK_TEMP.readAvg);
        Serial.print("READ MIN: "); Serial.println(TRACK_TEMP.readMin);

        analogToSensorVal(TRACK_TEMP);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = TRACK_TEMP.actualMax;
        msg.buf[2] = TRACK_TEMP.actualMax >> 8;
        msg.buf[3] = TRACK_TEMP.actualAvg;
        msg.buf[4] = TRACK_TEMP.actualAvg >> 8;
        msg.buf[5] = TRACK_TEMP.actualMin;
        msg.buf[6] = TRACK_TEMP.actualMin >> 8;
        msg.buf[7] = 0;
        sendCAN(0x00, 8, 0);

        Serial.println();
        Serial.print("CALC MAX: "); Serial.println(TRACK_TEMP.actualMax);
        Serial.print("CALC AVG: "); Serial.println(TRACK_TEMP.actualAvg);
        Serial.print("CALC MIN: "); Serial.println(TRACK_TEMP.actualMin);


        analogToSensorVal(FR_BRAKE_PRESSURE);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = FR_BRAKE_PRESSURE.actualMax;
        msg.buf[2] = FR_BRAKE_PRESSURE.actualMax >> 8;
        msg.buf[3] = FR_BRAKE_PRESSURE.actualAvg;
        msg.buf[4] = FR_BRAKE_PRESSURE.actualAvg >> 8;
        msg.buf[5] = FR_BRAKE_PRESSURE.actualMin;
        msg.buf[6] = FR_BRAKE_PRESSURE.actualMin >> 8;
        msg.buf[7] = 0;
        sendCAN(0x00, 8, 0);


        analogToSensorVal(FL_BRAKE_PRESSURE);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = FL_BRAKE_PRESSURE.actualMax;
        msg.buf[2] = FL_BRAKE_PRESSURE.actualMax >> 8;
        msg.buf[3] = FL_BRAKE_PRESSURE.actualAvg;
        msg.buf[4] = FL_BRAKE_PRESSURE.actualAvg >> 8;
        msg.buf[5] = FL_BRAKE_PRESSURE.actualMin;
        msg.buf[6] = FL_BRAKE_PRESSURE.actualMin >> 8;
        msg.buf[7] = 0;
        sendCAN(0x00, 8, 0);


        analogToSensorVal(RR_BRAKE_PRESSURE);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = RR_BRAKE_PRESSURE.actualMax;
        msg.buf[2] = RR_BRAKE_PRESSURE.actualMax >> 8;
        msg.buf[3] = RR_BRAKE_PRESSURE.actualAvg;
        msg.buf[4] = RR_BRAKE_PRESSURE.actualAvg >> 8;
        msg.buf[5] = RR_BRAKE_PRESSURE.actualMin;
        msg.buf[6] = RR_BRAKE_PRESSURE.actualMin >> 8;
        msg.buf[7] = 0;
        sendCAN(0x00, 8, 0);


        analogToSensorVal(RL_BRAKE_PRESSURE);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = RL_BRAKE_PRESSURE.actualMax;
        msg.buf[2] = RL_BRAKE_PRESSURE.actualMax >> 8;
        msg.buf[3] = RL_BRAKE_PRESSURE.actualAvg;
        msg.buf[4] = RL_BRAKE_PRESSURE.actualAvg >> 8;
        msg.buf[5] = RL_BRAKE_PRESSURE.actualMin;
        msg.buf[6] = RL_BRAKE_PRESSURE.actualMin >> 8;
        msg.buf[7] = 0;
        sendCAN(0x00, 8, 0);


        analogToSensorVal(WATER_TEMP_BETWEEN_RADS);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = WATER_TEMP_BETWEEN_RADS.actualMax;
        msg.buf[2] = WATER_TEMP_BETWEEN_RADS.actualMax >> 8;
        msg.buf[3] = WATER_TEMP_BETWEEN_RADS.actualAvg;
        msg.buf[4] = WATER_TEMP_BETWEEN_RADS.actualAvg >> 8;
        msg.buf[5] = WATER_TEMP_BETWEEN_RADS.actualMin;
        msg.buf[6] = WATER_TEMP_BETWEEN_RADS.actualMin >> 8;
        msg.buf[7] = 0;
        sendCAN(0x00, 8, 0);


        analogToSensorVal(FR_ROTOR_TEMP);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = FR_ROTOR_TEMP.actualMax;
        msg.buf[2] = FR_ROTOR_TEMP.actualMax >> 8;
        msg.buf[3] = FR_ROTOR_TEMP.actualAvg;
        msg.buf[4] = FR_ROTOR_TEMP.actualAvg >> 8;
        msg.buf[5] = FR_ROTOR_TEMP.actualMin;
        msg.buf[6] = FR_ROTOR_TEMP.actualMin >> 8;
        msg.buf[7] = 0;
        sendCAN(0x00, 8, 0);


        analogToSensorVal(FL_ROTOR_TEMP);
        msg.buf[0] = messageCount100Hz; // counter
        msg.buf[1] = FL_ROTOR_TEMP.actualMax;
        msg.buf[2] = FL_ROTOR_TEMP.actualMax >> 8;
        msg.buf[3] = FL_ROTOR_TEMP.actualAvg;
        msg.buf[4] = FL_ROTOR_TEMP.actualAvg >> 8;
        msg.buf[5] = FL_ROTOR_TEMP.actualMin;
        msg.buf[6] = FL_ROTOR_TEMP.actualMin >> 8;
        msg.buf[7] = 0;
        sendCAN(0x00, 8, 0);


      } // end 100Hz timer messages
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