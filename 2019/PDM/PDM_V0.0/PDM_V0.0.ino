/*
 * Written by:    Nicholas Kopec
 * Created :      1/29/2019
 * Modified By:
 * Last Modified:
 * Version:       0.0
 * Purpose: Code for the PDM
 * Description
 */

//include and initialize CAN
#include <FlexCAN.h>
#include <kinetis_flexcan.h>

// Set the CAN busses to 1Mbaud Rate
FlexCAN CANbus0(1000000);
FlexCAN CANbus1(1000000);

static CAN_message_t msg;
static CAN_message_t rxmsg;

//initialize the arrays to store the analog inputs
int ANAVolt[23], SensVal[23], ANAread[23];

//initialize the timer variables
unsigned long SendTimer1000Hz = 0;
unsigned long SendTimer500Hz  = 0;
unsigned long SendTimer200Hz  = 0;
unsigned long SendTimer100Hz  = 0;
unsigned long SendTimer50Hz   = 0;
unsigned long SendTimer20Hz   = 0;
unsigned long SendTimer10Hz   = 0;

// BEGIN - Initialization of fan speed and water pump globals
// -------------------------------------------------------------

// Number of temperature entries in the fan speed table
const int numTempEntries = 6;

// Number of RPM entries in the fan speed table
const int numRPMEntries = 6;

/*
 *  Fan speed look up table.
 *  table[0][0] index does not hold useful data.
 *  table[0][x] indexes hold temperature values.
 *  table[x][0] indexes hold rpm values.
 */
int fanLeftTable[numTempEntries][numRPMEntries] =
{
  {   0, 0,  20,  40,  60,   80},
  {   0, 0,   0,   0,   0,    0},
  {2000, 0,  10,  15,  20,   40},
  {4000, 0,  15,  20,  40,   60},
  {6000, 0,  20,  40,  60,   80},
  {8000, 0,  40,  60,  80,  100}
};

int fanRightTable[numTempEntries][numRPMEntries] =
{
  {   0, 0,  20,  40,  60,   80},
  {   0, 0,   0,   0,   0,    0},
  {2000, 0,  10,  15,  20,   40},
  {4000, 0,  15,  20,  40,   60},
  {6000, 0,  20,  40,  60,   80},
  {8000, 0,  40,  60,  80,  100}
};

int waterPumpTable[numTempEntries][numRPMEntries] =
{
  {   0, 0,  20,  40,  60,   80},
  {   0, 0,  0,  0,  0,  0},
  {2000, 0,  1,  2,  3,  4},
  {4000, 0,  2,  4,  6,  8},
  {6000, 0,  3,  6,  9, 12},
  {8000, 0,  4,  8, 12, 16}
};

// Temperature value read from sensor
int temperature = 55;

// RPM value read from sensor
int rpm = 4500;

// Fan speed set from look up table
int fanLeftSpeed = 0;
int fanRightSpeed = 0;
int waterPumpSpeed = 0;

// Variables used to hold values returned by findTemp
int temperatureGreater = 0;
int temperatureLesser = 0;

// Variables used to hold values returned by findRPM
int rpmGreater = 0;
int rpmLesser = 0;

// END - Initialization of fan speed and water pump globals
// -------------------------------------------------------------

