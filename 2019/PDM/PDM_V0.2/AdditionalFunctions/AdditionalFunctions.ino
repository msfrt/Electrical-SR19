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