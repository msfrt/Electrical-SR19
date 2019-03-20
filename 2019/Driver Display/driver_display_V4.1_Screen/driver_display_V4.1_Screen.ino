/*
 * Created By: Zoinul Choudhury
 * Created On: 10/16/18
 * 
 * Modified By: Zoinul Choudhury
 * Modified On: 10/16/18
 */

#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <SPI.h>
#include <FlexCAN.h>

FlexCAN CANbus(1000000);
static CAN_message_t rxmsg;

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9340.h>

// right screen
#define TFT1_sclk 13
#define TFT1_mosi 11
#define TFT1_cs 5
#define TFT1_dc 7
#define TFT1_rst 6
#define TFT1_bl 1

// left screen
#define TFT2_sclk 13
#define TFT2_mosi 11
#define TFT2_cs 18
#define TFT2_dc 20
#define TFT2_rst 19
#define TFT2_bl 1

Adafruit_ILI9340 tft_l = Adafruit_ILI9340(TFT1_cs, TFT1_dc, TFT1_rst);
Adafruit_ILI9340 tft_r = Adafruit_ILI9340(TFT2_cs, TFT2_dc, TFT2_rst);

//initialize pin constants-----------------------
int selector = 14;

//initialize screen variables----------------
int currentGear = 2;
int RPM = 0;
int oilTemperature = 0;
int engineTemperature = 0;
int fuelTemperature = 0;
float batteryVoltage = 0;
int Throttle = 0;
int lastThrottle = 0;

//initialize screen position variables-------
#define rpmScreenPos 1
#define engineTempScreenPos 50
#define oilTempScreenPos 100
#define fuelTempScreenPos 150
#define batteryVoltScreenPos 200

//initialize warnings------------------------
int rpmColor = ILI9340_WHITE;
int engineTemperatureColor = ILI9340_WHITE;
int oilTemperatureColor = ILI9340_WHITE;
int fuelTemperatureColor = ILI9340_WHITE;
int batteryVoltageColor = ILI9340_WHITE;

#define oilProtection1 80
#define oilProtection2 100

#define engineProtection1 80
#define engineProtection2 100

#define fuelProtection1 0
#define fuelProtection2 0

#define batteryProtection1 11.5
#define batteryProtection2 10.5

//initialize timers---------------------------
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
  //Serial.begin(9600);
  //delay(1000);
  CANbus.begin();
  analogReadResolution(13);
  
  pinMode(13, OUTPUT);
  digitalWrite(13, 1);
  
  pinMode(selector, INPUT);
  delay(1000);
  digitalWrite(13, 0);

  //initialize Screens-----------------------
  tft_l.begin();
  tft_l.setRotation(1);
  
  tft_r.begin();
  tft_r.setRotation(3);

  tft_r.fillScreen(ILI9340_BLACK);
  tft_l.fillScreen(ILI9340_BLACK);

  delay(1000);

  //display startup message
  startupMessage();

  delay(1000);

  clearScreens();
  pedalPositionScale();

}

//-------------------------------------------------------------------
//
//MAIN
//
//-------------------------------------------------------------------
void loop()
{

  RPM = random(2000, 12000); //temp
  oilTemperature = random(70, 200); //temp
  engineTemperature = random(70, 200); //temp
  fuelTemperature = random(70, 200); //temp
  
  canDecode();
  rpmReadout();
  
  //pedalPosition(Throttle);
  
  //Serial.println(RPM);
  
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
        tcReadout();
        engineTemperatureReadout();
        oilTemperatureReadout();
        fuelTemperatureReadout();
        batteryVoltageReadout();
      }
    }

    else 
    {
      if (s1 == 1)
      {
        clearScreens();
        pedalPositionScale();
        s1 = 0;
      }

      previousTime = time;
      tcReadout();
      //Show_Warnings_R();
    }
  }
}
//Can Input Conversion-----------------------------------------
void canDecode()
{
  //RPM = analogRead(A1);
  if (CANbus.read(rxmsg))
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

    //ID 0x101 contains
    switch(rxID)
    { 
      // ID 0x101    
      case 257:
        RPM = rxData[0] * 255 + rxData[1];
        
        Throttle = rxData[2] * 255 + rxData[3];
        Throttle = Throttle / 10;
        
        //Serial.print(Throttle);
        
        //Brake Pressure Front = rxData[4] * 255      + rxData[5];
        //Brake Pressure Rear  = rxData[6] * 255      + rxData[7];
        
        break;

      // ID 0x102
      case 258:
        //TC_SET  = rxData[0] * 255      + rxData[1];
        //TC_SET = (short) ( ((uint8_t) rxData[0] << 8) + ((uint8_t) rxData[1] ) );

        /*
        if (currentGear != (short) (((uint8_t) rxData[2] << 8) + ((uint8_t) rxData[3])))
        {
          currentGear = (short) (((uint8_t) rxData[2] << 8) + ((uint8_t) rxData[3]));
          gearReadout(); // Only update onscreen when gear changes
        }
        */
        
        oilTemperature = rxData[4] * 255      + rxData[5];
        oilTemperature = oilTemperature / 1000;
        
        engineTemperature = rxData[6] * 255      + rxData[7]; 
        engineTemperature = engineTemperature / 1000;
        break;

      // ID 0x103
      case 259:
        batteryVoltage = rxData[0] * 255      + rxData[1]; 
        break;
    }
  }
}