void setup(void)
{
  // start the CAN busses
  CANbus0.begin();
  CANbus1.begin();

  //Set the resolution of the analog input from 0-1023 (10bit) to 0-1891 (13bit)
  analogReadResolution(13);

  //Set the pins as inputs or outputs
  pinMode(A0, INPUT); // Fuel Current Sensor
  pinMode(A1, INPUT); // FanR Current Sensor
  pinMode(A2, INPUT); // FanL Current Sensor
  pinMode(A3, OUTPUT); // Brake Light Signal
  pinMode(A4, INPUT); // WP Current Sensor
  pinMode(A5, INPUT); // PDM Voltage
  pinMode(A6, OUTPUT); // FanL Signal
  pinMode(A7, OUTPUT); // FanR Signal
  pinMode(A8, OUTPUT); // Water Pump Signal
  pinMode(A9, INPUT); // VBAT Current Sense
  pinMode(13, OUTPUT); // Onboard LED
  pinMode(A16, INPUT); // Board Temperature
  pinMode(A17, INPUT); // Data Voltage
  pinMode(A18, INPUT); // Main Voltage
  pinMode(A19, INPUT); // Fuel Voltage
  pinMode(A20, INPUT); // FanL Voltage
  pinMode(A21, INPUT); // FanR Voltage
  pinMode(A22, INPUT); // WP Voltage

  //Flash the LED
  digitalWrite(13, 1);
  delay(1000);
  digitalWrite(13, 0);

}
// -------------------------------------------------------------

void loop(void)
{

  // Read from Analog voltage pins
  READ_ANA();
  CAN_DECODE();
  SENS_CAL();

  // convert from 3.3v to 5v scall 1.2K and 2.2K resistor volt divider
  //ANAVoltCalibration();

  //unsigned long currentTime = millis();

  if ( millis() - SendTimer1000Hz >=  1 )
  {
    SendTimer1000Hz = millis();
    CANMUX(); // get sensor value to Can message posision and send
  }

  // BEGIN - Setting fan and water pump speeds
  // -------------------------------------------------------------

  // Left Fan
  findTemp(fanLeftTable);
  findRPM(fanLeftTable);
  fanLeftSpeed = setFanSpeed(fanLeftTable);

  // Right Fan
  findTemp(fanRightTable);
  findRPM(fanRightTable);
  fanRightSpeed = setFanSpeed(fanRightTable);

  // Water Pump
  findTemp(waterPumpTable);
  findRPM(waterPumpTable);
  waterPumpSpeed = setFanSpeed(waterPumpTable);

  // END  - Setting fan and water pump speeds
  // -------------------------------------------------------------

}




// ************Functions************

static void READ_ANA()
{

  //---- Read analog pins ----------
  // takes the raw value and multiplys it by 33000 / 8191 to get the voltage at the pin

  ANAread[0] = analogRead(A0) * 33000 / 8191; //Fuel Current Sense
  ANAread[1] = analogRead(A1) * 33000 / 8191; // FanR Current Sense
  ANAread[2] = analogRead(A2) * 33000 / 8191; // FanL Current Sense
  //ANAread[3] = analogRead(A3) * 33000 / 8191; // Teensy Brake Sig
  ANAread[4] = analogRead(A4) * 33000 / 8191; // WP Current Sense
  ANAread[5] = analogRead(A5) * 33000 / 8191; // PDM Voltage
  //ANAread[6] = analogRead(A6) * 33000 / 8191; // FanL Signal
  //ANAread[7] = analogRead(A7) * 33000 / 8191; // FanR Signal
  //ANAread[8] = analogRead(A8) * 33000 / 8191; // Water Pump Signal
  ANAread[9] = analogRead(A9) * 33000 / 8191; // VBAT Current Sense
  //ANAread[10] = analogRead(A10) * 33000 / 8191;
  //ANAread[11] = analogRead(A11) * 33000 / 8191;
  //ANAread[12] = analogRead(A12) * 33000 / 8191;
  //ANAread[13] = analogRead(A13) * 33000 / 8191;
  //ANAread[14] = analogRead(A14) * 33000 / 8191;
  //ANAread[15] = analogRead(A15) * 33000 / 8191;
  ANAread[16] = analogRead(A16) * 33000 / 8191; // Board Temperature
  ANAread[17] = analogRead(A17) * 33000 / 8191; // Data Voltage
  ANAread[18] = analogRead(A18) * 33000 / 8191; // Main Voltage
  ANAread[19] = analogRead(A19) * 33000 / 8191; // Fuel Voltage
  ANAread[20] = analogRead(A20) * 33000 / 8191; // FanL Voltage
  ANAread[21] = analogRead(A21) * 33000 / 8191; // FanR Voltage
  ANAread[22] = analogRead(A22) * 33000 / 8191; // WP Voltage


}


