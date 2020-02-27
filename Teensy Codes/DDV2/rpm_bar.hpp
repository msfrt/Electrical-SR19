#include <StateCAN.h>
#include <Adafruit_NeoPixel.h>
#include <EasyTimer.h>

const int downshift12_rpm = 9362;
const int downshift23_rpm = 9703;
const int downshift34_rpm = 9783;
const int downshift45_rpm = 9855;
const int upshift12_rpm = 13224;
const int upshift23_rpm = 11900;
const int upshift34_rpm = 12000;
const int upshift45_rpm = 12000;

const int rpm_bar_min_bound = 5000;
const int rpm_bar_max_bound = 14000;

// returns a value 0-255 for the certain LED and subtracts the PWM output from the total
int led_pwm(int &total_pwm){
  static int temp_store;

  // LED should be completely on
  if (total_pwm >= 255){
    total_pwm -= 255;
    return 255;

  // LED should be completely off
  } else if (total_pwm <= 0){
    total_pwm = 0;
    return 0;

  // LED should be dimmed
  } else {
    temp_store = total_pwm;
    total_pwm = 0;
    return temp_store;
  }
}

// sets the rpm bar to the desired output. Returns a boolean true if the driver should shift
bool rpm_bar(Adafruit_NeoPixel &top, StateSignal &rpm, StateSignal &gear){

  static const int idle_rpm = 2000;
  static const int idle_rev_limit_rpm = 8000;
  static const int rev_limit_rpm = 14000;

  static const int num_yellow_leds = 6;
  static const int num_red_leds = 5;

  static int bar_pwms;
  static int bar_mode;
  static EasyTimer rpm_flash_timer(20);
  static bool rpm_flash_on;
  static const int max_bar_pwm_posns = top.numPixels() * 256;
  static int pwm_current_led;

  // determine if downshift mode
  if ((rpm.value() <= downshift12_rpm && (static_cast<int>(gear.value() == 2))) || // currently gear 2
      (rpm.value() <= downshift23_rpm && (static_cast<int>(gear.value() == 3))) || // 3
      (rpm.value() <= downshift34_rpm && (static_cast<int>(gear.value() == 4))) || // 4
      (rpm.value() <= downshift45_rpm && (static_cast<int>(gear.value() == 5)))){  // 5
    bar_mode = 1;

  // determine if upshift mode
  } else if ((rpm.value() >= upshift12_rpm && (static_cast<int>(gear.value() == 1))) ||
             (rpm.value() >= upshift23_rpm && (static_cast<int>(gear.value() == 2))) ||
             (rpm.value() >= upshift34_rpm && (static_cast<int>(gear.value() == 3))) ||
             (rpm.value() >= upshift45_rpm && (static_cast<int>(gear.value() == 4)))){
    bar_mode = 2;

  // otherwise we are in rpmbar mode
  } else {
    bar_mode = 0;
  }

  // downshift
  if (bar_mode == 1){
    // set half to be aqua
    for (int i = 0; i <= top.numPixels() / 2; ++i){
      top.setPixelColor(i, 0, 255 / (i + 1), 255 / (i + 1));
    }
    for (int i = top.numPixels() / 2; i <= top.numPixels(); ++i){
      top.setPixelColor(i, 0, 0, 0);
    }

  // upshift
  } else if (bar_mode == 2){
    if (rpm_flash_timer.isup()){


      // lights were on, turn them off
      if (rpm_flash_on){
        rpm_flash_on = false;
        // set the LEDs off
        for (int i = 0; i <= top.numPixels(); ++i){
          top.setPixelColor(i, 0, 0, 0);
        }
      // lights were off, turn them on
      } else {
        rpm_flash_on = true;
        for (int i = 0; i <= top.numPixels(); ++i){
          top.setPixelColor(i, 255, 0, 0);
        }
      }
    }


  // else, show rpm bar mode
  } else {

    // calculate the pwms to be set
    if (static_cast<int>(gear.value()) < 1){ // neutral
      bar_pwms = map(rpm.value(), idle_rpm, idle_rev_limit_rpm, 0, max_bar_pwm_posns);
    } else if (static_cast<int>(gear.value()) == 1){ // gear 1
      bar_pwms = map(rpm.value(), idle_rpm, upshift12_rpm, 0, max_bar_pwm_posns);
    } else if (static_cast<int>(gear.value()) == 2){ // gear 2
      bar_pwms = map(rpm.value(), downshift12_rpm, upshift23_rpm, 0, max_bar_pwm_posns);
    } else if (static_cast<int>(gear.value()) == 3){ // gear 3
      bar_pwms = map(rpm.value(), downshift23_rpm, upshift34_rpm, 0, max_bar_pwm_posns);
    } else if (static_cast<int>(gear.value()) == 4){ // gear 4
      bar_pwms = map(rpm.value(), downshift34_rpm, upshift45_rpm, 0, max_bar_pwm_posns);
    } else { // gear 5
      bar_pwms = map(rpm.value(), downshift45_rpm, rev_limit_rpm, 0, max_bar_pwm_posns);
    }


    // set the LEDs to be on
    for (int i = 0; i <= top.numPixels(); i++){
      pwm_current_led = led_pwm(bar_pwms); // get the PWM for the current LED

      // turn the current LED green
      if (i < (top.numPixels() - (num_yellow_leds + num_red_leds))){
        top.setPixelColor(i, 0, pwm_current_led, 0);

      // turn the current LED yellow
      } else if (i < (top.numPixels() - num_red_leds)){
        top.setPixelColor(i, (2 * pwm_current_led) / 3, (2 * pwm_current_led) / 3, 0); // 2/3 brightness for each color

      // turn the rest red
      } else {
        top.setPixelColor(i, pwm_current_led, 0, 0);
      }
    }

  }

  // if the driver should be shifting
  if (bar_mode == 2){
    return true;
  } else {
    return false;
  }
}