//change the text color of readouts if there is a warning--------
void showWarnings()
{

  // Set oil temperature color
  if (oilTemperature >= oilProtection2)
    oilTemperatureColor = ILI9340_RED;
      
  else if (oilTemperature >= oilProtection1)
    oilTemperatureColor = ILI9340_YELLOW;
  
  else
    oilTemperatureColor = ILI9340_WHITE;

  // Set engine temperature color
  if (engineTemperature >= engineProtection2)
    engineTemperatureColor = ILI9340_RED;

  else if (engineTemperature >= engineProtection1)
    engineTemperatureColor = ILI9340_YELLOW;
  
  else
    engineTemperatureColor = ILI9340_WHITE;

  // Set fuel temperature color
  if (fuelTemperature >= fuelProtection2)
    fuelTemperatureColor = ILI9340_RED;
    
  else if (fuelTemperature >= fuelProtection1)
    fuelTemperatureColor = ILI9340_YELLOW;
  
  else
    fuelTemperatureColor = ILI9340_WHITE;

  // Set battery voltage color
  if (batteryVoltage <= batteryProtection1)
    batteryVoltageColor = ILI9340_RED;
    
  else if (batteryVoltage <= batteryProtection2)
    batteryVoltageColor = ILI9340_YELLOW;
  
  else
    batteryVoltageColor = ILI9340_WHITE;
}

//display information on screens-------------------------

void gearReadout()
{
  //display gear number
  tft_r.setCursor(140, 15);
  tft_r.setTextColor(ILI9340_WHITE, ILI9340_BLACK);
  tft_r.setTextSize(30);
  tft_r.print(currentGear);

}

void tcReadout()
{
  //display tc settings
  tft_r.setCursor(40, 85);
  tft_r.setTextColor(ILI9340_WHITE, ILI9340_BLACK);
  tft_r.setTextSize(10);
  tft_r.print("W");
}

void rpmReadout()
{
  char out[6];
  sprintf(out, "%05d", RPM);
  
  tft_l.setCursor(170, rpmScreenPos);
  tft_l.setTextColor(rpmColor, ILI9340_BLACK);
  tft_l.setTextSize(5);
  tft_l.print(out);
  Serial.println(out);
}

void engineTemperatureReadout()
{
  char out[6];
  sprintf(out, "%03d", engineTemperature);
  
  tft_l.setCursor(230, engineTempScreenPos);
  tft_l.setTextColor(engineTemperatureColor, ILI9340_BLACK);
  tft_l.setTextSize(5);
  tft_l.print(out);
}

void oilTemperatureReadout()
{
  char out[6];
  sprintf(out, "%03d", oilTemperature);
  
  tft_l.setCursor(230, oilTempScreenPos);
  tft_l.setTextColor(oilTemperatureColor, ILI9340_BLACK);
  tft_l.setTextSize(5);
  tft_l.print(out);
}

void fuelTemperatureReadout()
{
  tft_l.setCursor(230, fuelTempScreenPos);
  tft_l.setTextColor(fuelTemperatureColor, ILI9340_BLACK);
  tft_l.setTextSize(5);
  tft_l.print(fuelTemperature);
}

