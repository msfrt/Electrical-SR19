// Forked from SR-20 driver display code on February 26th, 2020. Reduced functionality.

// Dave Yonkers, 2020

#include <Adafruit_NeoPixel.h>
#include <StateCAN.h>
#include <FlexCAN_T4.h>
#include <EasyTimer.h>
#include <BoardTemp.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "ILI9341_t3n.h"
#define SPI0_DISP1

#define READ_RESOLUTION_BITS 12

// can bus decleration
FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> cbus1;
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> cbus2;

//BoardTemp(int pin, int read_bits, int temp_cal, int mv_cal);
BoardTempDiode board_temp(21, READ_RESOLUTION_BITS, 26.2, 598);
EasyTimer board_temp_sample_timer(100);


// fonts :)
#include "font_LiberationMonoBold.h"
#include "font_LiberationMonoBoldItalic.h"

// photos :) - converted with http://www.rinkydinkelectronics.com/t_imageconverter565.php
#include "lana1.c"
#include "lana2.c"
#include "fuck_kyle_busch.c"

// NeoPixel parameters
const int pixels_top_pin = 16; // teensy pin #
const int pixels_left_pin= 15;
const int pixels_right_pin = 17;
const int pixels_top_cnt = 12; // number of LEDs
const int pixels_left_cnt = 4;
const int pixels_right_cnt = 4;
      int pixel_brightness_percent = 5; // 0 - 100; 100 is blinding... 4 is the minimum for all LED bar colors to work

