//------------------------------------------------------------------------------
//
//       ██████╗  ██████╗      ██████╗ ██████╗ ███████╗███████╗███╗   ██╗
//      ██╔════╝ ██╔═══██╗    ██╔════╝ ██╔══██╗██╔════╝██╔s════╝████╗  ██║
//      ██║  ███╗██║   ██║    ██║  ███╗██████╔╝█████╗  █████╗  ██╔██╗ ██║
//      ██║   ██║██║   ██║    ██║   ██║██╔══██╗██╔══╝  ██╔══╝  ██║╚██╗██║
//      ╚██████╔╝╚██████╔╝    ╚██████╔╝██║  ██║███████╗███████╗██║ ╚████║
//       ╚═════╝  ╚═════╝      ╚═════╝ ╚═╝  ╚═╝╚══════╝╚══════╝╚═╝  ╚═══╝
//
//------------------------------------------------------------------------------
//  Written by:     Dave Yonkers!
//  Created:        03/04/2019
//  Purpose:        Send some 1s and 0s over a couple of twisted wires
//  Description:    Analog-to-CAN converter code
//------------------------------------------------------------------------------


// IMPORTANT!!!
// before uploading, update the "ATCC" definition to the correct module
// possible values:   0 : FRONT
//                    1 : REAR
#define ATCC 1


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

// variables for message counters
int messageCount1000Hz = 0;
int messageCount500Hz  = 0;
int messageCount200Hz  = 0;
int messageCount100Hz  = 0;
int messageCount50Hz   = 0;
int messageCount20Hz   = 0;
int messageCount10Hz   = 0;
int messageCount1Hz    = 0;

// initialize a timer and variable for the LED
unsigned long LEDTimer          = 0;
bool LED_on                     = false;


// structure for sensor reading
typedef struct
{

  // used to tell the status of the module
  uint8_t deviceStatus;

  String sensName   = "";
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
  double mV10unit   = 0; // in mV*10 (mV per unit, ex. 30 could be 30mV/degree C)
  double scaleFact  = 0.1; // like in the DBC (.1, .01, .001 etc.)
  double z1         = 1200.0000; // z1 & z2 are for the voltage divider (units are ohms) (default is for 5V)
  double z2         = 2200.0000; // check the wikipedia page for a diagram en.wikipedia.org/wiki/Voltage_divider
                                  // if you want to change these per sensor, do it in calibration in the setup loop


} sensor;


// front ATCC
sensor FR_DAMPER_POS, FL_DAMPER_POS, TRACK_TEMP, FR_BRAKE_PRESSURE;
sensor FL_BRAKE_PRESSURE, RR_BRAKE_PRESSURE, RL_BRAKE_PRESSURE;
sensor WATER_TEMP_BETWEEN_RADS, FR_ROTOR_TEMP, FL_ROTOR_TEMP;
sensor TIRETEMP_FL_I, TIRETEMP_FL_M, TIRETEMP_FL_O; // inner, middle, outer
sensor TIRETEMP_FR_I, TIRETEMP_FR_M, TIRETEMP_FR_O; // inner, middle, outer

// rear ATCC
sensor RR_DAMPER_POS, RL_DAMPER_POS, RIGHT_RAD_TEMP, LEFT_RAD_TEMP;
sensor RR_ROTOR_TEMP, RL_ROTOR_TEMP;
sensor TIRETEMP_RL_I, TIRETEMP_RL_M, TIRETEMP_RL_O; // inner, middle, outer
sensor TIRETEMP_RR_I, TIRETEMP_RR_M, TIRETEMP_RR_O; // inner, middle, outer

