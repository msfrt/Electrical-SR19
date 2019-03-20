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
//  Written by:     Nicholas Kopec & Dave Yonkers
//  Created :       01/31/2019
//  Modified By:    Nicholas Kopec & Dave Yonkers
//  Last Modified:  01/31/2019 8:65 PM
//  Version:        0.1
//  Purpose:        Make a car go fast
//  Description:    PDM code
//------------------------------------------------------------------------------



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

//// set the CAN busses to 1Mbaud Rate
//FlexCAN CANbus0(1000000, 0);
//FlexCAN CANbus1(1000000, 1);

// define message type
//static CAN_message_t txmsg;
//static CAN_message_t rxmsg;


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

// initialize the Sensor reading timer variables
unsigned long SensTimer2000Hz   = 0;
unsigned long SensTimer1000Hz   = 0;
unsigned long SensTimer500Hz    = 0;
unsigned long SensTimer200Hz    = 0;
unsigned long SensTimer100Hz    = 0;
unsigned long SensTimer50Hz     = 0;
unsigned long SensTimer20Hz     = 0;
unsigned long SensTimer10Hz     = 0;

//------------------------------------------------------------------------------
//
//             Structures for PDM Output Channels
//
//------------------------------------------------------------------------------

// structure for PDM channels with only voltage logging
typedef struct
{

  // used to tell the status of the module
  uint8_t deviceStatus;

  // voltage sensing
  int voltSensVal = 0;
  int voltMax = -32768;
  int voltMin = 32768;
  int voltAvg = 0;
  int voltSensCount = 0;

} senseVolt;

senseVolt MAIN, DATA;

// structure for PDM channels with only voltage and current logging
typedef struct
{

  // used to tell the status of the module
  uint8_t deviceStatus;

  // voltage sensing
  int voltSensVal = 0;
  int voltMax = -32768;
  int voltMin = 32768;
  int voltAvg = 0;
  int voltSensCount = 0;

  // current sensing
  int currentSensVal = 0;
  int currentMax = -32768;
  int currentMin = 32768;
  int currentAvg = 0;
  int currentSensCount = 0;

} senseVoltCurrent;

senseVoltCurrent PDM, FUEL;

// structure for PDM channels with voltage, current, and PWM logging
typedef struct
{

  // used to tell the status of the module
  uint8_t deviceStatus;

  // voltage sensing
  int voltSensVal = 0;
  int voltMax = -32768;
  int voltMin = 32768;
  int voltAvg = 0;
  int voltSensCount = 0;

  // current sensing
  int currentSensVal = 0;
  int currentMax = -32768;
  int currentMin = 32768;
  int currentAvg = 0;
  int currentSensCount = 0;

  // pulse width modulation
  int currentPwmRate = 0;
  int targetPwmRate = 0;
  int currentPwmFreq = 0;
  int targetPwmFreq = 0;

} senseVoltCurrentPWM;

senseVoltCurrentPWM FANR, FANL, WP;

//------------------------------------------------------------------------------
//
//              Analog Inputs Array Initialization
//
//------------------------------------------------------------------------------

// initialize an array to store the voltage of the inputs at the
// teensy pin ( 0 - 3.3v )
int ANA_teensyVolt[23];

// initialize an array to store the voltage of the inputs adjusted
// for the sensors output ( 0 - 5v )
int ANA_sensorVolt[23];

//------------------------------------------------------------------------------
//
//              Other Analog Inputs Array Initialization
//
//------------------------------------------------------------------------------

// temp sensing
int BOARD_temp;

//------------------------------------------------------------------------------
//
//              Component State Initialization
//
//------------------------------------------------------------------------------

// initialize state variables for BRAKE LIGHT ------------------
int BLIGHT_state;


//------------------------------------------------------------------------------
//
//              CAN Input Variable Initialization
//
//------------------------------------------------------------------------------

// structure to store the attributes of messages recieved on the CAN Bus
typedef struct
{
  bool validity;
  unsigned long lastRecieve;
  int value;

}canSensor;

// CAN0 sensors
canSensor CAN0_engTemp, CAN0_rpm;

