
//------------------------------------------------------------------------------
//  Written by:     Zoinul Choudhury
//  Created :       10/16/18
//  Modified By:    Dave Yonkers
//  Last Modified:  3/16/2019 11:00 PM
//  Version:        5.0
//  Purpose:        Display useful information to the driver
//  Description:    Driver display code
//------------------------------------------------------------------------------

#include <Adafruit_NeoPixel.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9340.h>
#include <SD.h>
#include <SPI.h>
#include <FlexCAN.h>


static CAN_message_t rxmsg;

// Left screen
#define TFT1_sclk 13
#define TFT1_mosi 11
#define TFT1_cs 18
#define TFT1_dc 20
#define TFT1_rst 19
#define TFT1_bl 1

// Right screen
#define TFT2_sclk 13
#define TFT2_mosi 11
#define TFT2_cs 5
#define TFT2_dc 7
#define TFT2_rst 6
#define TFT2_bl 1

// NeoPixels have their own digital pins
#define pixelsTopPin 16 //6,7,8
#define pixelsLeftPin 15
#define pixelsRightPin 17

#define numPixelsTop 12
#define numPixelsLeft 4
#define numPixelsRight 4

// SD card
//#define SD_CS BUILTIN_SDCARDs

Adafruit_ILI9340 tftLeft = Adafruit_ILI9340(TFT1_cs, TFT1_dc, TFT1_rst);
Adafruit_ILI9340 tftRight = Adafruit_ILI9340(TFT2_cs, TFT2_dc, TFT2_rst);