// set the analog read resolution in bits (10 bits yeild an input 0-1023, etc.)
// initialize a variable for a calculation of the readMaximum read value from pins
const int read_resolution_bits = 13; // <--- modify this one
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
      FR_DAMPER_POS.sensName    = "FR_DAMPER_POS";
      FR_DAMPER_POS.pin         = A0;
      FR_DAMPER_POS.zeroMVolt10 = 0; // mV*10
      FR_DAMPER_POS.mV10unit    = 0; // mV*10 per sensor unit
      FR_DAMPER_POS.scaleFact   = 0.1;
      // FR_DAMPER_POS.z1          = 6450;
      // FR_DAMPER_POS.z2          = 0;

      FL_DAMPER_POS.sensName    = "FL_DAMPER_POS";
      FL_DAMPER_POS.pin         = A1;
      FL_DAMPER_POS.zeroMVolt10 = 0; // mV*10
      FL_DAMPER_POS.mV10unit    = 0; // mV*10 per sensor unit
      FL_DAMPER_POS.scaleFact   = 0.1;
      // FL_DAMPER_POS.z1          = 0;
      // FL_DAMPER_POS.z2          = 0;

      TRACK_TEMP.sensName    = "TRACK_TEMP";
      TRACK_TEMP.pin         = A2;
      TRACK_TEMP.zeroMVolt10 = 400; // mV*10
      TRACK_TEMP.mV10unit    = 300.0000; // mV*10 per sensor unit
      TRACK_TEMP.scaleFact   = 0.1;
      // TRACK_TEMP.z1          = 0;
      // TRACK_TEMP.z2          = 0;

      FR_BRAKE_PRESSURE.sensName    = "FR_BRAKE_PRESSURE";
      FR_BRAKE_PRESSURE.pin         = A13;
      FR_BRAKE_PRESSURE.zeroMVolt10 = 5000; // mV*10 (.5V)
      FR_BRAKE_PRESSURE.mV10unit    = 20; // mV*10 per sensor unit (Honeywell MLH2000PGB06A) Sens range .5V to 4.5V; 0psi to 2000psi
      FR_BRAKE_PRESSURE.scaleFact   = 0.1;
      // FR_BRAKE_PRESSURE.z1          = 0;
      // FR_BRAKE_PRESSURE.z2          = 0;

      FL_BRAKE_PRESSURE.sensName    = "FL_BRAKE_PRESSURE";
      FL_BRAKE_PRESSURE.pin         = A16;
      FL_BRAKE_PRESSURE.zeroMVolt10 = 4772; // mV*10
      FL_BRAKE_PRESSURE.mV10unit    = 20; // mV*10 per sensor unit
      FL_BRAKE_PRESSURE.scaleFact   = 0.1;
      // FL_BRAKE_PRESSURE.z1          = 6400;
      // FL_BRAKE_PRESSURE.z2          = 0;

      RR_BRAKE_PRESSURE.sensName    = "RR_BRAKE_PRESSURE";
      RR_BRAKE_PRESSURE.pin         = A4;
      RR_BRAKE_PRESSURE.zeroMVolt10 = 5000; // mV*10
      RR_BRAKE_PRESSURE.mV10unit    = 20; // mV*10 per sensor unit
      RR_BRAKE_PRESSURE.scaleFact   = 0.1;
      // RR_BRAKE_PRESSURE.z1          = 0;
      // RR_BRAKE_PRESSURE.z2          = 0;

      RL_BRAKE_PRESSURE.sensName    = "RL_BRAKE_PRESSURE";
      RL_BRAKE_PRESSURE.pin         = A5;
      RL_BRAKE_PRESSURE.zeroMVolt10 = 5000; // mV*10
      RL_BRAKE_PRESSURE.mV10unit    = 20; // mV*10 per sensor unit
      RL_BRAKE_PRESSURE.scaleFact   = 0.1;
      // RL_BRAKE_PRESSURE.z1          = 0;
      // RL_BRAKE_PRESSURE.z2          = 0;

      WATER_TEMP_BETWEEN_RADS.sensName    = "WATER_TEMP_BETWEEN_RADS";
      WATER_TEMP_BETWEEN_RADS.pin         = A19;
      WATER_TEMP_BETWEEN_RADS.scaleFact   = 0.1;

      FR_ROTOR_TEMP.sensName    = "FR_ROTOR_TEMP";
      FR_ROTOR_TEMP.pin         = A12;
      FR_ROTOR_TEMP.zeroMVolt10 = 5000; // mV*10
      FR_ROTOR_TEMP.mV10unit    = 50; // mV*10 per sensor unit
      FR_ROTOR_TEMP.scaleFact   = 0.1;
      // FR_ROTOR_TEMP.z1          = 0;
      // FR_ROTOR_TEMP.z2          = 0;

      FL_ROTOR_TEMP.sensName    = "FL_ROTOR_TEMP";
      FL_ROTOR_TEMP.pin         = A15;
      FL_ROTOR_TEMP.zeroMVolt10 = 5000; // mV*10
      FL_ROTOR_TEMP.mV10unit    = 50; // mV*10 per sensor unit
      FL_ROTOR_TEMP.scaleFact   = 0.1;
      // FR_ROTOR_TEMP.z1          = 0;
      // FR_ROTOR_TEMP.z2          = 0;

      // CHANGE THE PINS BEFORE LOADING

      TIRETEMP_FL_I.sensName    = "TIRETEMP_FL_I";
      TIRETEMP_FL_I.pin         = A8;
      TIRETEMP_FL_I.zeroMVolt10 = 4000; // mV*10
      TIRETEMP_FL_I.mV10unit    = 300; // mV*10 per sensor unit
      TIRETEMP_FL_I.scaleFact   = 0.1;
      // TIRETEMP_FL_I.z1          = 0;
      // TIRETEMP_FL_I.z2          = 0;

      TIRETEMP_FL_M.sensName    = "TIRETEMP_FL_M";
      TIRETEMP_FL_M.pin         = A11;
      TIRETEMP_FL_M.zeroMVolt10 = 4000; // mV*10
      TIRETEMP_FL_M.mV10unit    = 300; // mV*10 per sensor unit
      TIRETEMP_FL_M.scaleFact   = 0.1;
      // TIRETEMP_FL_M.z1          = 0;
      // TIRETEMP_FL_M.z2          = 0;

      TIRETEMP_FL_O.sensName    = "TIRETEMP_FL_O";
      TIRETEMP_FL_O.pin         = A9;
      TIRETEMP_FL_O.zeroMVolt10 = 4000; // mV*10
      TIRETEMP_FL_O.mV10unit    = 300; // mV*10 per sensor unit
      TIRETEMP_FL_O.scaleFact   = 0.1;
      // TIRETEMP_FL_O.z1          = 0;
      // TIRETEMP_FL_O.z2          = 0;

      TIRETEMP_FR_I.sensName    = "TIRETEMP_FR_I";
      TIRETEMP_FR_I.pin         = A7;
      TIRETEMP_FR_I.zeroMVolt10 = 4000; // mV*10
      TIRETEMP_FR_I.mV10unit    = 300; // mV*10 per sensor unit
      TIRETEMP_FR_I.scaleFact   = 0.1;
      // TIRETEMP_FR_I.z1          = 0;
      // TIRETEMP_FR_I.z2          = 0;

      TIRETEMP_FR_M.sensName    = "TIRETEMP_FR_M";
      TIRETEMP_FR_M.pin         = A13;
      TIRETEMP_FR_M.zeroMVolt10 = 4000; // mV*10
      TIRETEMP_FR_M.mV10unit    = 300; // mV*10 per sensor unit
      TIRETEMP_FR_M.scaleFact   = 0.1;
      // TIRETEMP_FR_M.z1          = 0;
      // TIRETEMP_FR_M.z2          = 0;

      TIRETEMP_FR_O.sensName    = "TIRETEMP_FR_O";
      TIRETEMP_FR_O.pin         = A6;
      TIRETEMP_FR_O.zeroMVolt10 = 4000; // mV*10
      TIRETEMP_FR_O.mV10unit    = 300; // mV*10 per sensor unit
      TIRETEMP_FR_O.scaleFact   = 0.1;
      // TIRETEMP_FR_O.z1          = 0;
      // TIRETEMP_FR_O.z2          = 0;




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
      pinMode(TIRETEMP_FL_I.pin,            INPUT);
      pinMode(TIRETEMP_FL_M.pin,            INPUT);
      pinMode(TIRETEMP_FL_O.pin,            INPUT);
      pinMode(TIRETEMP_FR_I.pin,            INPUT);
      pinMode(TIRETEMP_FR_M.pin,            INPUT);
      pinMode(TIRETEMP_FR_O.pin,            INPUT);

      break;


    // REAR ATCC
    case 1:

      // sensor calibration
      RR_DAMPER_POS.sensName    = "RR_DAMPER_POS";
      RR_DAMPER_POS.pin         = A0;
      RR_DAMPER_POS.zeroMVolt10 = 0; // mV*10
      RR_DAMPER_POS.mV10unit    = 0; // mV*10 per sensor unit
      RR_DAMPER_POS.scaleFact   = 0.1;

      RL_DAMPER_POS.sensName    = "RL_DAMPER_POS";
      RL_DAMPER_POS.pin         = A1;
      RL_DAMPER_POS.zeroMVolt10 = 0; // mV*10
      RL_DAMPER_POS.mV10unit    = 0; // mV*10 per sensor unit
      RL_DAMPER_POS.scaleFact   = 0.1;

      RIGHT_RAD_TEMP.sensName    = "RIGHT_RAD_TEMP";
      RIGHT_RAD_TEMP.pin         = A19;
      RIGHT_RAD_TEMP.scaleFact   = 0.1;

      LEFT_RAD_TEMP.sensName    = "LEFT_RAD_TEMP";
      LEFT_RAD_TEMP.pin         = A18;
      LEFT_RAD_TEMP.scaleFact   = 0.1;

      RL_ROTOR_TEMP.sensName    = "RL_ROTOR_TEMP";
      RR_ROTOR_TEMP.pin         = A15;
      RR_ROTOR_TEMP.zeroMVolt10 = 5000; // mV*10
      RR_ROTOR_TEMP.mV10unit    = 50; // mV*10 per sensor unit
      RR_ROTOR_TEMP.scaleFact   = 0.1;

      RR_ROTOR_TEMP.sensName    = "RR_ROTOR_TEMP";
      RR_ROTOR_TEMP.pin         = A12;
      RR_ROTOR_TEMP.zeroMVolt10 = 5000; // mV*10
      RR_ROTOR_TEMP.mV10unit    = 50; // mV*10 per sensor unit
      RR_ROTOR_TEMP.scaleFact   = 0.1;

      TIRETEMP_RL_I.sensName    = "TIRETEMP_RL_I";
      TIRETEMP_RL_I.pin         = A13;
      TIRETEMP_RL_I.zeroMVolt10 = 4000; // mV*10
      TIRETEMP_RL_I.mV10unit    = 300; // mV*10 per sensor unit
      TIRETEMP_RL_I.scaleFact   = 0.1;
      // TIRETEMP_FL_I.z1          = 0;
      // TIRETEMP_FL_I.z2          = 0;

      TIRETEMP_RL_M.sensName    = "TIRETEMP_RL_M";
      TIRETEMP_RL_M.pin         = A10;
      TIRETEMP_RL_M.zeroMVolt10 = 4000; // mV*10
      TIRETEMP_RL_M.mV10unit    = 300; // mV*10 per sensor unit
      TIRETEMP_RL_M.scaleFact   = 0.1;
      // TIRETEMP_RL_M.z1          = 0;
      // TIRETEMP_RL_M.z2          = 0;

      TIRETEMP_RL_O.sensName    = "TIRETEMP_RL_O";
      TIRETEMP_RL_O.pin         = A9;
      TIRETEMP_RL_O.zeroMVolt10 = 4000; // mV*10
      TIRETEMP_RL_O.mV10unit    = 300; // mV*10 per sensor unit
      TIRETEMP_RL_O.scaleFact   = 0.1;
      // TIRETEMP_RL_O.z1          = 0;
      // TIRETEMP_RL_O.z2          = 0;

      TIRETEMP_RR_I.sensName    = "TIRETEMP_RR_I";
      TIRETEMP_RR_I.pin         = A16;
      TIRETEMP_RR_I.zeroMVolt10 = 4000; // mV*10
      TIRETEMP_RR_I.mV10unit    = 300; // mV*10 per sensor unit
      TIRETEMP_RR_I.scaleFact   = 0.1;
      // TIRETEMP_RR_I.z1          = 0;
      // TIRETEMP_RR_I.z2          = 0;

      TIRETEMP_RR_M.sensName    = "TIRETEMP_RR_M";
      TIRETEMP_RR_M.pin         = A11;
      TIRETEMP_RR_M.zeroMVolt10 = 4000; // mV*10
      TIRETEMP_RR_M.mV10unit    = 300; // mV*10 per sensor unit
      TIRETEMP_RR_M.scaleFact   = 0.1;
      // TIRETEMP_RR_M.z1          = 0;
      // TIRETEMP_RR_M.z2          = 0;

      TIRETEMP_RR_O.sensName    = "TIRETEMP_RR_O";
      TIRETEMP_RR_O.pin         = A8;
      TIRETEMP_RR_O.zeroMVolt10 = 4000; // mV*10
      TIRETEMP_RR_O.mV10unit    = 300; // mV*10 per sensor unit
      TIRETEMP_RR_O.scaleFact   = 0.1;
      // TIRETEMP_RR_O.z1          = 0;
      // TIRETEMP_RR_O.z2          = 0;



      // pin more initialization
      pinMode(13,                           OUTPUT); // Onboard LED
      pinMode(RR_DAMPER_POS.pin,            INPUT);
      pinMode(RL_DAMPER_POS.pin,            INPUT);
      pinMode(RIGHT_RAD_TEMP.pin,           INPUT);
      pinMode(LEFT_RAD_TEMP.pin,            INPUT);
      pinMode(RL_ROTOR_TEMP.pin,            INPUT);
      pinMode(RR_ROTOR_TEMP.pin,            INPUT);
      pinMode(TIRETEMP_FL_I.pin,            INPUT);
      pinMode(TIRETEMP_FL_M.pin,            INPUT);
      pinMode(TIRETEMP_FL_O.pin,            INPUT);
      pinMode(TIRETEMP_FR_I.pin,            INPUT);
      pinMode(TIRETEMP_FR_M.pin,            INPUT);
      pinMode(TIRETEMP_FR_O.pin,            INPUT);


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

      // read the sensors in this timer at 2,000 Hz
      if ( micros() - SensTimer2000Hz >= 500 )
      {
        SensTimer2000Hz = micros();

        //read the sensors
        // analogReadSensor(FR_DAMPER_POS); - disabled, not set up
        // analogReadSensor(FL_DAMPER_POS); - disabled, not set up
      }


      // read the sensors in this timer at 200 Hz
      if (millis() - SensTimer200Hz >= 5)
      {
        SensTimer200Hz = millis();

        // read the sensors
        analogReadSensor(FL_BRAKE_PRESSURE);
        // analogReadSensor(FR_BRAKE_PRESSURE); -- disabled until ABS
        // analogReadSensor(RL_BRAKE_PRESSURE); -- disabled due to faulty sensor
        // analogReadSensor(RR_BRAKE_PRESSURE); -- disabled until ABS
      }



      // read the sensors in this timer at 100 Hz
      if (millis() - SensTimer100Hz >= 10)
      {
        SensTimer100Hz = millis();

        // read the sensors
        //analogReadSensor(FR_ROTOR_TEMP);
        //analogReadSensor(FL_ROTOR_TEMP);
      }



      // read the sensors in this timer at 50 Hz
      if (millis() - SensTimer50Hz >= 20)
      {
        SensTimer50Hz = millis();

        // read the sensors
        analogReadSensor(WATER_TEMP_BETWEEN_RADS);
        // analogReadSensor(TRACK_TEMP); - disabled, not set up
        analogReadSensor(TIRETEMP_FL_I);
        analogReadSensor(TIRETEMP_FL_M);
        analogReadSensor(TIRETEMP_FL_O);
        analogReadSensor(TIRETEMP_FR_I);
        analogReadSensor(TIRETEMP_FR_M);
        analogReadSensor(TIRETEMP_FR_O);
      }





      // continually launch the calculateAndLaunchCAN function, as it
      // has indivdual timers built in.
      calculateAndLaunchCAN();

      break;






    // main loop for rear ATCC
    case 1:

      if (millis() - SensTimer50Hz >= 20) // was 20
      {
        SensTimer50Hz = millis();

        // read the sensors
        analogReadSensor(RIGHT_RAD_TEMP);
        analogReadSensor(LEFT_RAD_TEMP);
        analogReadSensor(RL_DAMPER_POS);
        analogReadSensor(RR_DAMPER_POS);
        analogReadSensor(RL_ROTOR_TEMP);
        analogReadSensor(RR_ROTOR_TEMP);
        analogReadSensor(TIRETEMP_RL_I);
        analogReadSensor(TIRETEMP_RL_M);
        analogReadSensor(TIRETEMP_RL_O);
        analogReadSensor(TIRETEMP_RR_I);
        analogReadSensor(TIRETEMP_RR_M);
        analogReadSensor(TIRETEMP_RR_O);
      }

      // continually launch the calculateAndLaunchCAN function, as it
      // has indivdual timers built in.
      calculateAndLaunchCAN();

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

  // determine if its a min or max (DISABLED)
  // if      ( SENSOR.readVal < SENSOR.readMin ){ SENSOR.readMin = SENSOR.readVal; }
  // else if ( SENSOR.readVal > SENSOR.readMax ){ SENSOR.readMax = SENSOR.readVal; }

  // add to the average and the counter
  SENSOR.readAvg = SENSOR.readAvg + SENSOR.readVal;
  SENSOR.count++;

  // uncomment to read raw values
  //Serial.print(SENSOR.sensName); Serial.print(" analog reads: "); Serial.println(SENSOR.readVal);


}