// CAN1 sensors
canSensor CAN1_brakePressureFL, CAN1_brakePressureFR, CAN1_brakePressureRL, CAN1_brakePressureRR;
canSensor CAN1_betweenRadTemp, CAN1_rightRadInTemp, CAN1_leftRadOutTemp;


//------------------------------------------------------------------------------
//
//
//
//              END INITIALIZATION
//
//
//
//------------------------------------------------------------------------------



void setup() {

  // start the CAN busses
  Can0.begin(1000000);
  Can1.begin(1000000);

  Serial.begin(9600);

  // set the resolution of the analog input from 0-1023 (10bit) to 0-1891 (13bit)
  analogReadResolution(13);

  // set the pins as inputs or outputs
  pinMode(A0, INPUT); // Fuel Current Sensor
  pinMode(A1, INPUT); // FanR Current Sensor
  pinMode(A2, INPUT); // FanL Current Sensor
  pinMode(A3, OUTPUT); // Brake Light Signal
  pinMode(A4, INPUT); // WP Current Sensor
  pinMode(A5, INPUT); // PDM Voltage
  pinMode(A6, OUTPUT); // FanL Signal
  pinMode(A7, OUTPUT); // FanR Signal
  pinMode(A8, OUTPUT); // Water Pump Signal
  pinMode(A9, INPUT); // PDM Current Sense
  pinMode(13, OUTPUT); // Onboard LED
  pinMode(A16, INPUT); // Board Temperature
  pinMode(A17, INPUT); // Data Voltage
  pinMode(A18, INPUT); // Main Voltage
  pinMode(A19, INPUT); // Fuel Voltage
  pinMode(A20, INPUT); // FanL Voltage
  pinMode(A21, INPUT); // FanR Voltage
  pinMode(A22, INPUT); // WP Voltage

  // flash the LED
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
}

//------------------------------------------------------------------------------
//
//
//
//              MAIN LOOP
//
//
//
//------------------------------------------------------------------------------

void loop() {
  // put your main code here, to run repeatedly:
  //STORE_SENSE_VAL();
  Serial.println("testing");
}

//------------------------------------------------------------------------------
//
//
//
//              FUNCTIONS
//
//
//
//------------------------------------------------------------------------------

static void ANA_READ_TO_TEENSYVOLT()
{

  //----------------------------------------------------------------------------
  // Reads the analog value of the pins ( 0 - 8191) and then converts it
  // to the voltage of pin ( 0 - 3.3V )
  //----------------------------------------------------------------------------

  // Read Analog Pins ------------------------------------------
  // takes the raw value and multiplies it by 33000 / 8191 to get the voltage at the pin

  ANA_teensyVolt[0] = analogRead(A0) * 33000 / 8191; //Fuel Current Sense
  ANA_teensyVolt[1] = analogRead(A1) * 33000 / 8191; // FanR Current Sense
  ANA_teensyVolt[2] = analogRead(A2) * 33000 / 8191; // FanL Current Sense
  //ANA_teensyVolt[3] = analogRead(A3) * 33000 / 8191; // Teensy Brake Sig
  ANA_teensyVolt[4] = analogRead(A4) * 33000 / 8191; // WP Current Sense
  ANA_teensyVolt[5] = analogRead(A5) * 33000 / 8191; // PDM Voltage
  //ANA_teensyVolt[6] = analogRead(A6) * 33000 / 8191; // FanL Signal
  //ANA_teensyVolt[7] = analogRead(A7) * 33000 / 8191; // FanR Signal
  //ANA_teensyVolt[8] = analogRead(A8) * 33000 / 8191; // Water Pump Signal
  ANA_teensyVolt[9] = analogRead(A9) * 33000 / 8191; // VBAT Current Sense
  //ANA_teensyVolt[10] = analogRead(A10) * 33000 / 8191;
  //ANA_teensyVolt[11] = analogRead(A11) * 33000 / 8191;
  //ANA_teensyVolt[12] = analogRead(A12) * 33000 / 8191;
  //ANA_teensyVolt[13] = analogRead(A13) * 33000 / 8191;
  //ANA_teensyVolt[14] = analogRead(A14) * 33000 / 8191;
  //ANA_teensyVolt[15] = analogRead(A15) * 33000 / 8191;
  ANA_teensyVolt[16] = analogRead(A16) * 33000 / 8191; // Board Temperature
  ANA_teensyVolt[17] = analogRead(A17) * 33000 / 8191; // Data Voltage
  ANA_teensyVolt[18] = analogRead(A18) * 33000 / 8191; // Main Voltage
  ANA_teensyVolt[19] = analogRead(A19) * 33000 / 8191; // Fuel Voltage
  ANA_teensyVolt[20] = analogRead(A20) * 33000 / 8191; // FanL Voltage
  ANA_teensyVolt[21] = analogRead(A21) * 33000 / 8191; // FanR Voltage
  ANA_teensyVolt[22] = analogRead(A22) * 33000 / 8191; // WP Voltage

  // uncomment if you want to spit out what the teensy is doing
  //
  // for (int i = 0; i < 23; i++) {
  //   Serial.print("\nANA_teensyVolt[");
  //   Serial.print(i);
  //   Serial.print("]: ");
  //   Serial.print(ANA_teensyVolt[i]);
  // }


}

