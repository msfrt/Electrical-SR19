//DO NOT COPY THE STRUCTURES AND INITIALIZATIONS OVER TO PDM FILE
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
typedef struct
{
  bool validity = 0;
  unsigned long lastRecieve = 0;
  int value = 0;

} canSensor;

canSensor CAN0_engTemp, CAN0_rpm;

int BatteryVoltAvg = 119000;


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//DO NOT COPY THE STRUCTURES AND INITIALIZATIONS OVER TO PDM FILE


// Number of temperature entries in the fan speed table
const int FAN_numTempEntries = 12;

// Number of battery voltage entries in the fan speed table
const int FAN_numVoltEntries = 14;

// Number of temperature entries in the water pump speed table
const int WP_numTempEntries = 12;

// Number of battery voltage entries in the fan speed table
const int WP_numRPMEntries = 8;

// Fan speed set from look up table
int fanLeftSpeed = 0;
int fanRightSpeed = 0;
int waterPumpSpeed = 0;

// Variables used to hold values returned by findTemp
int FAN_temperatureGreater = 0;
int FAN_temperatureLesser = 0;

// Variables used to hold values returned by findTemp
int WP_temperatureGreater = 0;
int WP_temperatureLesser = 0;

// Variables used to hold values returned by findVolt
int FAN_voltGreater = 0;
int FAN_voltLesser = 0;

// Variables used to hold values returned by findRPM
int WP_rpmLesser = 0;
int WP_rpmGreater = 0;

//    rows: temp in degrees celcius * 10
// columns: battery voltage in mV * 10
int fanLeftTable[FAN_numTempEntries][FAN_numVoltEntries] =
{
  {    0, 80000, 90000, 100000, 105000, 110000, 119000, 120000, 130000, 137000, 138000, 139000, 142000, 145000},
  {    0,     0,     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0},
  {  700,     0,     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0},
  {  850,     0,     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     25},
  {  851,    15,    15,     15,     15,     15,     15,     15,     15,     15,     30,     30,     30,     30},
  {  920,    15,    15,     15,     15,     15,     15,     15,     15,     15,     50,     50,     50,     50},
  {  921,    15,    15,     15,     15,     25,     25,     25,     25,     25,     65,     65,     65,     65},
  {  950,    75,    75,     75,     75,     75,     75,     75,     75,     75,     75,     75,     75,     75},
  {  951,   100,   100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100},
  { 1000,   100,   100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100},
  { 1001,   100,   100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100},
  { 1500,   100,   100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100},
};


//    rows: temp in degrees celcius * 10
// columns: battery voltage in mV * 10
int fanRightTable[FAN_numTempEntries][FAN_numVoltEntries] =
{
  {    0, 80000, 90000, 100000, 105000, 110000, 119000, 120000, 130000, 137000, 138000, 139000, 142000, 145000},
  {    0,     0,     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0},
  {  700,     0,     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0},
  {  850,     0,     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     25},
  {  851,    15,    15,     15,     15,     15,     15,     15,     15,     15,     30,     30,     30,     30},
  {  920,    15,    15,     15,     15,     15,     15,     15,     15,     15,     50,     50,     50,     50},
  {  921,    15,    15,     15,     15,     25,     25,     25,     25,     25,     65,     65,     65,     65},
  {  950,    75,    75,     75,     75,     75,     75,     75,     75,     75,     75,     75,     75,     75},
  {  951,   100,   100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100},
  { 1000,   100,   100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100},
  { 1001,   100,   100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100},
  { 1500,   100,   100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100,    100}
};


//    rows: temp in degrees celcius * 10
// columns: RPM
int waterPumpTable[WP_numTempEntries][WP_numRPMEntries] =
{
  {   00,   0,  10,  20, 500, 510,  5000, 15000},
  {  100,   0,   0,   0,   0,  20,    20,    20},
  {  200,   0,   0,   0,   0,  20,    20,    20},
  {  400,   0,   0,   0,   0,  35,    35,    35},
  {  500,   0,   0,   0,   0,  35,    35,    35},
  {  600,   0,   0,   0,   0,  35,    35,    35},
  {  699,   0,   0,   0,   0,  35,    35,    35},
  {  700,   0,   0,   0,   0,  40,    40,    40},
  {  845,   0,   0,   0,   0,  50,    50,    50},
  {  846,  25,  25,   0,   0,  60,    60,    60},
  { 1000, 100, 100,   0,   0, 100,   100,   100},
  { 1500, 100, 100,   0,   0, 100,   100,   100},
};



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // begin the serial window

  pinMode(13, OUTPUT); // Onboard LED

  //Flash the LED
  digitalWrite(13, 1);
  delay(1000);
  digitalWrite(13, 0);

  // values read from the sensor
  CAN0_engTemp.value = 851;
  CAN0_rpm.value = 0;

}