void analogToSensorVal( sensor &SENSOR )
// takes raw sensor data and turns them into the human-readable
// numbers to send over CAN (MAX AND MIN ARE DISABLED)
{
  // divide the running total for average by the count, therefore calculating
  // the true raw average
  SENSOR.readAvg /= SENSOR.count;



  // convert the analog inputs into the teeny voltage (mV*10)
  float sensAvg = SENSOR.readAvg * (teensy_voltage_mV10 / read_resolution);
  // float sensMin = SENSOR.readMin * (teensy_voltage_mV10 / read_resolution);
  // float sensMax = SENSOR.readMax * (teensy_voltage_mV10 / read_resolution);



  // convert the teensy voltage (3.3V) to sensor voltage by doing the opposite
  // operations of a voltage divider (in ohms). For reference: en.wikipedia.org/wiki/Voltage_divider
  sensAvg = sensAvg / SENSOR.z2 * (SENSOR.z2 + SENSOR.z1);
  // sensMin = sensMin / SENSOR.z2 * (SENSOR.z2 + SENSOR.z1);
  // sensMax = sensMax / SENSOR.z2 * (SENSOR.z2 + SENSOR.z1);

  // print voltages for calibration
  // Serial.print(SENSOR.sensName); Serial.print(" sensor voltage: "); Serial.println(sensAvg);

  // sensor calibration (convert sensor voltage to CAN values)
  //                   zero volt of sens.    units per mV*10                inverse of the scale factor.
  sensAvg = (sensAvg - SENSOR.zeroMVolt10) * ( 1.0000 / SENSOR.mV10unit ) * ( 1.0000 / SENSOR.scaleFact );
  // sensMin = (sensMin - SENSOR.zeroMVolt10) * ( 1.0000 / SENSOR.mV10unit ) * ( 1.0000 / SENSOR.scaleFact );
  // sensMax = (sensMax - SENSOR.zeroMVolt10) * ( 1.0000 / SENSOR.mV10unit ) * ( 1.0000 / SENSOR.scaleFact );



  // put the final values into the sensor's structured variabels
  SENSOR.actualAvg = (int)sensAvg;
  // SENSOR.actualMin = (int)sensMin;
  // SENSOR.actualMax = (int)sensMax;

  // print final values for calibration
  Serial.print(SENSOR.sensName); Serial.print(" sensor value: "); Serial.println(SENSOR.actualAvg);


  // calculations are done, so reset the sensor raw read values
  resetSensor(SENSOR);
}