static void ANA_TEENSYVOLT_TO_SENSORVOLT()
{

  //----------------------------------------------------------------------------
  // Reads the voltage of the pins ( 0 - 3.3V ) and then converts it
  // to the voltage of sensor it is reading ( 0 - 5V ) or ( 0 - 12V )
  // by reversing the math of the voltage divider.
  //----------------------------------------------------------------------------


  // convert the read voltage with a range if 0 - 3.3 V to the original 0 - 5 V the sensor outputs

  // the formula ANA_sensorVolt[x] = ANA_teensyVolt[x] / 10000.0000 * (10000.0000 + 39000.0000);
  // represents the reversal of a voltage divider containing a 39k and 10k resistor

  ANA_sensorVolt[0] = ANA_teensyVolt[0]; //Fuel Current Sense - No adjustment needed
  ANA_sensorVolt[1] = ANA_teensyVolt[1]; // FanR Current Sense  - No adjustment needed
  ANA_sensorVolt[2] = ANA_teensyVolt[2]; // FanL Current Sense  - No adjustment needed
  //ANA_sensorVolt[3] = ANA_teensyVolt[3] / 2200.0000 * 3400.0000; // Teensy Brake Sig
  ANA_sensorVolt[4] = ANA_teensyVolt[4]; // WP Current Sense  - No adjustment needed
  ANA_sensorVolt[5] = ANA_teensyVolt[5] / 10000.0000 * (10000.0000 + 39000.0000); // PDM Voltage
  //ANA_sensorVolt[6] = ANA_teensyVolt[6] / 2200.0000 * 3400.0000; // FanL Signal
  //ANA_sensorVolt[7] = ANA_teensyVolt[7] / 2200.0000 * 3400.0000; // FanR Signal
  //ANA_sensorVolt[8] = ANA_teensyVolt[8] / 2200.0000 * 3400.0000; // Water Pump Signal
  ANA_sensorVolt[9] = ANA_teensyVolt[9]; // VBAT Current Sense  - No adjustment needed
  //ANA_sensorVolt[10] = ANA_teensyVolt[10] / 2200.0000 * 3400.0000;
  //ANA_sensorVolt[11] = ANA_teensyVolt[11] / 2200.0000 * 3400.0000;
  //ANA_sensorVolt[12] = ANA_teensyVolt[12] / 2200.0000 * 3400.0000;
  //ANA_sensorVolt[13] = ANA_teensyVolt[13] / 2200.0000 * 3400.0000;
  //ANA_sensorVolt[14] = ANA_teensyVolt[14] / 2200.0000 * 3400.0000;
  //ANA_sensorVolt[15] = ANA_teensyVolt[15] / 2200.0000 * 3400.0000;
  ANA_sensorVolt[16] = ANA_teensyVolt[16]; // Board Temperature - No adjustment needed
  ANA_sensorVolt[17] = ANA_teensyVolt[17] / 10000.0000 * (10000.0000 + 39000.0000); // Data Voltage
  ANA_sensorVolt[18] = ANA_teensyVolt[18] / 10000.0000 * (10000.0000 + 39000.0000); // Main Voltage
  ANA_sensorVolt[19] = ANA_teensyVolt[19] / 10000.0000 * (10000.0000 + 39000.0000); // Fuel Voltage
  ANA_sensorVolt[20] = ANA_teensyVolt[20] / 10000.0000 * (10000.0000 + 39000.0000); // FanL Voltage
  ANA_sensorVolt[21] = ANA_teensyVolt[21] / 10000.0000 * (10000.0000 + 39000.0000); // FanR Voltage
  ANA_sensorVolt[22] = ANA_teensyVolt[22] / 10000.0000 * (10000.0000 + 39000.0000); // WP Voltage


  // uncomment if you want to spit out what the teensy is doing
  //
  // for (int i = 0; i < 23; i++) {
  //   Serial.print("\nANA_sensorVolt[");
  //   Serial.print(i);
  //   Serial.print("]: ");
  //   Serial.print(ANA_sensorVolt[i]);
  // }


}