void loop() {
  // put your main code here, to run repeatedly:

  // BEGIN - Setting fan and water pump speeds
  // -------------------------------------------------------------

  // Left Fan
  FAN_findTemp(fanLeftTable);
  FAN_findVolt(fanLeftTable);
  fanLeftSpeed = setFanSpeed(fanLeftTable);

  // Right Fan
  FAN_findTemp(fanRightTable);
  FAN_findVolt(fanRightTable);
  fanRightSpeed = setFanSpeed(fanRightTable);


  // Water Pump
  WP_findTemp(waterPumpTable);
  WP_findRPM(waterPumpTable);
  waterPumpSpeed = setWaterPumpSpeed(waterPumpTable);


  // END  - Setting fan and water pump speeds
  // -------------------------------------------------------------

  CAN0_engTemp.value += 1;
  BatteryVoltAvg += 1;
  CAN0_rpm.value += 10;

  delay(100);
  digitalWrite(13, 1);
  delay(100);
  digitalWrite(13, 0);

  Serial.print("fanL speed: "); Serial.print(fanLeftSpeed); Serial.println("%");
  Serial.print("fanR speed: "); Serial.print(fanRightSpeed); Serial.println("%");
  Serial.print("pump speed: "); Serial.print(waterPumpSpeed); Serial.println("%");


  if (BatteryVoltAvg > 139000)
  {
    BatteryVoltAvg = 119000;
  }
  if (CAN0_engTemp.value > 1200)
  {
    CAN0_engTemp.value = 851;
  }
  if (CAN0_rpm.value > 10000)
  {
    CAN0_rpm.value = 0;
  }

}

/*
 *  Given a temperature value read from the sensor, findTemp will look through entries
 *  in the fan speed table to determine if the value is present in the table. If the value
 *  is present, the value is returned twice. If the value is not present, the values that
 *  are greater than and less than are returned.
*/
void FAN_findTemp(int table[FAN_numTempEntries][FAN_numVoltEntries])
{
  for (int i = 1; i < FAN_numTempEntries; i++)
  {

    if (table[i][0] == CAN0_engTemp.value)
    {
      FAN_temperatureGreater = i;
      FAN_temperatureLesser = i;
      break;
    }

    // create a check so the lesser will not exceed the lower bounds of the table
    if (table[1][0] > CAN0_engTemp.value)
    {
      FAN_temperatureGreater = 1;
      FAN_temperatureLesser =1 ;
      break;
    }

    if (table[i][0] > CAN0_engTemp.value)
    {
      FAN_temperatureGreater = i;
      FAN_temperatureLesser = i - 1;
      break;
    }
  }
}

/*
 *  Given a BatteryVoltAvg value read from the sensor, findVolt will look through entries
 *  in the fan speed table to determine if the value is present in the table. If the value
 *  is present, the value is returned twice. If the value is not present, the values that
 *  are greater than and less than are returned.
*/
void FAN_findVolt(int table[FAN_numTempEntries][FAN_numVoltEntries])
{
  for (int i = 1; i < FAN_numVoltEntries; i++)
  {

    if (table[0][i] == BatteryVoltAvg)
    {
      FAN_voltGreater = i;
      FAN_voltLesser = i;
      break;
    }

    // create a check so the lesser will not exceed the lower bounds of the table
    if (table[0][1] > BatteryVoltAvg)
    {
      FAN_voltGreater = 1;
      FAN_voltLesser =1;
      break;
    }


    if (table[0][i] > BatteryVoltAvg && i != 1)
    {
      FAN_voltGreater = i;
      FAN_voltLesser = i - 1;
      break;
    }
  }
}