void analogToBoschTempVal( sensor &SENSOR )
// takes raw sensor data and turns them into the human-readable
// numbers to send over CAN (MAX AND MIN ARE DISABLED)
{
  // divide the running total for average by the count, therefore calculating
  // the true raw average
  SENSOR.readAvg /= SENSOR.count;


  // convert the analog inputs into the teeny voltage (mV*10)
  float sensAvg = SENSOR.readAvg * (teensy_voltage_mV10 / read_resolution);
  // float sensMin = SENSOR.readMin * (teensy_voltage_mV10 / read_resolution);
  // float sensMax = SENSOR.readMax * (teensy_voltage_mV10 / read_resolution);


  // turn mV*10 values into straight up V
  sensAvg /= 10000;
  // sensMin /= 10000;
  // sensMax /= 10000;


  // sensor calibration (convert sensor voltage to celsius values)
  // this function was calculated in Excel ಠ_ಠ using the voltage divider resistor
  // pair for our setup, and the table of values in the Bosch sensor datasheet
  sensAvg = 4.129 * pow(sensAvg, 4) - 40.226 * pow(sensAvg, 3) + 129.61 * pow(sensAvg, 2) -
            197.54 * sensAvg + 151.25;
  // sensMin = 4.129 * pow(sensMin, 4) - 40.226 * pow(sensMin, 3) + 129.61 * pow(sensMin, 2) -
  //           197.54 * sensMin + 151.25;
  // sensMax = 4.129 * pow(sensMax, 4) - 40.226 * pow(sensMax, 3) + 129.61 * pow(sensMax, 2) -
  //           197.54 * sensMax + 151.25;


  // multiply the calculated temperatures by their factors for CAN
  sensAvg *= (1.0000 / SENSOR.scaleFact);
  // sensMin *= (1.0000 / SENSOR.scaleFact)
  // sensMax *= (1.0000 / SENSOR.scaleFact)



  // put the final values into the sensor's structured variables
  SENSOR.actualAvg = (int)sensAvg;
  // SENSOR.actualMin = (int)sensMin;
  // SENSOR.actualMax = (int)sensMax;


  // calculations are done, so reset the sensor raw read values
  resetSensor(SENSOR);
}











