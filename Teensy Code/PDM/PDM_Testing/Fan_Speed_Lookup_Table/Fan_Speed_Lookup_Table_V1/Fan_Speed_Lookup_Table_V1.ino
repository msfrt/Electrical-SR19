/*
 * Written by:    Zoinul Choudhury
 * Created :      10/15/2018
 * Last Modified: 10/15/2018 7:12 PM
 * Version:       0.1
 * 
 * This program reads values engine temperature and rpm values and searches
 * in a lookup table for an appropriate fan speed value.
 */

// Used to store runtime value
int prevTime = 0;

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
//int fanLeftTable [][]=
{
  {   0, 0,  20,  40,  60,   80},
  {   0, 0,   0,   0,   0,    0},
  {2000, 0,  10,  15,  20,   40},
  {4000, 0,  15,  20,  40,   60},
  {6000, 0,  20,  40,  60,   80},
  {8000, 0,  40,  60,  80,  100}
};

int fanRightTable[numTempEntries][numRPMEntries] =
//int fanRightTable [][]=
{
  {   0, 0,  20,  40,  60,   80},
  {   0, 0,   0,   0,   0,    0},
  {2000, 0,  10,  15,  20,   40},
  {4000, 0,  15,  20,  40,   60},
  {6000, 0,  20,  40,  60,   80},
  {8000, 0,  40,  60,  80,  100}
};

int waterPumpTable[numTempEntries][numRPMEntries] =
//int waterPumpTable [][]=
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

void setup()
{
  Serial.begin(9600);
}

void loop()
{

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

  Serial.print("Fan Left Speed: ");
  Serial.println(fanLeftSpeed);
  
  Serial.print("Fan Right Speed: ");
  Serial.println(fanRightSpeed);
  
  Serial.print("Water Pump Speed: ");
  Serial.println(waterPumpSpeed);

  Serial.print("Loop Run Time: ");
  Serial.print(millis() - prevTime);
  Serial.println(" ms\n");
  
  prevTime = millis();
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
