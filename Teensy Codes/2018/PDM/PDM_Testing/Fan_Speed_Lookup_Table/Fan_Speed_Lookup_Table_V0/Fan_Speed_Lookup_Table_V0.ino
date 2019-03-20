/*
 * Written by:    Zoinul Choudhury
 * Created :      10/13/2018
 * Last Modified: 10/14/2018 05:57 AM
 * Version:       0.1
 * 
 * This program reads values engine temperature and rpm values and searches
 * in a lookup table for an appropriate fan speed value.
 */

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
int table[numTempEntries][numRPMEntries] =
{
  {0, 0,  2,  4,  6,  8},
  {0, 0,  0,  0,  0,  0},
  {2, 0,  4,  8, 12, 16},
  {4, 0,  8, 16, 24, 32},
  {6, 0, 12, 24, 36, 48},
  {8, 0, 16, 32, 48, 64}
};

// Temperature value read from sensor
int temperature = 4; 

// RPM value read from sensor
int rpm = 4;

// Fan speed set from look up table
int fanSpeed = 0;

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
  
  findTemp();
  findRPM();

  // Only average two fan speed values from the lookup table if at least a given temperature or rpm value exist in the table
  if (temperatureGreater == temperatureLesser || rpmGreater == rpmLesser)
    fanSpeed = (table[temperatureGreater][rpmGreater] + table[temperatureLesser][rpmLesser]) / 2;

  // Average four values from the lookup table if neither the given temperature or rpm values exist in the lookup table
  else
  {
    int val1 = table[temperatureGreater][rpmGreater];
    int val2 = table[temperatureGreater][rpmLesser];
    int val3 = table[temperatureLesser][rpmGreater];
    int val4 = table[temperatureLesser][rpmLesser];

    fanSpeed = (val1 + val2 + val3 + val4) / 4;
  }

  Serial.print("Fan Speed: ");
  Serial.println(fanSpeed);
}

/* 
 *  Given a temperature value read from the sensor, findTemp will look through entries
 *  in the fan speed table to determine if the value is present in the table. If the value 
 *  is present, the value is returned twice. If the value is not present, the values that 
 *  are greater than and less than are returned.
*/
void findTemp()
{
  for (int i = 1; i < numTempEntries; i++)
  {

    if (table[0][i] == temperature)
    {
      //Serial.print("i: ");
      //Serial.println(i);
    
      temperatureGreater = i;
      temperatureLesser = i;
      
      break;
    }
    
    if (table[0][i] > temperature)
    {

      //Serial.print("i: ");
      //Serial.println(i);
      
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
void findRPM()
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
