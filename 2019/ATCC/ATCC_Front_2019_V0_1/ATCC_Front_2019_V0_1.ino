#include <FlexCAN.h>

FlexCAN CANbus(1000000);
static CAN_message_t msg;

unsigned long sendTimer1000Hz = 0;
unsigned long sendTimer500Hz = 0;
unsigned long sendTimer200Hz = 0;
unsigned long sendTimer100Hz = 0;
unsigned long sendTimer50Hz = 0;
unsigned long sendTimer20Hz = 0;
unsigned long sendTimer10Hz = 0;

typedef struct
{
  int rawValue;
  int inputVoltage;
  int zeroVoltage;
  int unitVoltageRatio;
  int scalingFactor;
  int CAN;
} sensor;

sensor sensors[11];
 
sensor damperPositionFR;
sensor damperPositionFL;
sensor brakePressureFR;
sensor brakePressureFL;
sensor brakePressureRR;
sensor brakePressureRL;
sensor brakeRotorTempFR;
sensor brakeRotorTempFL;
sensor trackTemperature;
sensor waterTemperature_betweenRadiators;
sensor swa;

// Number of binary bits used to change the read resolution of analog inputs. 
const int numResolutionBits = 13;

/* 
 *In setup() we call analogReadResolution(13). This means analog inputs on pins
 *are represented by 13-bit binary numbers. The max number 13-bits can represent
 *is 2^13 - 1 = 8191.
 */
int maxRawValue = pow(2, numResolutionBits) - 1;

/*
 *Max voltage input on teensy pins. Reading a pin receiving 3.3V and up 
 *will give a raw value of 2^numBits - 1.
 */
const int maxInputVoltage = 33000;

// Resistors on the circuits voltage divider are 2.2k and 3.4k ohms.
const int resistor1 = 2200.0000;
const int resistor2 = 3400.0000;

// The size of a CAN message is one Byte (8-bits)
const int sizeCAN = 8;


// the setup routine runs once when you press reset:
void setup()
{
  CANbus.begin();

  analogReadResolution(numResolutionBits);

  sensors[0] = damperPositionFR;
  sensors[1] = damperPositionFL;
  sensors[2] = brakePressureFR;
  sensors[3] = brakePressureFL;
  sensors[4] = brakePressureRR;
  sensors[5] = brakePressureRL;
  sensors[6] = brakeRotorTempFR;
  sensors[7] = brakeRotorTempFL;
  sensors[8] = trackTemperature;
  sensors[9] = waterTemperature_betweenRadiators;
  sensors[10] = swa;
  
  //damperPositionFR  Type: 
  sensors[0].rawValue = 0;
  sensors[0].inputVoltage = 0;
  sensors[0].zeroVoltage = 0;
  sensors[0].unitVoltageRatio = 0;
  sensors[0].scalingFactor = 0;
  sensors[0].CAN = 0;

  //damperPositionFL  Type: 
  sensors[1].rawValue = 0;
  sensors[1].inputVoltage = 0;
  sensors[1].zeroVoltage = 0;
  sensors[1].unitVoltageRatio = 0;
  sensors[1].scalingFactor = 0;
  sensors[1].CAN = 0;

  //brakePressureFR  Type: MLH100PGB06A
  sensors[2].rawValue = 0;
  sensors[2].inputVoltage = 0;
  sensors[2].zeroVoltage = 4748.4;
  sensors[2].unitVoltageRatio = 0.19875;
  sensors[2].scalingFactor = 1;
  sensors[2].CAN = 0;

  //brakePressureFL  Type: MLH100PGB06A 
  sensors[3].rawValue = 0;
  sensors[3].inputVoltage = 0;
  sensors[3].zeroVoltage = 4748.4;
  sensors[3].unitVoltageRatio = .19875;
  sensors[3].scalingFactor = 1;
  sensors[3].CAN = 0;

  //brakePressureRR  Type: MLH100PGB06A
  sensors[4].rawValue = 0;
  sensors[4].inputVoltage = 0;
  sensors[4].zeroVoltage = 4748.4;
  sensors[4].unitVoltageRatio = .19875;
  sensors[4].scalingFactor = 1;
  sensors[4].CAN = 0;

  //brakePressureRL  Type: MLH100PGB06A
  sensors[5].rawValue = 0;
  sensors[5].inputVoltage = 0;
  sensors[5].zeroVoltage = 4748.4;
  sensors[5].unitVoltageRatio = .19875;
  sensors[5].scalingFactor = 1;
  sensors[5].CAN = 0;

  //brakeRotorTempFR  Type: Infkl 800
  sensors[6].rawValue = 0;
  sensors[6].inputVoltage = 0;
  sensors[6].zeroVoltage = 5000;
  sensors[6].unitVoltageRatio = 1/50;
  sensors[6].scalingFactor = 100;
  sensors[6].CAN = 0;
    
  //brakeRotorTempFL  Type: Infkl 800
  sensors[7].rawValue = 0;
  sensors[7].inputVoltage = 0;
  sensors[7].zeroVoltage = 5000;
  sensors[7].unitVoltageRatio = 1/50;
  sensors[7].scalingFactor = 100;
  sensors[7].CAN = 0;

  //trackTemperature  Type: Infkl 150
  sensors[8].rawValue = 0;
  sensors[8].inputVoltage = 0;
  sensors[8].zeroVoltage = 4000;
  sensors[8].unitVoltageRatio = 1/300;
  sensors[8].scalingFactor = 100;
  sensors[8].CAN = 0;

  //waterTemperature_betweenRadiators  Type: not done
  sensors[9].rawValue = 0;
  sensors[9].inputVoltage = 0;
  sensors[9].zeroVoltage = 0;
  sensors[9].unitVoltageRatio = 0;
  sensors[9].scalingFactor = 0;
  sensors[9].CAN = 0;

  //Steering Wheel Angle (SWA)  Type: not done as well
  sensors[10].rawValue = 0;
  sensors[10].inputVoltage = 0;
  sensors[10].zeroVoltage = 0;
  sensors[10].unitVoltageRatio = 0;
  sensors[10].scalingFactor = 0;
  sensors[10].CAN = 0;
}

