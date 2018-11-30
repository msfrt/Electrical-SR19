#include <FlexCAN.h>

FlexCAN CANbus(1000000);
static CAN_message_t msg;

int analogPinVoltage[21];
int analogPinRaw[21];
int sensorValue[21];

unsigned long sendTimer1000Hz = 0;
unsigned long sendTimer500Hz = 0;
unsigned long sendTimer200Hz = 0;
unsigned long sendTimer100Hz = 0;
unsigned long sendTimer50Hz = 0;
unsigned long sendTimer20Hz = 0;
unsigned long sendTimer10Hz = 0;

/* 
 *In setup() we call analogReadResolution(13). This means analog inputs on pins
 *are represented by 13-bit binary numbers. The max number 13-bits can represent
 *is 2^13 - 1 = 8191.
 */
const int maxRawValue = 8191;

// Number of binary bits used to change the read resolution of analog inputs. 
const int numBits = 13;

/*
 *Max voltage input on teensy pins. Reading a pin receiving 3.3V and up 
 *will give a raw value of 2^numBits - 1.
 */
const int maxInputVoltage = 33000;

// Resistors on the circuits voltage divider are 2.2k and 3.4k ohms.
const int resistor1 = 2200.0000;
const int resistor2 = 3400.0000;

// -------------------------------------------------------------
void setup(void)
{
  CANbus.begin();
  analogReadResolution(numBits);
  
  pinMode(13, OUTPUT);
  digitalWrite(13, 1);
  delay(1000);
  digitalWrite(13, 0);
  
  Serial.print("Gua - Send\n\n");

  //AT/AV Pull up 1K resistor to 3.3 activation, should be 1 for deactive, 0 for active
//  pinMode(5, OUTPUT);
//  pinMode(6, OUTPUT);
//  pinMode(7, OUTPUT);
//  pinMode(8, OUTPUT);
//  digitalWrite(5, 1); // A13
//  digitalWrite(6, 1); // A18
//  digitalWrite(7, 1); // A19
//  digitalWrite(8, 1); // A20

}// -------------------------------------------------------------

void loop(void)
{

  readAnalogVoltage();             // Read from Analog voltage pins
  analogVoltageCalibration();   // convert from 3.3v to 5v scall 1.2K and 2.2K resistor volt divider
  sensorCalibration();

  //unsigned long currentTime = millis();

  if ( millis() - sendTimer500Hz >=  2 ) {
    sendTimer500Hz = millis();
    CANMUX(); // get sensor value to Can message posision and send
  }
}

// ************Functions************