static void ANAVoltCalibration() {

  //---- Convert the read voltage with a range if 0 - 3.3 V to the original 0 - 5 V the sensor outputs

  ANAVolt[0] = analogRead(A0); //Fuel Current Sense - No adjustment needed
  ANAVolt[1] = analogRead(A1); // FanR Current Sense  - No adjustment needed
  ANAVolt[2] = analogRead(A2); // FanL Current Sense  - No adjustment needed
  //ANAVolt[3] = analogRead(A3) / 2200.0000 * 3400.0000; // Teensy Brake Sig
  ANAVolt[4] = analogRead(A4); // WP Current Sense  - No adjustment needed
  ANAVolt[5] = analogRead(A5) / 2200.0000 * 3400.0000; // PDM Voltage
  //ANAVolt[6] = analogRead(A6) / 2200.0000 * 3400.0000; // FanL Signal
  //ANAVolt[7] = analogRead(A7) / 2200.0000 * 3400.0000; // FanR Signal
  //ANAVolt[8] = analogRead(A8) / 2200.0000 * 3400.0000; // Water Pump Signal
  ANAVolt[9] = analogRead(A9); // VBAT Current Sense  - No adjustment needed
  //ANAVolt[10] = analogRead(A10) / 2200.0000 * 3400.0000;
  //ANAVolt[11] = analogRead(A11) / 2200.0000 * 3400.0000;
  //ANAVolt[12] = analogRead(A12) / 2200.0000 * 3400.0000;
  //ANAVolt[13] = analogRead(A13) / 2200.0000 * 3400.0000;
  //ANAVolt[14] = analogRead(A14) / 2200.0000 * 3400.0000;
  //ANAVolt[15] = analogRead(A15) / 2200.0000 * 3400.0000;
  ANAVolt[16] = analogRead(A16); // Board Temperature - No adjustment needed
  ANAVolt[17] = analogRead(A17) / 2200.0000 * 3400.0000; // Data Voltage
  ANAVolt[18] = analogRead(A18) / 2200.0000 * 3400.0000; // Main Voltage
  ANAVolt[19] = analogRead(A19) / 2200.0000 * 3400.0000; // Fuel Voltage
  ANAVolt[20] = analogRead(A20) / 2200.0000 * 3400.0000; // FanL Voltage
  ANAVolt[21] = analogRead(A21) / 2200.0000 * 3400.0000; // FanR Voltage
  ANAVolt[22] = analogRead(A22) / 2200.0000 * 3400.0000; // WP Voltage


  for (int i = 0; i < 23; i++) {
    Serial.print("\nANAVolt[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.print(ANAVolt[i]);
  }

}

static void SENS_CAL()
{

  // ***** sensor calibration *****                             //factor, min rate, name
  //SensVal[0]  = ( ANAVolt[ 0] -  4880 ) / 40000.0000 * 10000; //  0.01, 100Hz, Fan Current
  //SensVal[1]  = ( ANAVolt[ 1] -  4880 ) / 40000.0000 * 10000; //  0.01, 100Hz, WP Current
  //SensVal[2]  = ( ANAVolt[ 2] -  4950 ) / 40000.0000 * 10000; //  0.01, 100Hz, Input Current
  //SensVal[3]  =  ANAVolt[3] / 1000.000 * 49.000 * 1.01785   ; //  0.01, 100Hz, UB_PDM
  //SensVal[4]  = 0;

}

static void CANMUX()
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
    CAN_DATA_SEND(0x50, 8, 0); // 500Hz
   *
   */

  if ( millis() - SendTimer1000Hz >= 1 )
  {
    SendTimer1000Hz = millis();
  }

  if ( millis() - SendTimer500Hz >=  2 )
  {
    SendTimer500Hz = millis();

  }

  if ( millis() - SendTimer200Hz >= 5 )
  {
    SendTimer200Hz = millis();
  }

  if ( millis() - SendTimer100Hz >= 10 )
  {
    SendTimer100Hz = millis();
  }

  if ( millis() - SendTimer50Hz >= 20 )
  {
    SendTimer50Hz = millis();
  }

  if ( millis() - SendTimer20Hz >= 50 )
  {
    SendTimer20Hz = millis();
  }

  if ( millis() - SendTimer10Hz >=  100 )
  {
    SendTimer10Hz = millis();


    /*/displaying the sent message buffer
      Serial.print("\n*****SendStart*****");
      for ( int i = 8; i >= 0; i-- ) {
      Serial.print("\nmsg.buf[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.print(msg.buf[i]);
      }
      Serial.print("\n*****SendEND*****\n");
      //*/
  }
}