void batteryVoltageReadout()
{
  tft_l.setCursor(200, batteryVoltScreenPos);
  tft_l.setTextColor(batteryVoltageColor, ILI9340_BLACK);
  tft_l.setTextSize(5);
  tft_l.print(batteryVoltage);
}

//display the brake and throttle bars----------------------------
void pedalPosition(int nThrottle)
{

//  if (nThrottle < lastThrottle) {
//    tft_l.fillRect(20, 80 + (160 - lastThrottle), 100, lastThrottle - nThrottle,  ILI9340_BLACK);
//  }
//  else {
//    tft_l.fillRect(20, 80 + (160 - nThrottle), 100, nThrottle - lastThrottle,  ILI9340_RED);
//  }
  if (nThrottle < lastThrottle)
    tft_l.fillRect(200, 81 + (160 - lastThrottle), 100, lastThrottle - nThrottle,  ILI9340_BLACK);

  else
    tft_l.fillRect(200, 81 + (160 - nThrottle), 100, nThrottle - lastThrottle,  ILI9340_GREEN);

  lastThrottle = nThrottle;
}

//display the scale of throttle and brake bars-------------------
void pedalPositionScale()
{

  tft_l.drawFastVLine(15, 79, 161, ILI9340_WHITE); // Vertical Scale Line
  tft_l.drawFastHLine(0, 79, 15, ILI9340_WHITE); // Major Division
  tft_l.drawFastHLine(5, 119, 10, ILI9340_WHITE); // Minor Division
  tft_l.drawFastHLine(0, 159, 15, ILI9340_WHITE); // Major Division
  tft_l.drawFastHLine(5, 199, 10, ILI9340_WHITE); // Minor Division
  tft_l.drawFastHLine(0, 239, 15, ILI9340_WHITE);  // Major Division

  tft_l.drawFastVLine(305, 79, 161, ILI9340_WHITE); // Vertical Scale Line
  tft_l.drawFastHLine(305, 79, 15, ILI9340_WHITE); // Major Division
  tft_l.drawFastHLine(305, 119, 10, ILI9340_WHITE); // Minor Division
  tft_l.drawFastHLine(305, 159, 15, ILI9340_WHITE); // Major Division
  tft_l.drawFastHLine(305, 199, 10, ILI9340_WHITE); // Minor Division
  tft_l.drawFastHLine(305, 239, 15, ILI9340_WHITE);  // Major Division

}


//clear the screens--------------------------------------
void clearScreens()
{

  tft_r.fillScreen(ILI9340_BLACK);
  tft_l.fillScreen(ILI9340_BLACK);

  // Print "RPM:"
  tft_l.setCursor(1, rpmScreenPos);
  tft_l.setTextColor(rpmColor, ILI9340_BLACK);
  tft_l.setTextSize(5);
  tft_l.print("RPM:");

  // Print "ENG:"
  tft_l.setCursor(1, engineTempScreenPos);
  tft_l.setTextColor(engineTemperatureColor, ILI9340_BLACK);
  tft_l.setTextSize(5);
  tft_l.print("ENG:");

  //Print "OIL:"
  tft_l.setCursor(1, oilTempScreenPos);
  tft_l.setTextColor(oilTemperatureColor, ILI9340_BLACK);
  tft_l.setTextSize(5);
  tft_l.print("OIL:");

  // Print "FUEL:"
  tft_l.setCursor(1, fuelTempScreenPos);
  tft_l.setTextColor(fuelTemperatureColor, ILI9340_BLACK);
  tft_l.setTextSize(5);
  tft_l.print("FUEL:");

  // Print "VOLT:"
  tft_l.setCursor(1, batteryVoltScreenPos);
  tft_l.setTextColor(batteryVoltageColor, ILI9340_BLACK);
  tft_l.setTextSize(5);
  tft_l.print("VOLT:");

}

//display "CAR 16" on the screens------------------------
void startupMessage()
{

  tft_r.fillScreen(ILI9340_BLACK);
  tft_l.fillScreen(ILI9340_BLACK);

  tft_r.setCursor(10, 65);
  tft_r.setTextColor(ILI9340_GREEN, ILI9340_BLACK);
  tft_r.setTextSize(17);
  tft_r.print("CAR");

  tft_l.setCursor(75, 65);
  tft_l.setTextColor(ILI9340_GREEN, ILI9340_BLACK);
  tft_l.setTextSize(17);
  tft_l.print("11");

}