static void GET_SENS_VAL()
{

  //----------------------------------------------------------------------------
  // Calls functions to read the analog voltage inputs of the Teensy
  // Calls the function to convert the analog voltage to sensor voltage
  // Manipulates voltage into a readable value based on sensor datasheets
  //----------------------------------------------------------------------------


  // retrieve the analog inputs --------------------------------
  ANA_READ_TO_TEENSYVOLT();

  // convert the voltage at the pins to the --------------------
  // voltage before the onboard voltage divider
  ANA_TEENSYVOLT_TO_SENSORVOLT();

  //----------------------------------------------------------------------------
  //
  //
  //
  //                Calculate True Values
  //
  //
  //
  //----------------------------------------------------------------------------

  // formula:
  // = ( ANA_sensorVolt[ x] -  16500 ) / (1.0000/132.0000) * 100;
  //            |                |                |            |
  // recorded sensor (mV * 10)   |                |            |
  //                             |                |            |
  //            sensor zero voltage (mV * 10)     |            |
  //                                              |            |
  //                    inverse of (mV*10)/unit for sensor     |
  //                                                           |
  //                       scaling factor ( inverse of scaling factor in the DBC)

  // ***** sensor calibration *****                                     //  factor, min rate, CAN signal name
  FUEL.currentSensVal   = ( ANA_sensorVolt[ 0] -  16500 ) / (1.0000 / 264.0000) * 100; //
  FANR.currentSensVal   = ( ANA_sensorVolt[ 1] -  16500 ) / (1.0000 / 264.0000) * 100; //
  FANL.currentSensVal   = ( ANA_sensorVolt[ 2] -  16500 ) / (1.0000 / 264.0000) * 100; //
  //SensVal[3]   = ( ANA_sensorVolt[ 3] / 1000.000 * 49.000 * 1.01785 ; //  -  signal to Brakelight, no sensor
  //WP.currentSensVal   = ( ANA_sensorVolt[ 4] -  16500 ) / (1.0000 / 264.0000) * 100; //
  //PDM.voltSensVal   = ( ANA_sensorVolt[ 5] / 1000.000 * 49.000 * 1.01785 ; //
  //SensVal[6]   = ( ANA_sensorVolt[ 6] / 1000.000 * 49.000 * 1.01785 ; //  -  signal to FanL, no sensor
  //SensVal[7]   = ( ANA_sensorVolt[ 7] / 1000.000 * 49.000 * 1.01785 ; //  -  signal to FanR, no sensor
  //SensVal[8]   = ( ANA_sensorVolt[ 8] / 1000.000 * 49.000 * 1.01785 ; //  -  signal to Water Pump, no sensor
  PDM.currentSensVal   = ( ANA_sensorVolt[ 9] -  16500 ) / (1.0000/132.0000) * 100; //
  //SensVal[10]  = ( ANA_sensorVolt[10] / 1000.000 * 49.000 * 1.01785 ; //  -  N/C
  //SensVal[11]  = ( ANA_sensorVolt[11] / 1000.000 * 49.000 * 1.01785 ; //  -  N/C
  //SensVal[12]  = ( ANA_sensorVolt[12] / 1000.000 * 49.000 * 1.01785 ; //  -  N/C
  //SensVal[13]  = ( ANA_sensorVolt[13] / 1000.000 * 49.000 * 1.01785 ; //  -  N/C
  //SensVal[14]  = ( ANA_sensorVolt[14] / 1000.000 * 49.000 * 1.01785 ; //  -  N/C
  //SensVal[15]  = ( ANA_sensorVolt[15] / 1000.000 * 49.000 * 1.01785 ; //  -  N/C
  //BOARD_temp  = ( ANA_sensorVolt[16] / 1000.000 * 49.000 * 1.01785 ; //
  //DATA.voltSensVal  = ( ANA_sensorVolt[17] / 1000.000 * 49.000 * 1.01785 ; //
  //MAIN.voltSensVal  = ( ANA_sensorVolt[18] / 1000.000 * 49.000 * 1.01785 ; //
  //FUEL.voltSensVal  = ( ANA_sensorVolt[19] / 1000.000 * 49.000 * 1.01785 ; //
  //FANL.voltSensVal  = ( ANA_sensorVolt[20] / 1000.000 * 49.000 * 1.01785 ; //
  //FANR.voltSensVal  = ( ANA_sensorVolt[21] / 1000.000 * 49.000 * 1.01785 ; //
  //WP.voltSensVal  = ( ANA_sensorVolt[22] / 1000.000 * 49.000 * 1.01785 ; //

}