void sensorCalibration()
{
// ***** sensor calibration *****                                   Factor | Min Rate | Name
  sensorValue[0]  = ( analogPinVoltage[ 0] - 25000 ) /  2500.0000 *   500;     //0.01   | 100Hz    | Rectifier Current 50B
//sensorValue[1]  = ( analogPinVoltage[ 1] -  5000 ) / 40000.0000 *  8000;     //0.1    | 10Hz     | Brake Rotor Temp Rear
  sensorValue[2]  = ( analogPinVoltage[ 2] -  5000 ) / 40000.0000 * 20000;     //0.1    | 20Hz     | Brake Pressure 2
  sensorValue[3]  = ( analogPinVoltage[ 3] -  5000 ) / 40000.0000 *  1500;     //0.1    | 10Hz     | Tire Temp RLO
//sensorValue[4]  = ( analogPinVoltage[ 4] -  5000 ) / 40000.0000 *  1500;     //0.1    | 10Hz     | Tire Temp RLM
//sensorValue[5]  = 20000;                                            //0.1    | 10Hz     | Tire Temp RLI
//sensorValue[5]  = ( analogPinVoltage[ 5] -  5000 ) / 40000.0000 *  1500;     //0.1    | 10Hz     | Tire Temp RLI
  sensorValue[3]  = ( analogPinVoltage[ 3] -  4000 ) * 0.06 + 320;             //0.1    | 10Hz     | Tire Temp RLO
  sensorValue[4]  = ( analogPinVoltage[ 4] -  4000 ) * 0.06 + 320;             //0.1    | 10Hz     | Tire Temp RLM
  sensorValue[5]  = ( analogPinVoltage[ 5] -  4000 ) * 0.06 + 320;             //0.1    | 10Hz     | Tire Temp RLI
  sensorValue[6]  = ( analogPinVoltage[ 6]         ) / 50000.0000 *  7500;     //0.1    | 100Hz    | Linear Pot RL
  sensorValue[7]  = ( analogPinVoltage[ 7]         ) / 50000.0000 *  7500;     //0.1    | 100Hz    | Linear Pot RR
  sensorValue[8]  = ( analogPinVoltage[ 8] -  5000 ) / 40000.0000 *  1500;     //0.1    | 10Hz     | Tire Temp RRO
  sensorValue[9]  = ( analogPinVoltage[ 9] -  5000 ) / 40000.0000 *  1500;     //0.1    | 10Hz     | Tire Temp RRM
  sensorValue[10] = ( analogPinVoltage[10] -  5000 ) / 40000.0000 *  1500;     //0.1    | 10Hz     | Tire Temp RRI
  sensorValue[11] = ( analogPinVoltage[11] - 25000 ) /  2500.0000 *  1000;     //0.01   | 100Hz    | ESP Motor Current 100B
  sensorValue[12] = BoschTempHS(analogPinVoltage[12] / 10, 5000, 470) ;        //0.1    | 10Hz     | Water Inlet Temp // **AT**
  sensorValue[13] = 0 ;                                               // **AT/AV**
  sensorValue[15] = BoschTempHS(analogPinVoltage[15] / 10, 5000, 470) ;        //0.1    | 10Hz     | Water Outlet Temp // **AT**
  sensorValue[16] = 0 ;                                               // **AT**
  sensorValue[17] = 0 ;                                               // **AT**
  sensorValue[18] = 0 ;                                               // ------------, 2nd BAT Current // **AT/AV**
  sensorValue[19] = 0 ;                                               // **AT/AV**
  sensorValue[20] = 0 ;                                               // ------------, 2nd BAT Volt    // **AT/AV**
}

static void sendCANData(int id, int len) 
{
  msg.len = len;  //CAN message length unit: Byte (8 bits)
  msg.id = id; //CAN ID
  CANbus.write(msg);  // this is send the static
}

/* 
 *This function converts the raw input value from analog pins and converts them to
 *a voltage value.
 */
static void readAnalogVoltage() 
{
  //---- Read analog pin ----------
  analogPinRaw[0] = analogRead(A0);
  analogPinRaw[1] = analogRead(A1);
  analogPinRaw[2] = analogRead(A2);
  analogPinRaw[3] = analogRead(A3);
  analogPinRaw[4] = analogRead(A4);
  analogPinRaw[5] = analogRead(A5);
  analogPinRaw[6] = analogRead(A6);
  analogPinRaw[7] = analogRead(A7);
  analogPinRaw[8] = analogRead(A8);
  analogPinRaw[9] = analogRead(A9);
  analogPinRaw[10] = analogRead(A10);
  analogPinRaw[11] = analogRead(A11);
  analogPinRaw[12] = analogRead(A12);
  analogPinRaw[13] = analogRead(A13);
  analogPinRaw[15] = analogRead(A15);
  analogPinRaw[16] = analogRead(A16);
  analogPinRaw[17] = analogRead(A17);
  analogPinRaw[18] = analogRead(A18);
  analogPinRaw[19] = analogRead(A19);
  analogPinRaw[20] = analogRead(A20);

//  for (int i = 0; i < 21; i++)
//  {
//    Serial.print("\nanalogPinRaw[");
//    Serial.print(i);
//    Serial.print("]: ");
//    Serial.print(analogPinRaw[i]);
//  }

  // Convert raw input values (ragning from 0 - 2^numBits - 1) to the voltage input on each pin.
  for (int i = 0; i < 21; i++)
  {
    analogPinVoltage[i] = analogPinRaw[i] * maxInputVoltage / maxRawValue ;
//    Serial.print("\nanalogPinVoltage[");
//    Serial.print(i);
//    Serial.print("]: ");
//    Serial.print(analogPinVoltage[i]);
  }

}

/* 
 *This function converts the voltage value prom each analog pin to the voltage value
 *once current passes throguh the voltage divider on the circuit.
 */