// copy of function above, but this has the OG rpmbar gradient, which looks cool :)
bool rpm_bar_gradient(Adafruit_NeoPixel &top, StateSignal &rpm, StateSignal &gear){

  static const int idle_rpm = 2000;
  static const int idle_rev_limit_rpm = 8000;
  static const int rev_limit_rpm = 14000;

  static int bar_pwms;
  static int bar_mode;
  static EasyTimer rpm_flash_timer(20);
  static bool rpm_flash_on;
  static const int max_bar_pwm_posns = top.numPixels() * 256;
  static int pwm_current_led;

  // determine if downshift mode
  if ((rpm.value() <= downshift12_rpm && (static_cast<int>(gear.value() == 2))) || // currently gear 2
      (rpm.value() <= downshift23_rpm && (static_cast<int>(gear.value() == 3))) || // 3
      (rpm.value() <= downshift34_rpm && (static_cast<int>(gear.value() == 4))) || // 4
      (rpm.value() <= downshift45_rpm && (static_cast<int>(gear.value() == 5)))){  // 5
    bar_mode = 1;

  // determine if upshift mode
  } else if ((rpm.value() >= upshift12_rpm && (static_cast<int>(gear.value() == 1))) ||
             (rpm.value() >= upshift23_rpm && (static_cast<int>(gear.value() == 2))) ||
             (rpm.value() >= upshift34_rpm && (static_cast<int>(gear.value() == 3))) ||
             (rpm.value() >= upshift45_rpm && (static_cast<int>(gear.value() == 4)))){
    bar_mode = 2;

  // otherwise we are in rpmbar mode
  } else {
    bar_mode = 0;
  }

  // downshift
  if (bar_mode == 1){
    // set half to be aqua
    for (int i = 0; i <= top.numPixels() / 2; ++i){
      top.setPixelColor(i, 0, 255 / (i + 1), 255 / (i + 1));
    }
    for (int i = top.numPixels() / 2; i <= top.numPixels(); ++i){
      top.setPixelColor(i, 0, 0, 0);
    }

  // upshift
  } else if (bar_mode == 2){
    if (rpm_flash_timer.isup()){


      // lights were on, turn them off
      if (rpm_flash_on){
        rpm_flash_on = false;
        // set the LEDs off
        for (int i = 0; i <= top.numPixels(); ++i){
          top.setPixelColor(i, 0, 0, 0);
        }
      // lights were off, turn them on
      } else {
        rpm_flash_on = true;
        for (int i = 0; i <= top.numPixels(); ++i){
          top.setPixelColor(i, 255, 0, 0);
        }
      }
    }


  // else, show rpm bar mode
  } else {

    // calculate the pwms to be set
    if (static_cast<int>(gear.value()) < 1){ // neutral
      bar_pwms = map(rpm.value(), idle_rpm, idle_rev_limit_rpm, 0, max_bar_pwm_posns);
    } else if (static_cast<int>(gear.value()) == 1){ // gear 1
      bar_pwms = map(rpm.value(), idle_rpm, upshift12_rpm, 0, max_bar_pwm_posns);
    } else if (static_cast<int>(gear.value()) == 2){ // gear 2
      bar_pwms = map(rpm.value(), downshift12_rpm, upshift23_rpm, 0, max_bar_pwm_posns);
    } else if (static_cast<int>(gear.value()) == 3){ // gear 3
      bar_pwms = map(rpm.value(), downshift23_rpm, upshift34_rpm, 0, max_bar_pwm_posns);
    } else if (static_cast<int>(gear.value()) == 4){ // gear 4
      bar_pwms = map(rpm.value(), downshift34_rpm, upshift45_rpm, 0, max_bar_pwm_posns);
    } else { // gear 5
      bar_pwms = map(rpm.value(), downshift45_rpm, rev_limit_rpm, 0, max_bar_pwm_posns);
    }

    // turn them all off so we can set the ones that we want to be on
    for (int i = 0; i <= top.numPixels(); i++){
      top.setPixelColor(i, 0, 0, 0);
    }

    // set the LEDs to be on
    for (int i = 0; i <= top.numPixels(); i++){
      pwm_current_led = led_pwm(bar_pwms);
      // the divisors take care of the gradients
      top.setPixelColor(i, pwm_current_led / (top.numPixels() - i), pwm_current_led / (i + 1), 0);
    }

  }

    // if the driver should be shifting
    if (bar_mode == 2){
      return true;
    } else {
      return false;
    }
  }



// lights up the light bar during ignition cut events. The motec ignition cut value is an integer 0-256
bool engine_cut_bar(Adafruit_NeoPixel &leds, StateSignal &tc_sig){
  static const int num_leds = 3; // currently, there are 3 LEDs on the light strip that are designated cut level lights
  static const int max_bar_pwm_posns = num_leds * 256;

  // variables used in every valculation. Static because there's no need to create and and delete them every time.
  static int bar_pwms;
  static int current_led_pwm;
  static bool leds_on = false;

  // calculations begin below ---------------

  // the total number of pwms to be written this iteration
  bar_pwms = map(tc_sig.value(), tc_sig.lower_bound(), tc_sig.upper_bound(), 0, max_bar_pwm_posns);

  if (bar_pwms > 0){
    leds_on = true;
  } else {
    leds_on = false;
  }

  // turn them on!
  for (int i = leds.numPixels() - 1; i >= leds.numPixels() - num_leds; i--){
    current_led_pwm = led_pwm(bar_pwms);
    leds.setPixelColor(i, 0, 0, current_led_pwm);
  }

  return leds_on;
}