static void STORE_SENSE_VAL()
{
  //----------------------------------------------------------------------------
  // Reads the calculated values from ANA_TEENSYVOLT_TO_SENSORVOLT
  //    determines if they are mins, maxs
  //    adds all of the sensor values to .voltAvg for a running total
  //    adds one to the counter, so the average can be calculated later
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  //
  //
  //
  //                Voltage and Current Variables
  //
  //
  //
  //----------------------------------------------------------------------------

  if( micros() - SensTimer2000Hz >= 500 )
  {
    SensTimer2000Hz = micros();

    // read the values -----------------------------------------
    GET_SENS_VAL();

    //--------------------------------------------------------------------------
    //
    //              Voltage Min, Max, Avg, and Calc Storage
    //
    //--------------------------------------------------------------------------

    // PDM Voltage ---------------------------------------------
    if        ( PDM.voltSensVal < PDM.voltMin ){ PDM.voltMin = PDM.voltSensVal; }
    else if   ( PDM.voltSensVal > PDM.voltMax ){ PDM.voltMax = PDM.voltSensVal; }
    PDM.voltAvg += PDM.voltSensVal;
    PDM.voltSensCount++;

    // Main Voltage --------------------------------------------
    if        ( MAIN.voltSensVal < MAIN.voltMin ){ MAIN.voltMin = MAIN.voltSensVal; }
    else if   ( MAIN.voltSensVal > MAIN.voltMax ){ MAIN.voltMax = MAIN.voltSensVal; }
    MAIN.voltAvg += MAIN.voltSensVal;
    MAIN.voltSensCount++;

    // DATA Voltage --------------------------------------------
    if        ( DATA.voltSensVal < DATA.voltMin ){ DATA.voltMin = DATA.voltSensVal; }
    else if   ( DATA.voltSensVal > DATA.voltMax ){ DATA.voltMax = DATA.voltSensVal; }
    DATA.voltAvg += DATA.voltSensVal;
    DATA.voltSensCount++;

    // FUEL Voltage --------------------------------------------
    if        ( FUEL.voltSensVal < FUEL.voltMin ){ FUEL.voltMin = FUEL.voltSensVal; }
    else if   ( FUEL.voltSensVal > FUEL.voltMax ){ FUEL.voltMax = FUEL.voltSensVal; }
    FUEL.voltAvg += FUEL.voltSensVal;
    FUEL.voltSensCount++;

    // WP Voltage ----------------------------------------------
    if        ( WP.voltSensVal < WP.voltMin ){ WP.voltMin = WP.voltSensVal; }
    else if   ( WP.voltSensVal > WP.voltMax ){ WP.voltMax = WP.voltSensVal; }
    WP.voltAvg += WP.voltSensVal;
    WP.voltSensCount++;

    // FANR Voltage --------------------------------------------
    if        ( FANR.voltSensVal < FANR.voltMin ){ FANR.voltMin = FANR.voltSensVal; }
    else if   ( FANR.voltSensVal > FANR.voltMax ){ FANR.voltMax = FANR.voltSensVal; }
    FANR.voltAvg += FANR.voltSensVal;
    FANR.voltSensCount++;

    // FANL Voltage --------------------------------------------
    if        ( FANL.voltSensVal < FANL.voltMin ){ FANL.voltMin = FANL.voltSensVal; }
    else if   ( FANL.voltSensVal > FANL.voltMax ){ FANL.voltMax = FANL.voltSensVal; }
    FANL.voltAvg += FANL.voltSensVal;
    FANL.voltSensCount++;

    //--------------------------------------------------------------------------
    //
    //              Current Min, Max, Avg, and Calc Storage
    //
    //--------------------------------------------------------------------------

    // PDM Current ---------------------------------------------
    if        ( PDM.currentSensVal < PDM.currentMin ){ PDM.currentMin = PDM.currentSensVal; }
    else if   ( PDM.currentSensVal > PDM.currentMax ){ PDM.currentMax = PDM.currentSensVal; }
    PDM.currentAvg += PDM.currentSensVal;
    PDM.currentSensCount++;

    // FUEL Current --------------------------------------------
    if        ( FUEL.currentSensVal < FUEL.currentMin ){ FUEL.currentMin = FUEL.currentSensVal; }
    else if   ( FUEL.currentSensVal > FUEL.currentMax ){ FUEL.currentMax = FUEL.currentSensVal; }
    FUEL.currentAvg += FUEL.currentSensVal;
    FUEL.currentSensCount++;

    // WP Current ----------------------------------------------
    if        ( WP.currentSensVal < WP.currentMin ){ WP.currentMin = WP.currentSensVal; }
    else if   ( WP.currentSensVal > WP.currentMax ){ WP.currentMax = WP.currentSensVal; }
    WP.currentAvg += WP.currentSensVal;
    WP.currentSensCount++;

    // FANR Current --------------------------------------------
    if        ( FANR.currentSensVal < FANR.currentMin ){ FANR.currentMin = FANR.currentSensVal; }
    else if   ( FANR.currentSensVal > FANR.currentMax ){ FANR.currentMax = FANR.currentSensVal; }
    FANR.currentAvg += FANR.currentSensVal;
    FANR.currentSensCount++;

    // FANL Current --------------------------------------------
    if        ( FANL.currentSensVal < FANL.currentMin ){ FANL.currentMin = FANL.currentSensVal; }
    else if   ( FANL.currentSensVal > FANL.currentMax ){ FANL.currentMax = FANL.currentSensVal; }
    FANL.currentAvg += FANL.currentSensVal;
    FANL.currentSensCount++;

  }
}