static void analogVoltageCalibration()
{
//  Serial.print("\n*******");
  for (int i = 0; i < 21; i++)
  {
    analogPinVoltage[i] = analogPinVoltage[i] / resistor1 * resistor2 ;

//  Serial.print("\nanalogPinVoltage[");
//  Serial.print(i);
//  Serial.print("]: ");
//  Serial.print(analogPinVoltage[i]);
  }
}


static void CANMUX()
{
  //if ( millis() - sendTimer500Hz >=  2 ) {
  //sendTimer500Hz = millis();
  msg.buf[0] = sensorValue[6];
  msg.buf[1] = sensorValue[6] >> 8; //Linear Pot RL
  msg.buf[2] = sensorValue[7];
  msg.buf[3] = sensorValue[7] >> 8; //Linear Pot RR
  msg.buf[4] = sensorValue[0];
  msg.buf[5] = sensorValue[0] >> 8; //Rectifier Current
  msg.buf[6] = sensorValue[11];
  msg.buf[7] = sensorValue[11] >> 8; //ESP Motor Current
  //sendCANData(0x30, 8); // 500Hz

  msg.buf[0] = 0;
  msg.buf[1] = 0 >> 8;
  msg.buf[2] = 0;
  msg.buf[3] = 0 >> 8;
  msg.buf[4] = 0;
  msg.buf[5] = 0 >> 8;
  msg.buf[6] = 0;
  msg.buf[7] = 0 >> 8;
  //sendCANData(0x31, 8); //500Hz
  //}

  if ( millis() - sendTimer50Hz >=  20 )
  {
    sendTimer50Hz = millis();
    msg.buf[0] = sensorValue[2];
    msg.buf[1] = sensorValue[2] >> 8; //Brake Pressure Rear
    msg.buf[2] = 0;
    msg.buf[3] = 0 >> 8;
    msg.buf[4] = 0;
    msg.buf[5] = 0 >> 8;
    msg.buf[6] = 0;
    msg.buf[7] = 0 >> 8;
    //sendCANData(0x32, 8); //50Hz
  }

  if ( millis() - sendTimer10Hz >=  100 )
  {
//    sendTimer10Hz = millis();
//    msg.buf[0] = sensorValue[1];
//    msg.buf[1] = sensorValue[1] >> 8; //Brake Rotor Temp Rear
//    msg.buf[2] = sensorValue[3];
//    msg.buf[3] = sensorValue[3] >> 8; //Tire Temp RLO
//    msg.buf[4] = sensorValue[4];
//    msg.buf[5] = sensorValue[4] >> 8; //Tire Temp RLM
//    msg.buf[6] = sensorValue[5];
//    msg.buf[7] = sensorValue[5] >> 8; //Tire Temp RLI
//    sendCANData(0x33, 8);  //10Hz

    sendTimer10Hz = millis();
    msg.buf[0] = sensorValue[1];
    msg.buf[1] = sensorValue[1] >> 8; //Brake Rotor Temp Rear
    msg.buf[2] = sensorValue[3];
    msg.buf[3] = sensorValue[3] >> 8; //Tire Temp RLO
    msg.buf[4] = 0;
    msg.buf[5] = 0;
    msg.buf[6] = 0;
    msg.buf[7] = 0;
    sendCANData(0x33, 8);  //10Hz

    msg.buf[0] = sensorValue[8];
    msg.buf[1] = sensorValue[8] >> 8; //Tire Temp RRO
    msg.buf[2] = sensorValue[9];
    msg.buf[3] = sensorValue[9] >> 8; //Tire Temp RRM
    msg.buf[4] = sensorValue[10];
    msg.buf[5] = sensorValue[10] >> 8; //Tire Temp RRI
    msg.buf[6] = 0;
    msg.buf[7] = 0 >> 8;
    //sendCANData(0x34, 8);  //10Hz

    msg.buf[0] = sensorValue[12];
    msg.buf[1] = sensorValue[12] >> 8; //Water Inlet Temp
    msg.buf[2] = sensorValue[15];
    msg.buf[3] = sensorValue[15] >> 8; //Water Outlet Temp
    msg.buf[4] = 0;
    msg.buf[5] = 0 >> 8;
    msg.buf[6] = 0;
    msg.buf[7] = 0 >> 8;
    //sendCANData(0x35, 8);  //10Hz

    //display the sensorValue in current sending cycle
//      Serial.print("\n*******");
//      
//      for (int i = 0; i < 21; i++)
//      {
//      Serial.print("\nsensorValue[");
//      Serial.print(i);
//      Serial.print("]: ");
//      Serial.print(sensorValue[i]);
//      }

//      Serial.print("\nanalogPinVoltage[");
//      Serial.print(i);
//      Serial.print("]: ");
//      Serial.print(analogPinVoltage[i]);
  }
}