Adafruit_NeoPixel pixelsTop = Adafruit_NeoPixel(numPixelsTop, pixelsTopPin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsLeft = Adafruit_NeoPixel(numPixelsLeft, pixelsLeftPin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsRight = Adafruit_NeoPixel(numPixelsRight, pixelsRightPin, NEO_GRB + NEO_KHZ800);

//initialize pin constants------------------------------------------------------
int selector = 14;

//initialize screen variables---------------------------------------------------
int currentGear = 0;
int RPM = 0;
int oilTemp = 0;
int engineTemperature = 0;
int fuelTemperature = 0;
float batteryVoltage = 0;
int Throttle = 0;

// variable to hold information from throttle funciton
int lastThrottle = 0;

// structure to store the attributes of messages recieved on the CAN Bus
typedef struct
{
  int16_t value = 0;
}canSensor;

// CAN0 sensors
canSensor CAN0_rpm, CAN0_currentGear, CAN0_oilPressure, CAN0_fuelTemp, CAN0_fuelPressure, CAN0_engTemp;
canSensor CAN0_throttle, CAN0_lastThrottle, CAN0_batteryVoltage, CAN0_oilTemp;
canSensor CAN1_pdmCurrent, CAN1_wpCurrent, CAN1_fanrCurrent, CAN1_wpPWM, CAN1_fanrWPM;

//initialize screen position variables------------------------------------------
const int rpmScreenPos = 1;
const int engineTempScreenPos = 50;
const int oilTempScreenPos = 100;
const int oilPressureScreenPos = 150;
const int batteryVoltScreenPos = 200;

// Left screen
const int pdmCurrentScreenPos = 1;
const int wpCurrentScreenPos = 50;
const int fanrCurrentScreenPos = 100;
const int wpPWMScreenPos = 150;
const int fanrPWMScreenPos = 200;

//initialize warnings-----------------------------------------------------------
int rpmColor = ILI9340_WHITE;
int engineTemperatureColor = ILI9340_WHITE;
int oilTempColor = ILI9340_WHITE;
int oilPressureColor = ILI9340_WHITE;
int batteryVoltageColor = ILI9340_WHITE;

// *** protection 1 is text colow yellow, 2 is red ***

// oil pressure times factor of 10
// colors change with falling values
const int oilTempProtection1 = 800;
const int oilTempProtection2 = 1000;

// temp times factor of 10
// colors change with rising values
const int tempProtection1 = 1000;
const int tempProtection2 = 1200;

// oil pressure times factor of 10
// colors change with falling values
const int oilPressureProtection1 = 350;
const int oilPressureProtection2 = 300;

// oil pressure times factor of 100
// colors change with falling values
const int batteryProtection1 = 1150;
const int batteryProtection2 = 1050;

//initialize rpm bar constants--------------
const int rpmMax = 13500;
const int rpmMin = 1400;
const int shiftPoint = 12000;

//intialize throttle position bar constants--------

const int ledBrightness = 150;       //Value between 0-255 -- 10 for nighttime value, 255 for daylight
const int ledBrightnessFlash = 150; //Value between 0-255

//initialize timers---------------------------
unsigned long rpmTimer = 0;
unsigned long previousTime = 0;
unsigned long time;
int s1 = 0;
int s2 = 0;
int s3 = 0;
int s4 = 0;

//---------------------------------------------------------------------------------------------------
//
//SETUP
//
//---------------------------------------------------------------------------------------------------
void setup()
{
  //initialize communications----------------
  Serial.begin(9600);
  Can0.begin(1000000);
  analogReadResolution(13);

  pinMode(13, OUTPUT);
  digitalWrite(13, 1);

  pinMode(selector, INPUT);
  delay(1000);
  digitalWrite(13, 0);

  // Initialize LEDs------------------------
  pixelsTop.setBrightness(ledBrightness);
  pixelsLeft.setBrightness(ledBrightness);
  pixelsRight.setBrightness(ledBrightness);

  pixelsTop.begin(); // This initializes the NeoPixel library.
  pixelsLeft.begin();
  pixelsRight.begin();

  pixelsTop.show();
  pixelsLeft.show();
  pixelsRight.show();


  //flash the LEDs on to test
  ledInitialize();

  // Initialize Screens-----------------------
  tftLeft.begin();
  tftLeft.setRotation(1);

  tftRight.begin();
  tftRight.setRotation(3);

  tftRight.fillScreen(ILI9340_BLACK);
  tftLeft.fillScreen(ILI9340_BLACK);


//  // initialize SD card
//  Serial.print("Initializing SD card...");
//  if (!SD.begin(SD_CS)) {
//    Serial.println("failed!");
//    return;
//  }
//  Serial.println("OK!");



  // display startup message
  // startupMessage();

  delay(1000);

  clearScreens();
  // pedalPositionScale();


}

//-------------------------------------------------------------------
//
//MAIN
//
//-------------------------------------------------------------------
void loop()
{

//  for (int i = rpmMin; i <= rpmMax; i = i + 100)
//  {
//    CAN0_rpm.value = i;
//    Serial.print("RPM: ");
//    Serial.println(CAN0_rpm.value);
//    //delay(5);
//
//    CAN0_oilTemp.value = random(70, 200); //temp
//    CAN0_engTemp.value = random(70, 200); //temp
//    CAN0_fuelTemp.value = random(70, 200); //temp
//
//    canDecode();
//    rpmReadout();
//    rpmBar();
//s
//    if (CAN0_rpm.value >= shiftPoint)
//      rpmBarFlash();
//  }

  canDecode();


  rpmBar();
  //throttleBar();

  // delay the rpm numbers on the screen by a little so CAN can read reliably
  if (millis() - rpmTimer >= 200)
    {
    rpmTimer = millis();
    rpmReadout();
    oilTempReadout();
    }

  // if (CAN0_rpm.value >= shiftPoint)
  //   rpmBarFlash();

  //pedalPosition(CAN0_throttle.value);



  time = millis();
  if (time - previousTime >= 1000)
  {

    if (digitalRead(selector) == HIGH)
    {

      if (s1 == 0)
      {
        clearScreens();
        s1 = 1;
      }

      else // <-- else was not here before
      {
        previousTime = time;

        showWarnings();
        //tcReadout();
        engineTemperatureReadout();
        // oilTempReadout(); // remove from rpm and uncomment this when engine is reliable
        OilPressureReadout();
        batteryVoltageReadout();

        pdmCurrentReadout();
        fanrCurrentReadout();
        wpCurrentReadout();
        fanrPWMReadout();
        wpPWMReadout();
      }
    }

    else
    {
      if (s1 == 1)
      {
        clearScreens();
        //pedalPositionScale();
        s1 = 0;
      }

      previousTime = time;
      //tcReadout();
      //Show_Warnings_R();
    }
  }
}









//Can Input Conversion-----------------------------------------
void canDecode()
{

  if ( Can0.read(rxmsg) )
  {


    // store the message ID and length
    int rxID = rxmsg.id;
    int rxDataLen = rxmsg.len;

    // assign the message data into an easily readable array, rxData
    int rxData[rxDataLen];
    for(int i=0; i<rxDataLen; i++)
    {
      rxData[i]=rxmsg.buf[i];
      //Serial.print(rxData[i]);
      //Serial.print(" ");
    }

    // assign the first byte as the multiplexor ID
    int rxMultID = rxData[0];

    switch(rxID)
    {

      // MOTEC M400 MESSAGES *KEEP AT TOP*

      // M400_dataSet2
      // ID 0x5EF
      case 0x5EF:
      {

        // read the multiplexor
        switch(rxMultID)
        {

          // MultID 0x0
          case 0x0:
            CAN0_batteryVoltage.value = rxData[2] * 256 + rxData[3];
            CAN0_rpm.value = rxData[4] * 256 + rxData[5];
            break;

          // MultID 0x4
          case 0x4:
            CAN0_engTemp.value = rxData[4] * 256 + rxData[5];
            CAN0_fuelPressure.value = rxData[6] * 256 + rxData[7];
            break;

          // MultID 0x5
          case 0x5:
            CAN0_oilPressure.value = rxData[4] * 256 + rxData[5];
            CAN0_oilTemp.value = rxData[6] * 256 + rxData[7];
            break;

          // MultID 0x6
          case 0x6:
            CAN0_throttle.value = rxData[2] * 256 + rxData[3];
            break;

          // MultID 0x9
          case 0x9:
            CAN0_fuelTemp.value = rxData[6] * 256 + rxData[7];
            break;

          // MultID 0xE
          case 0xE:
            CAN0_currentGear.value = rxData[2] * 256 + rxData[3];
            break;

        }

      }

      // PDM msgs (intel byte order!)
      case 0x9A:
        CAN1_pdmCurrent.value = rxData[3] + rxData[4] * 256;
        break;

      case 0x97:
        CAN1_fanrCurrent.value = rxData[3] + rxData[4] * 256;
        break;

      case 0x99:
        CAN1_wpCurrent.value = rxData[3] + rxData[4] * 256;
        break;

      case 0xA3:
        CAN1_wpPWM.value = rxData[3];
        CAN1_fanrWPM.value = rxData[1];
        break;

    }

  }

}











//change the text color of readouts if there is a warning--------
void showWarnings()
{

  // Set oil temperature color
  if (CAN0_oilTemp.value <= oilTempProtection2)
    oilTempColor = ILI9340_RED;

  else if (CAN0_oilTemp.value <= oilTempProtection1)
    oilTempColor = ILI9340_YELLOW;

  else
    oilTempColor = ILI9340_WHITE;

  // Set engine temperature color
  if (CAN0_engTemp.value >= tempProtection2)
    engineTemperatureColor = ILI9340_RED;

  else if (CAN0_engTemp.value >= tempProtection1)
    engineTemperatureColor = ILI9340_YELLOW;

  else
    engineTemperatureColor = ILI9340_WHITE;

  // Set oil pressure color
  if (CAN0_oilPressure.value <= oilPressureProtection2)
    oilPressureColor = ILI9340_RED;

  else if (CAN0_oilPressure.value <= oilPressureProtection1)
    oilPressureColor = ILI9340_YELLOW;

  else
    oilPressureColor = ILI9340_WHITE;

  // Set battery voltage color
  if (CAN0_batteryVoltage.value <= batteryProtection2)
    batteryVoltageColor = ILI9340_RED;

  else if (CAN0_batteryVoltage.value <= batteryProtection2)
    batteryVoltageColor = ILI9340_YELLOW;

  else
    batteryVoltageColor = ILI9340_WHITE;
}

//display information on screens-------------------------

void gearReadout()
{
  //display gear number
  tftRight.setCursor(140, 15);
  tftRight.setTextColor(ILI9340_WHITE, ILI9340_BLACK);
  tftRight.setTextSize(30);
  tftRight.print(CAN0_currentGear.value);

}

void tcReadout()
{
  //display tc settings
  tftRight.setCursor(40, 85);
  tftRight.setTextColor(ILI9340_WHITE, ILI9340_BLACK);
  tftRight.setTextSize(10);
  tftRight.print("TC");
}

void rpmReadout()
{
  char out[6];
  sprintf(out, "%05d", CAN0_rpm.value);

  tftLeft.setCursor(170, rpmScreenPos);
  tftLeft.setTextColor(rpmColor, ILI9340_BLACK);
  tftLeft.setTextSize(5);
  tftLeft.print(out);
  // Serial.println(out);
}

void engineTemperatureReadout()
{
  // factor down by 10
  double engineTempDouble = (double)CAN0_engTemp.value;
  engineTempDouble /= 10;

  char out[6];
  sprintf(out, "%5.1f", engineTempDouble);

  tftLeft.setCursor(170, engineTempScreenPos);
  tftLeft.setTextColor(engineTemperatureColor, ILI9340_BLACK);
  tftLeft.setTextSize(5);
  tftLeft.print(out);
}

void oilTempReadout()
{
  // facter oil presure down by a factor of 10
  double oilTempDouble = (double)CAN0_oilTemp.value;
  oilTempDouble /= 10;


  char out[6];
  sprintf(out, "%5.1f", oilTempDouble);

  tftLeft.setCursor(170, oilTempScreenPos);
  tftLeft.setTextColor(oilTempColor, ILI9340_BLACK);
  tftLeft.setTextSize(5);
  tftLeft.print(out);
}

void OilPressureReadout()
{
  char out[6];

  // turn oil pressure int into a double, then divide by factor 10
  double oilPressureDouble = (double)CAN0_oilPressure.value;
  oilPressureDouble /= 10;

  sprintf(out, "%5.1f", oilPressureDouble);

  tftLeft.setCursor(170, oilPressureScreenPos);
  tftLeft.setTextColor(oilPressureColor, ILI9340_BLACK);
  tftLeft.setTextSize(5);
  tftLeft.print(out);
}

void batteryVoltageReadout()
{
  char out[6];

  // turn the 4 digit int into a double, then divide by 100 to get voltage
  double batteryVoltageDouble = (double)CAN0_batteryVoltage.value;
  batteryVoltageDouble /= 100;

  sprintf(out, "%5.2f", batteryVoltageDouble);

  tftLeft.setCursor(170, batteryVoltScreenPos);
  tftLeft.setTextColor(batteryVoltageColor, ILI9340_BLACK);
  tftLeft.setTextSize(5);
  tftLeft.print(out);
}



void pdmCurrentReadout()
{
  char out[6];

  // frankenstiened(?) fuel pressure in here for christian ;)
  double fuelPressureDouble = (double)CAN0_fuelPressure.value;
  fuelPressureDouble /= 10;
  sprintf(out, "%5.1f", fuelPressureDouble);

  tftRight.setCursor(170, pdmCurrentScreenPos);
  tftRight.setTextColor(ILI9340_WHITE, ILI9340_BLACK);
  tftRight.setTextSize(5);
  tftRight.print(out);
}

void wpCurrentReadout()
{
  char out[6];

  double currentDouble = (double)CAN1_wpCurrent.value;
  currentDouble /= 100;
  sprintf(out, "%5.2f", currentDouble);

  tftRight.setCursor(170, wpCurrentScreenPos);
  tftRight.setTextColor(ILI9340_WHITE, ILI9340_BLACK);
  tftRight.setTextSize(5);
  tftRight.print(out);
}

void fanrCurrentReadout()
{
  char out[6];

  double currentDouble = (double)CAN1_fanrCurrent.value;
  currentDouble /= 100;
  sprintf(out, "%5.2f", currentDouble);

  tftRight.setCursor(170, fanrCurrentScreenPos);
  tftRight.setTextColor(ILI9340_WHITE, ILI9340_BLACK);
  tftRight.setTextSize(5);
  tftRight.print(out);
}

void wpPWMReadout()
{
  char out[6];

  sprintf(out, "%3d", CAN1_wpPWM.value);

  tftRight.setCursor(230, wpPWMScreenPos);
  tftRight.setTextColor(ILI9340_RED, ILI9340_BLACK);
  tftRight.setTextSize(5);
  tftRight.print(out);
}

void fanrPWMReadout()
{
  char out[6];

  sprintf(out, "%3d", CAN1_fanrWPM.value);

  tftRight.setCursor(230, fanrPWMScreenPos);
  tftRight.setTextColor(ILI9340_RED, ILI9340_BLACK);
  tftRight.setTextSize(5);
  tftRight.print(out);
}


//display the brake and throttle bars----------------------------
void pedalPosition(int nThrottle)
{

//  if (nThrottle < lastThrottle) {
//    tftLeft.fillRect(20, 80 + (160 - lastThrottle), 100, lastThrottle - nThrottle,  ILI9340_BLACK);
//  }
//  else {
//    tftLeft.fillRect(20, 80 + (160 - nThrottle), 100, nThrottle - lastThrottle,  ILI9340_RED);
//  }
  if (nThrottle < lastThrottle)
    tftLeft.fillRect(200, 81 + (160 - lastThrottle), 100, lastThrottle - nThrottle,  ILI9340_BLACK);

  else
    tftLeft.fillRect(200, 81 + (160 - nThrottle), 100, nThrottle - lastThrottle,  ILI9340_GREEN);

  lastThrottle = nThrottle;
}

//display the scale of throttle and brake bars-------------------
void pedalPositionScale()
{

  tftLeft.drawFastVLine(15, 79, 161, ILI9340_WHITE); // Vertical Scale Line
  tftLeft.drawFastHLine(0, 79, 15, ILI9340_WHITE); // Major Division
  tftLeft.drawFastHLine(5, 119, 10, ILI9340_WHITE); // Minor Division
  tftLeft.drawFastHLine(0, 159, 15, ILI9340_WHITE); // Major Division
  tftLeft.drawFastHLine(5, 199, 10, ILI9340_WHITE); // Minor Division
  tftLeft.drawFastHLine(0, 239, 15, ILI9340_WHITE);  // Major Division

  tftLeft.drawFastVLine(305, 79, 161, ILI9340_WHITE); // Vertical Scale Line
  tftLeft.drawFastHLine(305, 79, 15, ILI9340_WHITE); // Major Division
  tftLeft.drawFastHLine(305, 119, 10, ILI9340_WHITE); // Minor Division
  tftLeft.drawFastHLine(305, 159, 15, ILI9340_WHITE); // Major Division
  tftLeft.drawFastHLine(305, 199, 10, ILI9340_WHITE); // Minor Division
  tftLeft.drawFastHLine(305, 239, 15, ILI9340_WHITE);  // Major Division

}


//clear the screens--------------------------------------
void clearScreens()
{

  tftRight.fillScreen(ILI9340_BLACK);
  tftLeft.fillScreen(ILI9340_BLACK);

  // Print "RPM:"
  tftLeft.setCursor(1, rpmScreenPos);
  tftLeft.setTextColor(rpmColor, ILI9340_BLACK);
  tftLeft.setTextSize(5);
  tftLeft.print("RPM:");

  // Print "ENG:"
  tftLeft.setCursor(1, engineTempScreenPos);
  tftLeft.setTextColor(engineTemperatureColor, ILI9340_BLACK);
  tftLeft.setTextSize(5);
  tftLeft.print("ENG:");

  //Print "OILT:"
  tftLeft.setCursor(1, oilTempScreenPos);
  tftLeft.setTextColor(oilTempColor, ILI9340_BLACK);
  tftLeft.setTextSize(5);
  tftLeft.print("OILT:");

  // Print "OILP:"
  tftLeft.setCursor(1, oilPressureScreenPos);
  tftLeft.setTextColor(oilPressureColor, ILI9340_BLACK);
  tftLeft.setTextSize(5);
  tftLeft.print("OILP:");

  // Print "VOLT:"
  tftLeft.setCursor(1, batteryVoltScreenPos);
  tftLeft.setTextColor(batteryVoltageColor, ILI9340_BLACK);
  tftLeft.setTextSize(5);
  tftLeft.print("VOLT:");

  // right
  // Print
  tftRight.setCursor(1, pdmCurrentScreenPos);
  tftRight.setTextColor(ILI9340_WHITE, ILI9340_BLACK);
  tftRight.setTextSize(5);
  tftRight.print("FUELP:");

  // Print
  tftRight.setCursor(1, wpCurrentScreenPos);
  tftRight.setTextColor(ILI9340_WHITE, ILI9340_BLACK);
  tftRight.setTextSize(5);
  tftRight.print("WP:");

  //Print
  tftRight.setCursor(1, fanrCurrentScreenPos);
  tftRight.setTextColor(ILI9340_WHITE, ILI9340_BLACK);
  tftRight.setTextSize(5);
  tftRight.print("FANR:");

  // Print
  tftRight.setCursor(1, wpPWMScreenPos);
  tftRight.setTextColor(ILI9340_RED, ILI9340_BLACK);
  tftRight.setTextSize(5);
  tftRight.print("WP:");

  // Print
  tftRight.setCursor(1, fanrPWMScreenPos);
  tftRight.setTextColor(ILI9340_RED, ILI9340_BLACK);
  tftRight.setTextSize(5);
  tftRight.print("FANR:");

}

//display "CAR 16" on the screens------------------------
void startupMessage()
{

  tftRight.fillScreen(ILI9340_BLACK);
  tftLeft.fillScreen(ILI9340_BLACK);

  bmpDraw(tftLeft, "left.bmp", 0, 0);
  bmpDraw(tftRight, "right.bmp", 0, 0);

  delay(5000);

  tftRight.fillScreen(ILI9340_BLACK);
  tftLeft.fillScreen(ILI9340_BLACK);

  tftRight.setCursor(10, 65);
  tftRight.setTextColor(ILI9340_GREEN, ILI9340_BLACK);
  tftRight.setTextSize(17);
  tftRight.print("CAR");

  tftLeft.setCursor(75, 65);
  tftLeft.setTextColor(ILI9340_GREEN, ILI9340_BLACK);
  tftLeft.setTextSize(17);
  tftLeft.print("11");

}

/*
 * This function illuminates the appropriate LEDs on top of the driver display based on
 * the engines RPM.
 */
void rpmBar()
{
  int ledValue = map(CAN0_rpm.value, rpmMin, rpmMax, 0, 1800);

  //         .setPixelColor(LED#, red, green, blue)
  //----------------------------------------------------------------------------
  if (ledValue > 1650 && ledValue <= 1800)
  {
    pixelsTop.setPixelColor(11, ledValue - 1650, 0, 0);

    // Turn these LEDs on
    for (int led = 0; led <= 10; led++)
    {
      if (led <= 4)
        pixelsTop.setPixelColor(led, 0, 150, 0);

      else if (led <= 8)
        pixelsTop.setPixelColor(led, 150, 150, 0);

      else
        pixelsTop.setPixelColor(led, 150, 0, 0);
    }
  }

  //----------------------------------------------------------------------------
  else if (ledValue > 1500 && ledValue <= 1650)
  {
    pixelsTop.setPixelColor(10, ledValue - 1500, 0, 0);

    // Turn these LEDs on
    for (int led = 0; led <= 9; led++)
    {
      if (led <= 4)
        pixelsTop.setPixelColor(led, 0, 150, 0);

      else if (led <= 8)
        pixelsTop.setPixelColor(led, 150, 150, 0);

      else
        pixelsTop.setPixelColor(led, 150, 0, 0);
    }

    // Turn these LEDs off
    pixelsTop.setPixelColor(11, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  else if (ledValue > 1350 && ledValue <= 1500)
  {
    pixelsTop.setPixelColor(9,  ledValue - 1350, 0, 0);

    // Turn these LEDs on
    for (int led = 0; led <= 8; led++)
    {
      if (led <= 4)
        pixelsTop.setPixelColor(led, 0, 150, 0);

      else
        pixelsTop.setPixelColor(led, 150, 150, 0);
    }

    // Turn these LEDs off
    for (int led = 10; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  else if (ledValue > 1200 && ledValue <= 1350)
  {
    pixelsTop.setPixelColor(8,  ledValue - 1200, ledValue - 1200, 0);

    // Turn these LEDs on
    for (int led = 0; led <= 7; led++)
    {
      if (led <= 4)
        pixelsTop.setPixelColor(led, 0, 150, 0);

      else
        pixelsTop.setPixelColor(led, 150, 150, 0);
    }

    // Turn these LEDs off
    for (int led = 9; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  else if (ledValue > 1050 && ledValue <= 1200)
  {
    pixelsTop.setPixelColor(7,  ledValue - 1050, ledValue - 1050 , 0);

    // Turn these LEDs on
    for (int led = 0; led <= 6; led++)
    {
      if (led <= 4)
        pixelsTop.setPixelColor(led, 0, 150, 0);

      else
        pixelsTop.setPixelColor(led, 150, 150, 0);
    }

    // Turn these LEDs off
    for (int led = 8; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  else if (ledValue > 900 && ledValue <= 1050)
  {
    pixelsTop.setPixelColor(6,  ledValue - 900, ledValue - 900 , 0);

    // Turn these LEDs on
    for (int led = 0; led <= 5; led++)
    {
      if (led <= 4)
        pixelsTop.setPixelColor(led, 0, 150, 0);

      else
        pixelsTop.setPixelColor(led, 150, 150, 0);
    }

    // Turn these LEDs off
    for (int led = 7; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }
  //----------------------------------------------------------------------------
  else if (ledValue > 750 && ledValue <= 900)
  {
    pixelsTop.setPixelColor(5,  ledValue - 750, ledValue - 750 , 0);

    // Turn these LEDs on
    for (int led = 0; led <= 4; led++)
        pixelsTop.setPixelColor(led, 0, 150, 0);

    // Turn these LEDs off
    for (int led = 6; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }
  //----------------------------------------------------------------------------
  else if (ledValue > 600 && ledValue <= 750)
  {
    pixelsTop.setPixelColor(4,  0, ledValue - 600 , 0);

    // Turn these LEDs on
    for (int led = 0; led <= 3; led++)
        pixelsTop.setPixelColor(led, 0, 150, 0);

    // Turn these LEDs off
    for (int led = 5; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }
  //----------------------------------------------------------------------------
  else if (ledValue > 450 && ledValue <= 600)
  {
    pixelsTop.setPixelColor(3,  0, ledValue - 450 , 0);

    // Turn these LEDs on
    for (int led = 0; led <= 2; led++)
        pixelsTop.setPixelColor(led, 0, 150, 0);

    // Turn these LEDs off
    for (int led = 4; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  else if (ledValue > 300 && ledValue <= 450)
  {
    pixelsTop.setPixelColor(2,  0, ledValue - 300 , 0);

    // Turn these LEDs on
    for (int led = 0; led <= 1; led++)
        pixelsTop.setPixelColor(led, 0, 150, 0);

    // Turn these LEDs off
    for (int led = 3; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  else if (ledValue > 150 && ledValue <= 300)
  {
    pixelsTop.setPixelColor(1,  0, ledValue - 150 , 0);

    // Turn these LEDs on
    pixelsTop.setPixelColor(0, 0, 150, 0);

    // Turn these LEDs off
    for (int led = 2; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  else
  {
    pixelsTop.setPixelColor(0, 0, ledValue, 0);

    // Turn these LEDs off
    for (int led = 1; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }
  //----------------------------------------------------------------------------
  pixelsTop.show();
}








// copy of rpm bar function with temporary ability for throttle
void throttleBar()
{

  int ledValue = map(CAN0_throttle.value, 0, 1000, 0, 600);

  //         .setPixelColor(LED#, red, green, blue)
  //----------------------------------------------------------------------------
  if (ledValue > 450 && ledValue <= 600)
  {
    pixelsRight.setPixelColor(3,  ledValue - 450, 0, 0);

    // Turn these LEDs on
    pixelsRight.setPixelColor(2, 150, 50, 0);
    pixelsRight.setPixelColor(1, 75, 150, 0);
    pixelsRight.setPixelColor(0, 0, 150, 0);



  }

  //----------------------------------------------------------------------------
  else if (ledValue > 300 && ledValue <= 450)
  {
    pixelsRight.setPixelColor(2,  ledValue - 300, (ledValue - 300) / 3 , 0);

    // Turn these LEDs on
    pixelsRight.setPixelColor(1, 50, 150, 0);
    pixelsRight.setPixelColor(0, 0, 150, 0);

    // Turn these LEDs off
    pixelsRight.setPixelColor(3, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  else if (ledValue > 150 && ledValue <= 300)
  {
    pixelsRight.setPixelColor(1,  (ledValue / 3) - 50, ledValue - 150 , 0);

    // Turn these LEDs on
    pixelsRight.setPixelColor(0, 0, 150, 0);

    // Turn these LEDs off
    pixelsRight.setPixelColor(2, 0, 0, 0);
    pixelsRight.setPixelColor(3, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  else
  {
    pixelsRight.setPixelColor(0, 0, ledValue, 0);

    // Turn these LEDs off
    for (int led = 1; led <= 3; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }
  //----------------------------------------------------------------------------
  pixelsRight.show();
}











void rpmBarFlash()
{
    pixelsTop.setBrightness(ledBrightnessFlash);

    for (int i = 0; i < numPixelsTop; i++)
      pixelsTop.setPixelColor(i, 150, 0, 0);

    pixelsTop.show();
    delay(30); //temp

    pixelsTop.setBrightness(ledBrightness);
    pixelsTop.clear();

    pixelsTop.show();
    delay(30); //temp
}

//flash the leds on startup------------------------------
void ledInitialize()
{
  int led = 0;
  int ledt = numPixelsTop;
  int ledt2 = 6;

  for (int i = 0; i < 10; i++)
  {

    pixelsTop.setPixelColor(led - 4, 0, 30, 0);
    pixelsTop.show();

    if (led >= 3)
    {
      pixelsTop.setPixelColor(ledt, 0, 30, 0);
      pixelsTop.show();

      ledt--;
    }

    pixelsLeft.setPixelColor(led, 0, 30, 0);
    pixelsLeft.show();

    pixelsRight.setPixelColor(led, 0, 30, 0);
    pixelsRight.show();

    led++;

    delay(80);
  }

  for (int i = 11; i > 0; i--)
  {

    pixelsTop.setPixelColor(led - 4, 0, 0, 0);
    pixelsTop.show();

    if (led >= 3)
    {
      pixelsTop.setPixelColor(ledt2, 0, 0, 0);
      pixelsTop.show();

      ledt2++;
    }

    pixelsLeft.setPixelColor(led, 0, 0, 0);
    pixelsLeft.show();

    pixelsRight.setPixelColor(led, 0, 0, 0);
    pixelsRight.show();

    led--;

    delay(80);
  }
}

/*
 * The code below is from the spitftbitmap example in the
 * Adafruit_ILI9340 library
 */

#define BUFFPIXEL 20

void bmpDraw(Adafruit_ILI9340 tft, const char *filename, uint16_t x, uint16_t y)
{

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == false) {
    Serial.print("File not found");
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print("File size: "); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print("Image Offset: "); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print("Header size: "); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print("Bit Depth: "); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print("Image size: ");
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.Color565(r,g,b));
          } // end pixel
        } // end scanline
        Serial.print("Loaded in ");
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println("BMP format not recognized.");
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File & f)
{
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File & f)
{
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