// send the CAN data over the selected bus, if the bus number isnt specified, it wont print a message
static void CAN_DATA_SEND(int id, int len, int busNo)
{
  msg.len = len;  //CAN message length unit: Byte (8 bits)
  msg.id = id; //CAN ID

  if (busNo == 0)
  {
    CANbus0.write(msg);  // this is send the static
  }
  if (busNo == 1)
  {
    CANbus1.write(msg);  // this is send the static
  }

}

static void CAN_DECODE()
{

  if ( CANbus0.read(rxmsg) )
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

  if ( CANbus1.read(rxmsg) )
  {
    //CAN bus 1 messages

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

/*
 *  Given a temperature value read from the sensor, findTemp will look through entries
 *  in the fan speed table to determine if the value is present in the table. If the value
 *  is present, the value is returned twice. If the value is not present, the values that
 *  are greater than and less than are returned.
*/
void findTemp(int table[numTempEntries][numRPMEntries])
{
  for (int i = 1; i < numTempEntries; i++)
  {

    if (table[0][i] == temperature)
    {
      temperatureGreater = i;
      temperatureLesser = i;
      break;
    }

    if (table[0][i] > temperature)
    {
      temperatureGreater = i;
      temperatureLesser = i - 1;
      break;
    }
  }
}

/*
 *  Given an rpm value read from the sensor, findRPM will look through entries
 *  in the fan speed table to determine if the value is present in the table. If the value
 *  is present, the value is returned twice. If the value is not present, the values that
 *  are greater than and less than are returned.
*/
void findRPM(int table[numTempEntries][numRPMEntries])
{
  for (int i = 1; i < numRPMEntries; i++)
  {

    if (table[i][0] == rpm)
    {
      rpmGreater = i;
      rpmLesser = i;
      break;
    }

    if (table[i][0] > rpm)
    {
      rpmGreater = i;
      rpmLesser = i - 1;
      break;
    }
  }
}

int setFanSpeed(int table[numTempEntries][numRPMEntries])
{
  // Only average two fan speed values from the lookup table if at least a given temperature or rpm value exist in the table
  if (temperatureGreater == temperatureLesser || rpmGreater == rpmLesser)
  {
    int val1 = table[temperatureGreater][rpmGreater];
    int val2 = table[temperatureLesser][rpmLesser];

    int fanSpeed = (val1 + val2) / 2;

    return fanSpeed;
  }

  // Average four values from the lookup table if neither the given temperature or rpm values exist in the lookup table
  else
  {
    int val1 = table[temperatureGreater][rpmGreater];
    int val2 = table[temperatureGreater][rpmLesser];
    int val3 = table[temperatureLesser][rpmGreater];
    int val4 = table[temperatureLesser][rpmLesser];

    int fanSpeed = (val1 + val2 + val3 + val4) / 4;

    return fanSpeed;
  }
}