int BoschTempHS( int Input_mV, int Vs_mV, int Rpullup )
{
  float cTemp[] = { -55, -35, -20, -10, 0, 10, 20, 25, 30, 40, 50, 60, 70, 80, 90, 100, 120, 140, 160, 180, 200, 220, 240, 260, 280, 300};
  float cR[]    = { 519910, 158090, 71668, 44087, 27936, 18187, 12136, 10000, 8284, 5774, 4103, 2967, 2182, 1629, 1234, 946.6, 578.1, 368.8, 244.4, 167.6, 118.5, 86.08, 64.08, 48.76, 37.86, 29.94 };
  float cVolt[sizeof(cTemp) / 4];

  if (sizeof(cTemp) != sizeof(cR))
    return -9999;
    
  else
  {
    for (int i = 0; i < ((int) sizeof(cTemp) / 4); i++  )
    {
      cVolt[i] = cR[i] / (cR[i] + (float) Rpullup) * (float) Vs_mV ;
      //Serial.println(cVolt[i]);
    }
  }

  if ((float) Input_mV > cVolt[0])
    return (int) ((cTemp[0] - 0.1) * 10);
    
  else if ((float) Input_mV <  cVolt[ sizeof(cVolt) / 4 - 1 ] )
    return (int) (( cTemp[ sizeof(cTemp) / 4 - 1 ] + 0.1 ) * 10);
    
  else
  {
    for (int i = 0; i < (int) (sizeof(cVolt) / 4 - 1); i++)
    {
      if ( ( (float) Input_mV <=  cVolt[i]) &&  ( (float) Input_mV >= cVolt[i + 1]) )
        return (int) ( ( ( (float) Input_mV - cVolt[i + 1]) / (cVolt[i] - cVolt[i + 1]) * ( cTemp[i] - cTemp[i + 1] ) + cTemp[i + 1]      ) * 10 );
    }
  }
  return -7777;
}

int BoschTempH( int Input_mV, int Vs_mV, int Rpullup )
{
  float cTemp[] = {   -40,   -30,   -20,  -10,    0,   10,   20,   30,   40,  50,  60,  70,  80,  90, 100, 110, 120, 130, 140, 150 };
  float cR[]    = { 45313, 26114, 15462, 9397, 5896, 3792, 2500, 1707, 1175, 834, 596, 436, 323, 243, 187, 144, 113,  89,  71,  57 };
  float cVolt[sizeof(cTemp) / 4];

  if (sizeof(cTemp) != sizeof(cR))
    return -9999;
    
  else 
  {
    for (int i = 0; i < ((int) sizeof(cTemp) / 4); i++  )
    {
      cVolt[i] = cR[i] / (cR[i] + (float) Rpullup) * (float) Vs_mV ;
      //Serial.println(cVolt[i]);
    }
  }

  if ((float) Input_mV > cVolt[0])
    return (int) ((cTemp[0] - 0.1) * 10);
    
  else if ((float) Input_mV <  cVolt[ sizeof(cVolt) / 4 - 1 ] )
    return (int) (( cTemp[ sizeof(cTemp) / 4 - 1 ] + 0.1 ) * 10);
    
  else 
  {
    for (int i = 0; i < (int) (sizeof(cVolt) / 4 - 1); i++)
    {
      if ( ( (float) Input_mV <=  cVolt[i]) &&  ( (float) Input_mV >= cVolt[i + 1]) )
        return (int) ( ( ( (float) Input_mV - cVolt[i + 1]) / (cVolt[i] - cVolt[i + 1]) * ( cTemp[i] - cTemp[i + 1] ) + cTemp[i + 1]      ) * 10 );
    }
  }
  return -7777;
}
