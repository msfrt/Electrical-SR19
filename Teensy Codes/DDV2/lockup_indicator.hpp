#ifndef WHEELS_LOCKED
#define WHEELS_LOCKED

#include <StateCAN.h>
#include "sigs_inside.hpp"


// determines if the wheel is in lockup. Turns the led_index purple if that is the case. Otherwise,
// the LED is set to be off. DOES NOT SHOW THE LED. That must be called externally once all lockups have been calculated
bool lockup_indicator(Adafruit_NeoPixel &lightbar, int led_index,
                 StateSignal &wheel_speed, StateSignal &Ax, StateSignal &bp_front, StateSignal &bp_rear){

  static EasyTimer lockup_led_flash_timer(25);
  static bool leds_on;

  // if the wheel is locked up
  if ((wheel_speed.value() < 1) && ((bp_front.value() > 100) || (bp_rear.value() > 100)) && (Ax.value() < -0.5)){

    // do the flashing thing
    if (lockup_led_flash_timer.isup()){
      if (leds_on){
        leds_on = false;
      } else {
        leds_on = true;
      }
    }

    // this toggles the LED on or off
    if (leds_on){
      lightbar.setPixelColor(led_index, 100, 0, 150);
    } else {
      lightbar.setPixelColor(led_index, 0, 0, 0);
    }

    return true;

  // wheel is not locked up
  } else {

    lightbar.setPixelColor(led_index, 0, 0, 0);
    return false;
  }

}


#endif