Adafruit_NeoPixel pixels_top =   Adafruit_NeoPixel(pixels_top_cnt,   pixels_top_pin,   NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_left =  Adafruit_NeoPixel(pixels_left_cnt,  pixels_left_pin,  NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_right = Adafruit_NeoPixel(pixels_right_cnt, pixels_right_pin, NEO_GRB + NEO_KHZ800);

// TFT display paramters
#define TFTL_DC 7
#define TFTL_CS 5
#define TFTL_MOSI 11
#define TFTL_MISO 12
#define TFTL_CLK 13
#define TFTL_RST 6
#define TFTL_BL 1
    int display_left_brightness_percent = 100;


#define TFTR_DC 20
#define TFTR_CS 18
#define TFTR_MOSI 11
#define TFTR_MISO 12
#define TFTR_CLK 13
#define TFTR_RST 19
#define TFTR_BL 1
    int display_right_brightness_percent = 100;

const int DISPLAY_HEIGHT = 240;
const int DISPLAY_WIDTH = 320;

// ILI9341_t3(uint8_t _CS, uint8_t _DC, uint8_t _RST = 255, uint8_t _MOSI=11, uint8_t _SCLK=13, uint8_t _MISO=12);
ILI9341_t3n display_left = ILI9341_t3n(TFTL_CS, TFTL_DC, TFTL_RST, TFTL_MOSI, TFTL_CLK, TFTL_MISO);
ILI9341_t3n display_right = ILI9341_t3n(TFTR_CS, TFTR_DC, TFTR_RST, TFTR_MOSI, TFTR_CLK, TFTR_MISO);


// modes for the screen and leds
int led_mode = 1;
int screen_mode = 1;

// include externally-written functions
#include "led_startup.hpp"
#include "rpm_bar.hpp"
#include "party_bar.hpp"
#include "warning_lights.hpp"
#include "lockup_indicator.hpp"

// signal definitions
#include "sigs_inside.hpp" // eventually move signal and can message definitions to shared folder, then link using the full file path

// CAN message definitions
#include "can_read.hpp"
//#include "can_send.hpp"

// bitmaps - generated here: http://javl.github.io/image2cpp/
#include "sr_bitmap.hpp"
#include "big_numbers.hpp"

// info screen struct and functions
#include "info_screen.hpp"
// big number display struct and functions
#include "big_number_display.hpp"

// includes warning messages display
#include "user_message_display.hpp"

// lap timer screen
#include "lap_timer.hpp"


char rpm_form[] = "%04.1f";
char oilp_form[] = "%03.0f";
char oilt_form[] = "%03.0f";
char engt_form[] = "%04.1f";
InfoScreen engine_vitals_right_screen(display_right, M400_rpm, M400_oilPressure, M400_oilTemp, M400_engineTemp,
                                        /* label */  "RPM:",   "OILP:",          "OILT:",      "ENG:",
                            /* string formatting */  rpm_form, oilp_form ,       oilt_form,    engt_form );

char speed_form[] = "%04.1f";
char battv_form[] = "%04.1f";
char bias_form[] = "%02.0f%%"; // the extra two %'s are not a typo!
char fanl_form[] = "%03.0f";
InfoScreen auxilary_info_left_screen(display_left, M400_groundSpeed, PDM_pdmVoltAvg, ATCCF_brakeBias, PDM_fanLeftPWM,
                                      /* label */  "SPD:",           "BAT:",         "BIAS:",         "FANS:",
                          /* string formatting */  speed_form,       battv_form,     bias_form,       fanl_form);


char warning_msg[9] = "LOW OIL";
UserMessageDisplay warning_message_display(display_right, warning_msg, "MESSAGE:", ILI9341_WHITE);

EasyTimer info_screen_update_timer(10); // rate at which the screens will check their variables for updates

EasyTimer debug(50); // debugging timer

// used for dynamically changing clock speed :-)))
// #if defined(__IMXRT1062__)
// extern "C" uint32_t set_arm_clock(uint32_t frequency);
// #endif


void setup() {

  // dynamically change clock speed
  // #if defined(__IMXRT1062__)
  //   set_arm_clock(45000000);
  //   Serial.print("F_CPU_ACTUAL=");
  //   Serial.println(F_CPU_ACTUAL);
  // #endif

  // set analog read resolution
  analogReadResolution(READ_RESOLUTION_BITS);

  // initilize CAN busses
  cbus1.begin();
  cbus1.setBaudRate(1000000);
  cbus2.begin();
  cbus2.setBaudRate(1000000);

  // initialze serial coms
  Serial.begin(115200);

  // init top pixels
  pixels_top.setBrightness(map(pixel_brightness_percent, 0, 100, 0, 255));
  pixels_top.begin();
  pixels_top.show();
  // init right pixels
  pixels_right.setBrightness(map(pixel_brightness_percent, 0, 100, 0, 255));
  pixels_right.begin();
  pixels_right.show();
  // init left pixels
  pixels_left.setBrightness(map(pixel_brightness_percent, 0, 100, 0, 255));
  pixels_left.begin();
  pixels_left.show();

  // initialize screens
  display_left.begin();
  display_right.begin();
  display_left.setRotation(3);
  display_right.setRotation(1);
  // draw SR logo
  display_left.fillScreen(ILI9341_BLACK);
  display_right.fillScreen(ILI9341_BLACK);
  analogWrite(TFTL_BL, map(display_left_brightness_percent, 0, 100, 0, 255));
  analogWrite(TFTR_BL, map(display_right_brightness_percent, 0, 100, 0, 255));
  // draw the OG state racing logo in white
  display_left.drawBitmap(0, 0, state_racing_bitmap, DISPLAY_WIDTH, DISPLAY_HEIGHT, ILI9341_WHITE);
  display_right.drawBitmap(0, 0, state_racing_bitmap, DISPLAY_WIDTH, DISPLAY_HEIGHT, ILI9341_WHITE);


  // fun LED startup sequence. Last parameter is time multiplier. 0 is fastest, 5 is pretty darn slow.
  // if you set it higher than 5, I have respect for your patience
  led_startup(pixels_top, pixels_left, pixels_right, 1);


  auxilary_info_left_screen.begin();
   // scale rpm down by 1000
  engine_vitals_right_screen.begin();

}




void loop() {

  // read CAN buses
  read_can1();
  read_can2();


  obd_oil_pressure_acceptence(M400_oilPressure, M400_rpm);

  // LED updates -------------------------------------------------------

  if (led_mode == 1){
    rpm_bar(pixels_top, M400_rpm, M400_gear);
    engine_cut_bar(pixels_left,  M400_tcPowerReduction);
    engine_cut_bar(pixels_right, M400_tcPowerReduction);

    // this will prolly be changed
    //lockup_indicator(pixels_left, 0, M400_groundSpeedLeft, MM5_Ax, ATCCF_brakePressureF, ATCCF_brakePressureR);
    //lockup_indicator(pixels_left, 3, M400_driveSpeedLeft, MM5_Ax, ATCCF_brakePressureF, ATCCF_brakePressureR);
    //lockup_indicator(pixels_right, 0, M400_groundSpeedRight, MM5_Ax, ATCCF_brakePressureF, ATCCF_brakePressureR);
    //lockup_indicator(pixels_right, 3, M400_driveSpeedRight, MM5_Ax, ATCCF_brakePressureF, ATCCF_brakePressureR);

    // these two are unused
    pixels_left.setPixelColor(0, 0, 0, 0);
    pixels_right.setPixelColor(0, 0, 0, 0);

    pixels_top.show();
    pixels_left.show();
    pixels_right.show();

  // tell the driver to come in
  } else if (led_mode == 10){
    full_warning_lights(pixels_top, pixels_left, pixels_right, "WHITE");

  // tell the driver to stop
  } else if (led_mode == 11){
    full_warning_lights(pixels_top, pixels_left, pixels_right, "YELLOW");

  // tell the driver to stop and shut off the car
  } else if (led_mode == 12){
    full_warning_lights(pixels_top, pixels_left, pixels_right, "RED");

  }



  // display updates --------------------------------------------------

  if (info_screen_update_timer.isup()){
    static bool warning_on = false;

    // display the warning, otherwise display the info screen
    if (warning_message_display.show()){
        warning_on = true;

    // update the screens or begin them if necessary
    } else {
      if (warning_on){
        warning_on = false;
        auxilary_info_left_screen.begin();
        engine_vitals_right_screen.begin();

      } else {
        auxilary_info_left_screen.update();
        engine_vitals_right_screen.update();
      }
    }
  }

  // send can messages
  //send_can1();
  //send_can2();

}



// relocated function that's called when the screen mode changes. This runs the required initializations for
// every screen mode, except for lap trigger, which is timed by itself.
void screen_mode_begins(int &screen_mode, bool startup_screen){
  // auxilarry info screen and engine vitals screen
  if (screen_mode == 1){
    auxilary_info_left_screen.begin();
    engine_vitals_right_screen.begin();
  }
}





int OBDFLAG_oil_pressure = 0;

// Oil pressure parameters
const int OBDPARAM_oil_pressure_min_rpm = 2500;   // only check for oil issues above this rpm
const int OBDPARAM_oil_pressure_dip_time_ms = 5000;   // time allowed below minimum pressure before raising a flag
const int OBDPARAM_oil_pressure_min_percent_allowed = 70;  // percent of predicted oil pressure before it is determined bad
EasyTimer OBDTIMER_oil_pressure_check_timer(10);


bool obd_oil_pressure_acceptence(StateSignal &oil_pressure, StateSignal &rpm){
  static unsigned long good_until_time = 5000;
  static bool oil_pressure_good = true;
  static float predicted_pressure = 0.0;

  // check if the sensors are valid in the first place.
  if (!oil_pressure.is_valid() || !rpm.is_valid()){
    OBDFLAG_oil_pressure = 0;
    return true;

  // if the rpm is not above the minimum, we assume the oil pressure is good
  } else if (M400_rpm.value() < OBDPARAM_oil_pressure_min_rpm){
    OBDFLAG_oil_pressure = 0;
    return true;
  }


  // calcluate the predicted pressure
  // Function generated: 02/15/2020 10:28:53
  predicted_pressure = 20.068 * log(rpm.value() - 1776.948) - 116.958;

  // current oil pressure is acceptable
  if ((oil_pressure.value() * 100) / predicted_pressure >= OBDPARAM_oil_pressure_min_percent_allowed){
    oil_pressure_good = true;
    OBDFLAG_oil_pressure = 1;
    good_until_time = millis() + OBDPARAM_oil_pressure_dip_time_ms;


  // current oil pressure is unacceptable
  } else {
    oil_pressure_good = false;


    // raise a flag
    if (millis() > good_until_time && OBDFLAG_oil_pressure == 0){
      OBDFLAG_oil_pressure = 1;
      warning_message_display.begin();

    }
  }

  return oil_pressure_good;
}
