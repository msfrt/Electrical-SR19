#include <Adafruit_NeoPixel.h>

// neopixels have their own digital pins
#define PIN_TOP 16 //6,7,8
#define PIN_L 15
#define PIN_R 17

#define NUMPIXELS_TOP 12
#define NUMPIXELS_L 4
#define NUMPIXELS_R 4

Adafruit_NeoPixel pixels_t = Adafruit_NeoPixel(NUMPIXELS_TOP, PIN_TOP, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_l = Adafruit_NeoPixel(NUMPIXELS_L, PIN_L, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_r = Adafruit_NeoPixel(NUMPIXELS_R, PIN_R, NEO_GRB + NEO_KHZ800);

#include <FlexCAN.h>
#include <kinetis_flexcan.h>
FlexCAN CANbus(1000000);
static CAN_message_t rxmsg;
static CAN_message_t msg;

//initialize constants-----------------------
const int led_brightness = 255;
const int led_brightness_flash = 150;

//initialize rpm bar constants--------------

int RPM_Max = 13500;
int RPM_Min = 1400;
int shiftPoint = 12000;

//initialize screen variables----------------
int GearN = 0;
int RPM = 0;
int Oil_temp = 0;
int Engine_temp = 0;
int Fuel_temp = 0;
int Battery_volt = 0;

//initialize timer---------------------------
unsigned long prev_time = 0;
unsigned long time;


//---------------------------------------------------------------------------------------------------
//
//SETUP
//
//---------------------------------------------------------------------------------------------------
void setup() {
  //initialize communications----------------
  Serial.begin(9600);
  CANbus.begin();
  pinMode(13, OUTPUT);
  digitalWrite(13, 1);
  delay(1000);
  digitalWrite(13, 0);
  pixels_t.setBrightness(led_brightness);



  //initialize leds------------------------
  pixels_t.begin(); // This initializes the NeoPixel library.
  pixels_l.begin();
  pixels_r.begin();

  pixels_t.show();
  pixels_l.show();
  pixels_r.show();

  delay(1500);

  //flash the LEDs on to test
  led_initialize();

}
//-------------------------------------------------------------------
//
//MAIN
//
//-------------------------------------------------------------------
void loop() {

  CAN_decode();

  for (int i = RPM_Min; i <= shiftPoint; i++)
  {
    RPM = i;
    Serial.print("RPM: ");
    Serial.println(RPM);
    delay(1);

    RPM_Bar();

    if (RPM >= shiftPoint)
      RPM_Bar_Flash();
  }

//  //Show_Warnings();
//  time = millis();
//  RPM_Bar();
//  //Left_Bar();
//  //Right_Bar();
//
//
//  if (RPM >= shiftPoint) {
//    //if (time - prev_time >= 1000) {
//    // prev_time = time;
//    RPM_Bar_Flash();
//  }
}

//}

void CAN_decode() {

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

    if (rxID == 257) {      //ID 0x101 contains RPM, Throttle Pos
      RPM = rxData[0] * 255      + rxData[1];
      //Throttle  = rxData[2] * 255      + rxData[3];
      // Throttle = Throttle / 10;
      //TC  = rxData[4] * 255      + rxData[5];
      //BP  = rxData[6] * 255      + rxData[7];
    }

  }
  //Serial.println(RPM);
}

void RPM_Bar() {

  int led_value = map(RPM, RPM_Min, RPM_Max, 0, 1800);

  if (led_value >= 0) {
    if (led_value >= 150) {
      pixels_t.setPixelColor(0, pixels_t.Color(0, 150, 0));
    }
    else {
      pixels_t.setPixelColor(0, pixels_t.Color(0, led_value, 0));
    }

  }
  else {
    pixels_t.setPixelColor(0, pixels_t.Color(0, 0, 0));
  }
  //----------------------------------------------------------------------------
  if (led_value >= 150) {
    if (led_value >= 300) {
      pixels_t.setPixelColor(1, pixels_t.Color(0, 150, 0));
    }
    else {
      pixels_t.setPixelColor(1, pixels_t.Color(0, led_value - 150 , 0));
    }

  }
  else {
    pixels_t.setPixelColor(1, pixels_t.Color(0, 0, 0));
  }
  //----------------------------------------------------------------------------
  if (led_value >= 300) {
    if (led_value >= 450) {
      pixels_t.setPixelColor(2, pixels_t.Color(0, 150, 0));
    }
    else {
      pixels_t.setPixelColor(2, pixels_t.Color(0, led_value - 300 , 0));
    }

  }
  else {
    pixels_t.setPixelColor(2, pixels_t.Color(0, 0, 0));
  }
  //----------------------------------------------------------------------------
  if (led_value >= 450) {
    if (led_value >= 600) {
      pixels_t.setPixelColor(3, pixels_t.Color(0, 150, 0));
    }
    else {
      pixels_t.setPixelColor(3, pixels_t.Color(0, led_value - 450 , 0));
    }

  }
  else {
    pixels_t.setPixelColor(3, pixels_t.Color(0, 0, 0));
  }
  //----------------------------------------------------------------------------
  if (led_value >= 600) {
    if (led_value >= 750) {
      pixels_t.setPixelColor(4, pixels_t.Color(0, 150, 0));
    }
    else {
      pixels_t.setPixelColor(4, pixels_t.Color(0, led_value - 600 , 0));
    }

  }
  else {
    pixels_t.setPixelColor(4, pixels_t.Color(0, 0, 0));
  }
  //----------------------------------------------------------------------------
  if (led_value >= 750) {
    if (led_value >= 900) {
      pixels_t.setPixelColor(5, pixels_t.Color(150, 150, 0));
    }
    else {
      pixels_t.setPixelColor(5, pixels_t.Color(led_value - 750, led_value - 750 , 0));
    }

  }
  else {
    pixels_t.setPixelColor(5, pixels_t.Color(0, 0, 0));
  }
  //----------------------------------------------------------------------------
  if (led_value >= 900) {
    if (led_value >= 1050) {
      pixels_t.setPixelColor(6, pixels_t.Color(150, 150, 0));
    }
    else {
      pixels_t.setPixelColor(6, pixels_t.Color(led_value - 900, led_value - 900 , 0));
    }

  }
  else {
    pixels_t.setPixelColor(6, pixels_t.Color(0, 0, 0));
  }
  //----------------------------------------------------------------------------
  if (led_value >= 1050) {
    if (led_value >= 1200) {
      pixels_t.setPixelColor(7, pixels_t.Color(150, 150, 0));
    }
    else {
      pixels_t.setPixelColor(7, pixels_t.Color(led_value - 1050, led_value - 1050 , 0));
    }

  }
  else {
    pixels_t.setPixelColor(7, pixels_t.Color(0, 0, 0));
  }
  //----------------------------------------------------------------------------
  if (led_value >= 1200) {
    if (led_value >= 1350) {
      pixels_t.setPixelColor(8, pixels_t.Color(150, 150, 0));
    }
    else {
      pixels_t.setPixelColor(8, pixels_t.Color(led_value - 1200, led_value - 1200, 0));
    }

  }
  else {
    pixels_t.setPixelColor(8, pixels_t.Color(0, 0, 0));
  }
  //----------------------------------------------------------------------------
  if (led_value >= 1350) {
    if (led_value >= 1500) {
      pixels_t.setPixelColor(9, pixels_t.Color(150, 0, 0));
    }
    else {
      pixels_t.setPixelColor(9, pixels_t.Color(led_value - 1350, 0, 0));
    }

  }
  else {
    pixels_t.setPixelColor(9, pixels_t.Color(0, 0, 0));
  }
  //----------------------------------------------------------------------------
  if (led_value >= 1500) {
    if (led_value >= 1650) {
      pixels_t.setPixelColor(10, pixels_t.Color(150, 0, 0));
    }
    else {
      pixels_t.setPixelColor(10, pixels_t.Color(led_value - 1500, 0, 0));
    }

  }
  else {
    pixels_t.setPixelColor(10, pixels_t.Color(0, 0, 0));
  }
  //----------------------------------------------------------------------------
  if (led_value >= 1650) {
    if (led_value >= 1800) {
      pixels_t.setPixelColor(11, pixels_t.Color(150, 0, 0));
    }
    else {
      pixels_t.setPixelColor(11, pixels_t.Color(led_value - 1650, 0, 0));
    }

  }
  else {
    pixels_t.setPixelColor(11, pixels_t.Color(0, 0, 0));
  }

  pixels_t.show();
}
void Show_Warnings() {


}

void RPM_Bar_Flash() {

  //for (int i = 0; i < 2; i++) {
    for (int i = 0; i < NUMPIXELS_TOP; i++) {
      pixels_t.setPixelColor(i, pixels_t.Color(led_brightness_flash, 0, 0));
      pixels_t.setBrightness(led_brightness_flash);
      pixels_t.show();
    }
    delay(10);

    for (int i = 0; i < NUMPIXELS_TOP; i++) {
      pixels_t.setPixelColor(i, pixels_t.Color(0, 0, 0));
      pixels_t.setBrightness(led_brightness);
      pixels_t.show();
    }
    delay(10);
  //}
}

//flash the leds on startup------------------------------
void led_initialize() {
  int led = 0;
  int ledt = NUMPIXELS_TOP;
  int ledt2 = 6;

  for (int i = 0; i < 10; i++) {

    pixels_t.setPixelColor(led - 4, pixels_t.Color(0, 30, 0));
    pixels_t.show();
    if (led >= 3) {
      pixels_t.setPixelColor(ledt, pixels_t.Color(0, 30, 0));
      pixels_t.show();
      ledt--;
    }
    pixels_l.setPixelColor(led, pixels_l.Color(0, 30, 0));
    pixels_l.show();
    pixels_r.setPixelColor(led, pixels_r.Color(0, 30, 0));
    pixels_r.show();
    led++;
    delay(80);
  }

  for (int i = 11; i > 0; i--) {

    pixels_t.setPixelColor(led - 4, pixels_t.Color(0, 0, 0));
    pixels_t.show();
    if (led >= 3) {
      pixels_t.setPixelColor(ledt2, pixels_t.Color(0, 0, 0));
      pixels_t.show();
      ledt2++;
    }
    pixels_l.setPixelColor(led, pixels_l.Color(0, 0, 0));
    pixels_l.show();
    pixels_r.setPixelColor(led, pixels_r.Color(0, 0, 0));
    pixels_r.show();
    led--;
    delay(80);
  }

}
