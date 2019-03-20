
#include <Adafruit_NeoPixel.h>


#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>

#include <SPI.h>

#include <FlexCAN.h>
#include <kinetis_flexcan.h>

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


// left screen
#define TFT2_sclk 13
#define TFT2_mosi 11
#define TFT2_cs 18
#define TFT2_dc 20
#define TFT2_rst 19

Adafruit_ILI9340 tft_l = Adafruit_ILI9340(TFT1_cs, TFT1_dc, TFT1_rst);
Adafruit_ILI9340 tft_r = Adafruit_ILI9340(TFT2_cs, TFT2_dc, TFT2_rst);

//initialize constants-----------------------
int selector = 14;

//initialize screen variables----------------
int GearN = 0;
int RPM = 0;
int Oil_temp = 0;
int Engine_temp = 0;
int Fuel_temp = 0;
int Battery_volt = 0;
int Throttle = 0;
int lastThrottle = 0;

//initialize screen position variables-------
#define pos_1 1
#define pos_2 50
#define pos_3 100
#define pos_4 150
#define pos_5 200
#define rpm_pos pos_1
#define oilTemp_pos pos_3
#define engineTemp_pos pos_2
#define fuelTemp_pos pos_4
#define batteryVolt_pos pos_5

//initialize warnings------------------------
int rpm_color = ILI9340_WHITE;
int oilTemp_color = ILI9340_WHITE;
int engineTemp_color = ILI9340_WHITE;
int fuelTemp_color = ILI9340_WHITE;
int batteryVolt_color = ILI9340_WHITE;
#define oil_prot_1 80
#define oil_prot_2 100
#define engine_prot_1 80
#define engine_prot_2 100
#define fuel_prot_1 0
#define fuel_prot_2 0
#define batt_prot_1 11.5
#define batt_prot_2 10.5

int t = 0;

//initialize timers---------------------------
unsigned long prev_time = 0;
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
void setup() {
  //initialize communications----------------
  //Serial.begin(9600);
  delay(1000);
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


  //clear the screens
  clear_screens();

  delay(1000);

  //display startup message
  startup_message();


  delay(1000);

  clear_screens();
  Pedal_Pos_Scale();


}
//-------------------------------------------------------------------
//
//MAIN
//
//-------------------------------------------------------------------
void loop() {

  CAN_decode();
  RPM_Readout();
  Gear_Readout();
  Pedal_Pos(Throttle);
  //Serial.println(RPM);
  time = millis();
  
  if (time - prev_time >= 1000) {
    if (digitalRead(selector) == LOW) {
      if (s1 == 0) {
        clear_screens();
        s1 = 1;
      }
      {
        prev_time = time;
        Show_Warnings();
        TC_Readout();
        oilTemp_Readout();
        engineTemp_Readout();
        fuelTemp_Readout();
        battVolt_Readout();
      }
    }

    else {
      if (s1 == 1) {
        clear_screens();
        Pedal_Pos_Scale();
        s1 = 0;
      }
      //RPM = map(RPM, 300, 8100, 0, 160);
      
      if (time - prev_time >= 1000) {
        prev_time = time;
        TC_Readout();
        //Show_Warnings_R();
      }
    }
  }
}
//Can Input Conversion-----------------------------------------
void CAN_decode() {
    
  //RPM = analogRead(A1);
    if ( CANbus.read(rxmsg) ) {
      
    uint8_t dumpLen = sizeof(rxmsg);
    int rxByte[dumpLen];
    uint8_t *gua = (uint8_t *)&rxmsg;
    uint8_t working;

    for ( int i = 0 ; i < dumpLen ; i++  ) {
      working = *gua++;
      rxByte[i] = working; //Serial.print(rxByte[i]);
    }

    int rxID = rxByte[1] * 256 + rxByte[0];
    int rxDataLen = rxByte[5];
    int rxData[rxDataLen];
    for (int i = 0; i < rxDataLen; i++) {
      rxData[i] = rxByte[i + 8];
    }

    //****************

    if (rxID == 257) {      //ID 0x101 contains 
      Serial.print("HI");
      RPM = rxData[0] * 255      + rxData[1];
      Throttle  = rxData[2] * 255      + rxData[3];
      Throttle = Throttle / 10;
      Serial.print(Throttle);
      //Brake Pressure Front = rxData[4] * 255      + rxData[5];
      //Brake Pressure Rear  = rxData[6] * 255      + rxData[7];
    }
   
    else if (rxID == 258) {      //ID 0x102 contains
      //TC_SET  = rxData[0] * 255      + rxData[1];
      //TC_SET = (short) ( ((uint8_t) rxData[0] << 8) + ((uint8_t) rxData[1] ) );
      //GearN = (short) ( ((uint8_t) rxData[2] << 8) + ((uint8_t) rxData[3] ) );
      Oil_temp = rxData[4] * 255      + rxData[5];
      Oil_temp = Oil_temp / 1000;
      Engine_temp = rxData[6] * 255      + rxData[7]; 
      Engine_temp = Engine_temp / 1000;
    }
    else if (rxID == 259) {      //ID 0x103 contains
      Battery_volt = rxData[0] * 255      + rxData[1]; 
    }    
    }

}
//change the text color of readouts if there is a warning--------
void Show_Warnings() {

  if (Oil_temp >= oil_prot_1) {
    if (Oil_temp >= oil_prot_2) {
      oilTemp_color = ILI9340_RED;
    }
    else {
      oilTemp_color = ILI9340_YELLOW;
    }
  }
  else {
    oilTemp_color = ILI9340_WHITE;
  }

  if (Engine_temp >= engine_prot_1) {
    if (Engine_temp >= engine_prot_2) {
      engineTemp_color = ILI9340_RED;
    }
    else {
      engineTemp_color = ILI9340_YELLOW;
    }
  }
  else {
    engineTemp_color = ILI9340_WHITE;
  }

  if (Fuel_temp >= fuel_prot_1) {
    if (Fuel_temp >= fuel_prot_2) {
      fuelTemp_color = ILI9340_RED;
    }
    else {
      fuelTemp_color = ILI9340_YELLOW;
    }
  }
  else {
    fuelTemp_color = ILI9340_WHITE;
  }

  if (Battery_volt <= batt_prot_2) {
    if (Battery_volt <= batt_prot_1) {
      batteryVolt_color = ILI9340_RED;
    }
    else {
      batteryVolt_color = ILI9340_YELLOW;
    }
  }
  else {
    batteryVolt_color = ILI9340_WHITE;
  }

}
//display the letter of the warning in race mode-----------------
void Show_Warnings_R() {

  if (Oil_temp >= oil_prot_1) {

  }
  else {
    oilTemp_color = ILI9340_WHITE;
  }

  if (Engine_temp >= engine_prot_1) {

  }
  else {
    engineTemp_color = ILI9340_WHITE;
  }

  if (Fuel_temp >= fuel_prot_1) {

  }
  else {
    fuelTemp_color = ILI9340_WHITE;
  }

  if (Battery_volt <= batt_prot_2) {

  }
  else {
    batteryVolt_color = ILI9340_WHITE;
  }

}
//display information on screens-------------------------