/*
 *  Given a temperature value read from the sensor, findTemp will look through entries
 *  in the water pump table to determine if the value is present in the table. If the value
 *  is present, the value is returned twice. If the value is not present, the values that
 *  are greater than and less than are returned.
*/
void WP_findTemp(int table[WP_numTempEntries][WP_numRPMEntries])
{
  for (int i = 1; i < FAN_numTempEntries; i++)
  {

    if (table[i][0] == CAN0_engTemp.value)
    {
      WP_temperatureGreater = i;
      WP_temperatureLesser = i;
      break;
    }

    // check that the lesser will not exceed the lower bounds of the table
    if (table[1][0] > CAN0_engTemp.value)
    {
      WP_temperatureGreater = i;
      WP_temperatureLesser = i;
      break;
    }

    if (table[i][0] > CAN0_engTemp.value)
    {
      WP_temperatureGreater = i;
      WP_temperatureLesser = i - 1;
      break;
    }
  }
}


/*
 *  Given an rpm value read from the sensor, findRPM will look through entries
 *  in the water pump table to determine if the value is present in the table. If the value
 *  is present, the value is returned twice. If the value is not present, the values that
 *  are greater than and less than are returned.
*/
void WP_findRPM(int table[WP_numTempEntries][WP_numRPMEntries])
{
  for (int i = 1; i < WP_numRPMEntries; i++)
  {

    if (table[0][i] == CAN0_rpm.value)
    {
      WP_rpmGreater = i;
      WP_rpmLesser = i;
      break;
    }

    // check that the lesser will not exceed the lower bounds of the table
    if (table[0][1] > CAN0_engTemp.value)
    {
      WP_rpmLesser = i;
      WP_rpmGreater = i;
      break;
    }

    if (table[0][i] > CAN0_rpm.value)
    {
      WP_rpmGreater = i;
      WP_rpmLesser = i - 1;
      break;
    }
  }
}

/*
 *  When a table is input into the setFanSpeed function, the function looks at the global temperature in relation to
 *  tempLesser and tempGreater, then maps it to the corresponding fanspeeds for the lower voltage. The process happens
 *  again but maps to the fanspeeds for the higher voltage. Then, the function looks at the voltage in relation to voltLesser
 *  and voltGreater, and maps it between the results of the first two maps. Simple, right?
*/
int setFanSpeed(int table[FAN_numTempEntries][FAN_numVoltEntries])
{
  // map the actual temp input between the max and min temp in the table,
  // to the corresponding bottom and top rates found in voltLesser
  int map1 = map(CAN0_engTemp.value, table[FAN_temperatureLesser][0], table[FAN_temperatureGreater][0], table[FAN_temperatureLesser][FAN_voltLesser], table[FAN_temperatureGreater][FAN_voltLesser]);


  // do the same as map1, only map it to the corresponding voltLesser values in the fan table
  int map2 = map(CAN0_engTemp.value, table[FAN_temperatureLesser][0], table[FAN_temperatureGreater][0], table[FAN_temperatureLesser][FAN_voltGreater], table[FAN_temperatureGreater][FAN_voltGreater]);



  // now, map the opposite direction in the table, by mapping the actual rpm between the min and max in the table
  // to the results of the previous map
  int fanSpeed = map(BatteryVoltAvg, table[0][FAN_voltLesser], table[0][FAN_voltGreater], map1, map2);



  return fanSpeed;
}

/*
 *  When a table is input into the setWaterPumpSpeed function, the function looks at the global temperature in relation to
 *  tempLesser and tempGreater, then maps it to the corresponding fanspeeds for the lower RPM. The process happens
 *  again but maps to the fanspeeds for the higher RPM. Then, the function looks at the rpm in relation to rpmLesser
 *  and rpmGreater, and maps it between the results of the first two maps. Simple, right?
*/
int setWaterPumpSpeed(int table[WP_numTempEntries][WP_numRPMEntries])
{
  // map the actual temp input between the max and min temp in the table,
  // to the corresponding bottom and top rates found in rpmLesser
  int map1 = map(CAN0_engTemp.value, table[WP_temperatureLesser][0], table[WP_temperatureGreater][0], table[WP_temperatureLesser][WP_rpmLesser], table[WP_temperatureGreater][WP_rpmLesser]);


  // do the same as map1, only map it to the corresponding rpmLesser values in the fan table
  int map2 = map(CAN0_engTemp.value, table[WP_temperatureLesser][0], table[WP_temperatureGreater][0], table[WP_temperatureLesser][WP_rpmGreater], table[WP_temperatureGreater][WP_rpmGreater]);


  // now, map the opposite direction in the table, by mapping the actual rpm between the min and max in the table
  // to the results of the previous map
  int pumpSpeed = map(CAN0_rpm.value, table[0][WP_rpmLesser], table[0][WP_rpmGreater], map1, map2);



  return pumpSpeed;
}