void resetSensor( sensor &SENSOR )
// resets the sensor mins, maxs, avgs, and count
{
  // (MAX AND MIN ARE DISABLED)
  // SENSOR.readMin =  2147483647; // maximum number possible
  // SENSOR.readMax = -2147483647; // minimum number possible
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
    just_send_it_bro(0x50, 8, 0);
   *
   */

   switch ( ATCC )
   {
    // front ATCC
    case 0:

      // messages to be sent at 200 Hz
      if ( millis() - SendTimer200Hz >= 5 )
      {
        SendTimer200Hz = millis();

        // add one for every cycle through. Reset after 14
        if ( messageCount200Hz < 15 ){messageCount200Hz++;}
        else {messageCount200Hz = 0;}


        // ATCCF_02
        // analogToSensorVal(FL_DAMPER_POS);
        // analogToSensorVal(FR_DAMPER_POS);
        msg.buf[0] = messageCount200Hz;
        msg.buf[1] = 0;//FL_DAMPER_POS.actualAvg; -- disabled, not in use
        msg.buf[2] = 0;//FL_DAMPER_POS.actualAvg >> 8;
        msg.buf[3] = 0;//FR_DAMPER_POS.actualAvg; -- disabled, not in use
        msg.buf[4] = 0;//FR_DAMPER_POS.actualAvg >> 8;
        msg.buf[5] = 0;
        msg.buf[6] = 0;
        msg.buf[7] = 0;
        sendCAN(0x8E, 8, 1);

      } // end 200Hz timer messages



      // messages to be sent at 50 Hz
      if ( millis() - SendTimer50Hz >= 20 )
      {
        SendTimer50Hz = millis();

        // add one for every cycle through. Reset after 14
        if ( messageCount50Hz < 15 ){messageCount50Hz++;}
        else {messageCount50Hz = 0;}


        // ATCCF_00
        // turn the raw numbers into the ones we can read over CAN.
        analogToSensorVal(FL_BRAKE_PRESSURE);
        analogToSensorVal(FR_BRAKE_PRESSURE);
        // analogToSensorVal(TRACK_TEMP);
        // put the results into a message buffer
        msg.buf[0] = messageCount50Hz; // counter
        msg.buf[1] = FL_BRAKE_PRESSURE.actualAvg;
        msg.buf[2] = FL_BRAKE_PRESSURE.actualAvg >> 8;
        msg.buf[3] = 0;//FR_BRAKE_PRESSURE.actualAvg; -- disabled until ABS
        msg.buf[4] = 0;//FR_BRAKE_PRESSURE.actualAvg >> 8;
        msg.buf[5] = 0;//TRACK_TEMP.actualAvg; -- disabled, not in use
        msg.buf[6] = 0;//TRACK_TEMP.actualAvg >> 8;
        msg.buf[7] = 0;
        // send the message
        sendCAN(0x8C, 8, 1);


        // ATCCF_01
        // analogToSensorVal(RL_BRAKE_PRESSURE);
        // analogToSensorVal(RR_BRAKE_PRESSURE);
        // analogToBoschTempVal(WATER_TEMP_BETWEEN_RADS);
        // msg.buf[0] = messageCount50Hz;
        // msg.buf[1] = 0;//RL_BRAKE_PRESSURE.actualAvg; -- disabled due to faulty sensor
        // msg.buf[2] = 0;//RL_BRAKE_PRESSURE.actualAvg >> 8;
        // msg.buf[3] = 0;//RR_BRAKE_PRESSURE.actualAvg; -- disabled until ABS
        // msg.buf[4] = 0;//RR_BRAKE_PRESSURE.actualAvg >> 8;
        // msg.buf[5] = 0; //WATER_TEMP_BETWEEN_RADS.actualAvg;
        // msg.buf[6] = 0; //WATER_TEMP_BETWEEN_RADS.actualAvg >> 8;
        // msg.buf[7] = 0;
        // sendCAN(0x8D, 8, 1);

        //ATCCF_04
        analogToSensorVal(TIRETEMP_FL_I);
        analogToSensorVal(TIRETEMP_FL_M);
        analogToSensorVal(TIRETEMP_FL_O);
        msg.buf[0] = TIRETEMP_FL_I.actualAvg;
        msg.buf[1] = TIRETEMP_FL_I.actualAvg >> 8;
        msg.buf[2] = TIRETEMP_FL_M.actualAvg;
        msg.buf[3] = TIRETEMP_FL_M.actualAvg >> 8;
        msg.buf[4] = TIRETEMP_FL_O.actualAvg;
        msg.buf[5] = TIRETEMP_FL_O.actualAvg >> 8;
        msg.buf[6] = 0;
        msg.buf[7] = 0;
        sendCAN(0x92, 8, 1);

        //ATCCF_05
        analogToSensorVal(TIRETEMP_FR_I);
        analogToSensorVal(TIRETEMP_FR_M);
        analogToSensorVal(TIRETEMP_FR_O);
        msg.buf[0] = TIRETEMP_FR_I.actualAvg;
        msg.buf[1] = TIRETEMP_FR_I.actualAvg >> 8;
        msg.buf[2] = TIRETEMP_FR_M.actualAvg;
        msg.buf[3] = TIRETEMP_FR_M.actualAvg >> 8;
        msg.buf[4] = TIRETEMP_FR_O.actualAvg;
        msg.buf[5] = TIRETEMP_FR_O.actualAvg >> 8;
        msg.buf[6] = 0;
        msg.buf[7] = 0;
        sendCAN(0x93, 8, 1);


      } // end 50 Hz messages


      // messages to be sent at 10 Hz
      if ( millis() - SendTimer10Hz >= 100 )
      {
        SendTimer10Hz = millis();

        // add one for every cycle through. Reset after 14
        if ( messageCount10Hz < 15 ){messageCount10Hz++;}
        else {messageCount10Hz = 0;}

        // ATCCF_03
        analogToSensorVal(FL_ROTOR_TEMP);
        analogToSensorVal(FR_ROTOR_TEMP);
        msg.buf[0] = messageCount10Hz;
        msg.buf[1] = FL_ROTOR_TEMP.actualAvg;
        msg.buf[2] = FL_ROTOR_TEMP.actualAvg >> 8;
        msg.buf[3] = FR_ROTOR_TEMP.actualAvg;
        msg.buf[4] = FR_ROTOR_TEMP.actualAvg >> 8;
        msg.buf[5] = 0;
        msg.buf[6] = 0;
        msg.buf[7] = 0;
        sendCAN(0x8F, 8, 1);

      } // end 10 Hz messages


      break;




    // rear ATCC
    case 1:

      if ( millis() - SendTimer100Hz >= 10 )
      {
        SendTimer100Hz = millis();

        // add one for every cycle through. Reset after 14
        if ( messageCount100Hz < 15 ){messageCount100Hz++;}
        else {messageCount100Hz = 0;}



        analogToSensorVal(RL_DAMPER_POS);
        analogToSensorVal(RR_DAMPER_POS);
        analogToBoschTempVal(LEFT_RAD_TEMP);
        msg.buf[0] = messageCount100Hz;
        msg.buf[1] = RL_DAMPER_POS.actualAvg;
        msg.buf[2] = RL_DAMPER_POS.actualAvg >> 8;
        msg.buf[3] = RR_DAMPER_POS.actualAvg;
        msg.buf[4] = RR_DAMPER_POS.actualAvg >> 8;
        msg.buf[5] = LEFT_RAD_TEMP.actualAvg;
        msg.buf[6] = LEFT_RAD_TEMP.actualAvg >> 8;
        msg.buf[7] = 0;
        sendCAN(0x4, 8, 1);


        analogToSensorVal(RL_ROTOR_TEMP);
        analogToSensorVal(RR_ROTOR_TEMP);
        analogToBoschTempVal(RIGHT_RAD_TEMP);
        msg.buf[0] = messageCount100Hz;
        msg.buf[1] = RL_ROTOR_TEMP.actualAvg;
        msg.buf[2] = RL_ROTOR_TEMP.actualAvg >> 8;
        msg.buf[3] = RR_ROTOR_TEMP.actualAvg;
        msg.buf[4] = RR_ROTOR_TEMP.actualAvg >> 8;
        msg.buf[5] = RIGHT_RAD_TEMP.actualAvg;
        msg.buf[6] = RIGHT_RAD_TEMP.actualAvg >> 8;
        msg.buf[7] = 0;
        sendCAN(0x5, 8, 1);

      } // end 100Hz timer messages

      if ( millis() - SendTimer50Hz >= 20 ) // changed for testing
      {
        SendTimer50Hz = millis();


        //ATCCR_02
        analogToSensorVal(TIRETEMP_RL_I);
        analogToSensorVal(TIRETEMP_RL_M);
        analogToSensorVal(TIRETEMP_RL_O);
        msg.buf[0] = TIRETEMP_RL_I.actualAvg;
        msg.buf[1] = TIRETEMP_RL_I.actualAvg >> 8;
        msg.buf[2] = TIRETEMP_RL_M.actualAvg;
        msg.buf[3] = TIRETEMP_RL_M.actualAvg >> 8;
        msg.buf[4] = TIRETEMP_RL_O.actualAvg;
        msg.buf[5] = TIRETEMP_RL_O.actualAvg >> 8;
        msg.buf[6] = 0;
        msg.buf[7] = 0;
        sendCAN(0x94, 8, 1);

        //ATCCR_03
        analogToSensorVal(TIRETEMP_RR_I);
        analogToSensorVal(TIRETEMP_RR_M);
        analogToSensorVal(TIRETEMP_RR_O);
        msg.buf[0] = TIRETEMP_RR_I.actualAvg;
        msg.buf[1] = TIRETEMP_RR_I.actualAvg >> 8;
        msg.buf[2] = TIRETEMP_RR_M.actualAvg;
        msg.buf[3] = TIRETEMP_RR_M.actualAvg >> 8;
        msg.buf[4] = TIRETEMP_RR_O.actualAvg;
        msg.buf[5] = TIRETEMP_RR_O.actualAvg >> 8;
        msg.buf[6] = 0;
        msg.buf[7] = 0;
        sendCAN(0x95, 8, 1);

      } // end 100Hz timer messages
      break;
  }
}










void sendCAN(int id, int len, int busNo)
{
  msg.len = len;  //CAN message length unit: Byte (8 bits)
  msg.id = id; //CAN ID

  switch(busNo)
  {
    case 1:
      Can0.write(msg); // let er rip
      break;
  }

}
