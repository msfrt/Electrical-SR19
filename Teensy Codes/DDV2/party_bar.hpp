#include <StateCAN.h>
#include <Adafruit_NeoPixel.h>
#include <EasyTimer.h>


// RGB LEDs are siiiccckkkkkkk bro
void party_bar(Adafruit_NeoPixel &top, Adafruit_NeoPixel &left, Adafruit_NeoPixel &right){

  // static variable definitions. The only thing to change here is the frequency in the rgb_update_timer
  static EasyTimer rgb_update_timer(10);
  static int red = 255;
  static int green = 0;
  static int blue = 0;
  static int current_step = 0;

  if (rgb_update_timer.isup()){

    // set the top pixels
    for (int i = 0; i <= top.numPixels(); i++){
      top.setPixelColor(i, red, green, blue);
    }

    // set the left pixels
    for (int i = 0; i <= left.numPixels(); i++){
      left.setPixelColor(i, red, green, blue);
    }

    // set the right pixels
    for (int i = 0; i <= right.numPixels(); i++){
      right.setPixelColor(i, red, green, blue);
    }

    // output the pixel colors
    top.show();
    left.show();
    right.show();

    // division is done by 17, because after 17 increments of (+) or (-) 15, you will be at 255 or 0.

    // change the RGB colors
    if (current_step / 17 == 0){
      blue += 15;

    } else if (current_step / 17 == 1){
      red -= 15;

    } else if (current_step / 17 == 2){
      green += 15;

    } else if (current_step / 17 == 3){
      blue -= 15;

    } else if (current_step / 17 == 4){
      red += 15;

    } else if (current_step / 17 == 5){
      green -= 15;

    // reset
    } else {
      red = 255;
      green = 0;
      blue = 0;
      current_step = -1;
    }

    // increment the step counter
    current_step++;

  } // end update timer
}
