#ifndef WARNING_LIGHTS_FLASHER_HPP
#define WARNING_LIGHTS_FLASHER_HPP

void full_warning_lights(Adafruit_NeoPixel &top, Adafruit_NeoPixel &left,
                    Adafruit_NeoPixel &right, String color, bool override = false){
  static EasyTimer flash_timer(3);
  static bool leds_on(false);
  static uint8_t R = 255;
  static uint8_t G = 255;
  static uint8_t B = 255;

  if (flash_timer.isup()){

    // turn them off
    if (leds_on && !override){
      leds_on = false;
      left.clear();
      top.clear();
      right.clear();

    // turn them on
    } else {
      leds_on = true;

      // set the colors
      if (color.toLowerCase() == "red"){
        R = 255;
        G = 0;
        B = 0;

      } else if (color.toLowerCase() == "orange"){
        R = 255;
        G = 165;
        B = 0;

      } else if (color.toLowerCase() == "yellow"){
        R = 255;
        G = 255;
        B = 0;

      } else if (color.toLowerCase() == "green"){
        R = 0;
        G = 255;
        B = 0;

      } else if (color.toLowerCase() == "aqua"){
        R = 0;
        G = 255;
        B = 255;

      } else if (color.toLowerCase() == "blue"){
        R = 0;
        G = 0;
        B = 255;

      } else if (color.toLowerCase() == "purple"){
        R = 255;
        G = 0;
        B = 255;

      } else {
        R = 255;
        G = 255;
        B = 255;
      }

      for (int i = 0; i <= left.numPixels(); i++){
        left.setPixelColor(i, R, G, B);
        right.setPixelColor(i, R, G, B); // assumes left and right counts are the same
      }
      for (int i = 0; i <= top.numPixels(); i++){
        top.setPixelColor(i, R, G, B);
      }
    }

    // update the LEDs
    left.show();
    top.show();
    right.show();
  }
}

#endif