static void CALC_SEND_CAN()
{

  //----------------------------------------------------------------------------
  // Timers for every CAN message
  //    calculate values and put them into CAN arrays
  //    call Jacky's send CAN function
  //    reset sensor min, max, avg, and count for future data calculations
  //----------------------------------------------------------------------------

  // CAN message timers

    // calculate

    // sauce them into the CAN arrays

    // no half sends! (send it)

    // reset sensor min, max, avg, and count for next

}



// do this:

// store analog inputs & calc their min, max, running total, counter, etc...

// calc avg when we want to send out a message
// convert to a readable format before sending the message














//------------------------------------------------------------------------------
//
//       ██████╗  ██████╗     ██╗    ██╗██╗  ██╗██╗████████╗███████╗
//      ██╔════╝ ██╔═══██╗    ██║    ██║██║  ██║██║╚══██╔══╝██╔════╝
//      ██║  ███╗██║   ██║    ██║ █╗ ██║███████║██║   ██║   █████╗
//      ██║   ██║██║   ██║    ██║███╗██║██╔══██║██║   ██║   ██╔══╝
//      ╚██████╔╝╚██████╔╝    ╚███╔███╔╝██║  ██║██║   ██║   ███████╗
//       ╚═════╝  ╚═════╝      ╚══╝╚══╝ ╚═╝  ╚═╝╚═╝   ╚═╝   ╚══════╝
//
//------------------------------------------------------------------------------
*/