// the loop routine runs over and over again forever:
void loop()
{
  ReadRawSensorValue();
  CalculateInputVoltage();
  CalibrateInputVoltage();
  CalculateCAN();
}

void ReadRawSensorValue()
{
  sensors[0].rawValue = analogRead(A0);   //damperPositionFR
  sensors[1].rawValue = analogRead(A1);   //damperPositionFL
  sensors[2].rawValue = analogRead(A13);  //brakePressureFR
  sensors[3].rawValue = analogRead(A16);  //brakePressureFL
  sensors[4].rawValue = analogRead(A4);   //brakePressureRR
  sensors[5].rawValue = analogRead(A5);   //brakePressureRL
  sensors[6].rawValue = analogRead(A12);  //brakeRotorTempFR
  sensors[7].rawValue = analogRead(A15);  //brakeRotorTempFL
  sensors[8].rawValue = analogRead(A2);   //trackTemperature
  sensors[9].rawValue = analogRead(A19);   //waterTemperature_betweenRadiators
  sensors[10].rawValue = analogRead(A3);  //swa
}

void CalculateInputVoltage()
{
  for (unsigned int i = 0; i <= sizeof(sensors); i++)
    sensors[i].inputVoltage = sensors[i].rawValue * maxInputVoltage / maxRawValue;
}

void CalibrateInputVoltage()
{
  for (unsigned int i = 0; i <= sizeof(sensors); i++)
    sensors[i].inputVoltage = sensors[i].inputVoltage / resistor1 * resistor2;
}

void CalculateCAN()
{
  for (unsigned int i = 0; i <= sizeof(sensors); i++)
    sensors[i].CAN = ((sensors[i].inputVoltage - sensors[i].zeroVoltage) 
                      * sensors[i].unitVoltageRatio * sensors[i].scalingFactor);
}

void SendCAN(int id)
{
  msg.len = sizeCAN; //CAN messages are 8-bits long
  msg.id = id;
  CANbus.write(msg);
}

void CANMUX()
{
  msg.buf[0] = sensors[0].CAN;
  msg.buf[1] = sensors[0].CAN >> 8;
  msg.buf[2] = sensors[0].CAN;
  msg.buf[3] = sensors[0].CAN >> 8;
  msg.buf[4] = sensors[0].CAN;
  msg.buf[5] = sensors[0].CAN >> 8;
  msg.buf[6] = sensors[0].CAN;
  msg.buf[7] = sensors[0].CAN >> 8;

  SendCAN(0xFF);

  if(millis() - sendTimer10Hz >= 100)
  {
    msg.buf[0] = sensors[0].CAN;
    msg.buf[1] = sensors[0].CAN >> 8;
    msg.buf[2] = sensors[0].CAN;
    msg.buf[3] = sensors[0].CAN >> 8;
    msg.buf[4] = sensors[0].CAN;
    msg.buf[5] = sensors[0].CAN >> 8;
    msg.buf[6] = sensors[0].CAN;
    msg.buf[7] = sensors[0].CAN >> 8;
    
    SendCAN(0xFF);
  }
}
