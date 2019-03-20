#include <Adafruit_NeoPixel.h>

// neopixels have their own digital pins
#define PIN_TOP 16 //6,7,8
#define PIN_L 15
#define PIN_R 17

#define numPixelsTop 12
#define numPixelsLeft 4
#define numPixelsRight 4

Adafruit_NeoPixel pixelsTop = Adafruit_NeoPixel(numPixelsTop, PIN_TOP, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsLeft = Adafruit_NeoPixel(numPixelsLeft, PIN_L, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsRight = Adafruit_NeoPixel(numPixelsRight, PIN_R, NEO_GRB + NEO_KHZ800);

#include <FlexCAN.h>
#include <kinetis_flexcan.h>
FlexCAN CANbus(1000000);
static CAN_message_t rxmsg;
static CAN_message_t msg;

//initialize constants-----------------------
const int ledBrightness = 80;       //Value between 0-255
const int ledBrightnessFlash = 150; //Value between 0-255

//Number of LEDs on each side of the driver display
const int ledCountTop = 12;
const int ledCountLeft = 4;
const int ledCountRight = 4;

//initialize rpm bar constants--------------

const int rpmMax = 13500;
const int rpmMin = 1400;
const int shiftPoint = 12000;

//initialize screen variables----------------
int gearNumber = 0;
int RPM = 0;
int oilTemperature = 0;
int engineTemperature = 0;
int fuelTemperature = 0;
int batteryVoltage = 0;

//initialize timer---------------------------
unsigned long previousTime = 0;
unsigned long time;


//---------------------------------------------------------------------------------------------------
//
//SETUP
//
//---------------------------------------------------------------------------------------------------
void setup()
{
  //initialize communications----------------
  Serial.begin(9600);
  CANbus.begin();

  pinMode(13, OUTPUT);
  digitalWrite(13, 1);

  delay(1000);

  digitalWrite(13, 0);
  pixelsTop.setBrightness(ledBrightness);


  //initialize leds------------------------
  pixelsTop.begin(); // This initializes the NeoPixel library.
  pixelsLeft.begin();
  pixelsRight.begin();

  pixelsTop.show();
  pixelsLeft.show();
  pixelsRight.show();

  delay(1500);

  //flash the LEDs on to test
  ledInitialize();
}


//-------------------------------------------------------------------
//
//MAIN
//
//-------------------------------------------------------------------
void loop()
{

  canDecode();

  for (int i = rpmMin; i <= rpmMax; i = i + 100)
  {
    RPM = i;
    Serial.print("RPM: ");
    Serial.println(RPM);
    delay(5);

    rpmBar();

    if (RPM >= shiftPoint)
      rpmBarFlash();
  }

  //Show_Warnings();
  // time = millis();
  // rpmBar();
  // //Left_Bar();
  // //Right_Bar();
  //
  // if (RPM >= shiftPoint)
  //   rpmBarFlash();
}

void canDecode()
{

  if ( CANbus.read(rxmsg) )
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
      rxData[i] = rxByte[i + 8];

    //ID 0x101 contains RPM, Throttle Pos
    if (rxID == 257)
      RPM = rxData[0] * 255 + rxData[1];
  }
}

/*
 * This function illuminates the appropriate LEDs on top of the driver display based on
 * the engines RPM.
 */
void rpmBar()
{
  int ledValue = map(RPM, rpmMin, rpmMax, 0, 1800);

  //         .setPixelColor(LED#, red, green, blue)
  //----------------------------------------------------------------------------
  if (ledValue >= 1800)
    pixelsTop.setPixelColor(11, 150, 0, 0);

  else
    pixelsTop.setPixelColor(11, ledValue - 1650, 0, 0);

  //----------------------------------------------------------------------------
  if (ledValue >= 1650)
    pixelsTop.setPixelColor(10, 150, 0, 0);

  else
  {
    pixelsTop.setPixelColor(10, ledValue - 1500, 0, 0);

    // Turn these LEDs off
    pixelsTop.setPixelColor(11, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  if (ledValue >= 1500)
    pixelsTop.setPixelColor(9, 150, 0, 0);

  else
  {
    pixelsTop.setPixelColor(9,  ledValue - 1350, 0, 0);

    // Turn these LEDs off

    for (int led = 10; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  if (ledValue >= 1350)
    pixelsTop.setPixelColor(8, 150, 150, 0);

  else
  {
    pixelsTop.setPixelColor(8,  ledValue - 1200, ledValue - 1200, 0);

    // Turn these LEDs off
    for (int led = 9; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  if (ledValue >= 1200)
    pixelsTop.setPixelColor(7, 150, 150, 0);

  else
  {
    pixelsTop.setPixelColor(7,  ledValue - 1050, ledValue - 1050 , 0);

    // Turn these LEDs off
    for (int led = 8; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  if (ledValue >= 1050)
    pixelsTop.setPixelColor(6, 150, 150, 0);

  else
  {
    pixelsTop.setPixelColor(6,  ledValue - 900, ledValue - 900 , 0);

    // Turn these LEDs off
    for (int led = 7; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }
  //----------------------------------------------------------------------------
  if (ledValue >= 900)
    pixelsTop.setPixelColor(5, 150, 150, 0);

  else
  {
    pixelsTop.setPixelColor(5,  ledValue - 750, ledValue - 750 , 0);

    // Turn these LEDs off
    for (int led = 6; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }
  //----------------------------------------------------------------------------
  if (ledValue >= 750)
    pixelsTop.setPixelColor(4, 0, 150, 0);

  else
  {
    pixelsTop.setPixelColor(4,  0, ledValue - 600 , 0);

    // Turn these LEDs off
    for (int led = 5; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }
  //----------------------------------------------------------------------------
  if (ledValue >= 600)
    pixelsTop.setPixelColor(3, 0, 150, 0);

  else
  {
    pixelsTop.setPixelColor(3,  0, ledValue - 450 , 0);

    // Turn these LEDs off
    for (int led = 4; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  if (ledValue >= 450)
    pixelsTop.setPixelColor(2, 0, 150, 0);

  else
  {
    pixelsTop.setPixelColor(2,  0, ledValue - 300 , 0);

    // Turn these LEDs off
    for (int led = 3; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  if (ledValue >= 300)
    pixelsTop.setPixelColor(1, 0, 150, 0);

  else
  {
    pixelsTop.setPixelColor(1,  0, ledValue - 150 , 0);

    // Turn these LEDs off
    for (int led = 2; led <= 11; led++)
      pixelsTop.setPixelColor(led, 0, 0, 0);
  }

  //----------------------------------------------------------------------------
  if (ledValue >= 150)
    pixelsTop.setPixelColor(0, 0, 150, 0);

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

void rpmBarFlash()
{

    for (int i = 0; i < numPixelsTop; i++)
    {
      pixelsTop.setBrightness(ledBrightnessFlash);
      pixelsTop.setPixelColor(i, 150, 0, 0);
      pixelsTop.show();
    }
    
    delay(10);

    for (int i = 0; i < numPixelsTop; i++)
    {
      pixelsTop.setBrightness(ledBrightness);
      pixelsTop.setPixelColor(i, 0, 0, 0);
      pixelsTop.show();
    }
    
    delay(10);
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