void Gear_Readout() {
  //display gear number
  //char buffer[5];

  //dtostrf(GearN, 1, 0, buffer);

  tft_r.setCursor(140, 15);
  tft_r.setTextColor(ILI9340_WHITE, ILI9340_BLACK);
  tft_r.setTextSize(30);
  tft_r.print(GearN);

}

void TC_Readout() {
  //display tc settings
  tft_r.setCursor(40, 85);
  tft_r.setTextColor(ILI9340_WHITE, ILI9340_BLACK);
  tft_r.setTextSize(10);
  tft_r.print("W");

}

void RPM_Readout() {
  //display the RPM
  //char buffer[6];

  //dtostrf(RPM, 6, 0, buffer);

  tft_l.setCursor(1, rpm_pos);
  tft_l.setTextColor(rpm_color, ILI9340_BLACK);
  tft_l.setTextSize(5);
  tft_l.print("RPM:");
  tft_l.print(RPM);
}

void oilTemp_Readout() {
  //display oil temp
  //char buffer[4];

  //dtostrf(Oil_temp, 6, 0, buffer);

  tft_l.setCursor(1, oilTemp_pos);
  tft_l.setTextColor(oilTemp_color, ILI9340_BLACK);
  tft_l.setTextSize(5);
  tft_l.print("OIL:");
  tft_l.print(Oil_temp);


}

void engineTemp_Readout() {
  //display engine temp
  //char buffer[4];

  //dtostrf(Engine_temp, 3, 0, buffer);

  tft_l.setCursor(1, engineTemp_pos);
  tft_l.setTextColor(engineTemp_color, ILI9340_BLACK);
  tft_l.setTextSize(5);
  tft_l.print("ENGINE:");
  tft_l.print(Engine_temp);

}

void fuelTemp_Readout() {
  //display fuel temp
  //char buffer[4];

  //dtostrf(Fuel_temp, 5, 0, buffer);


  tft_l.setCursor(1, fuelTemp_pos);
  tft_l.setTextColor(fuelTemp_color, ILI9340_BLACK);
  tft_l.setTextSize(5);
  tft_l.print("FUEL:");
  tft_l.print(Fuel_temp);

}

void battVolt_Readout() {
  //display battery voltage
  //char buffer[5];

  //dtostrf(Battery_volt, 5, 1, buffer);


  tft_l.setCursor(1, batteryVolt_pos);
  tft_l.setTextColor(batteryVolt_color, ILI9340_BLACK);
  tft_l.setTextSize(5);
  tft_l.print("VOLT:");
  tft_l.print(Battery_volt);

}

//display the brake and throttle bars----------------------------
void Pedal_Pos(int nThrottle) {

//  if (nThrottle < lastThrottle) {
//    tft_l.fillRect(20, 80 + (160 - lastThrottle), 100, lastThrottle - nThrottle,  ILI9340_BLACK);
//  }
//  else {
//    tft_l.fillRect(20, 80 + (160 - nThrottle), 100, nThrottle - lastThrottle,  ILI9340_RED);
//  }
  if (nThrottle < lastThrottle) {
    tft_l.fillRect(200, 81 + (160 - lastThrottle), 100, lastThrottle - nThrottle,  ILI9340_BLACK);
  }
  else {
    tft_l.fillRect(200, 81 + (160 - nThrottle), 100, nThrottle - lastThrottle,  ILI9340_GREEN);
  }
  lastThrottle = nThrottle;

}

//display the scale of throttle and brake bars-------------------
void Pedal_Pos_Scale() {

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
void clear_screens() {

  tft_r.fillScreen(ILI9340_BLACK);
  tft_l.fillScreen(ILI9340_BLACK);

}

//display "CAR 16" on the screens------------------------
void startup_message() {

  tft_r.setCursor(10, 65);
  tft_r.setTextColor(ILI9340_GREEN, ILI9340_BLACK);
  tft_r.setTextSize(17);
  tft_r.print("CAR");

  tft_l.setCursor(75, 65);
  tft_l.setTextColor(ILI9340_GREEN, ILI9340_BLACK);
  tft_l.setTextSize(17);
  tft_l.print("16");

}
