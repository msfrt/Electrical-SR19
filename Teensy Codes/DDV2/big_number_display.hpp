#ifndef BIG_BIG_NUMBERS_ON_THE_SMALL_SMALL_SCREEN
#define BIG_BIG_NUMBERS_ON_THE_SMALL_SMALL_SCREEN

struct NumberDisplay{

  String label;
  ILI9341_t3n &screen;
  StateSignal &signal;
  int last_val = 9999;
  int current_val = 0;
  unsigned long startup_until;
  unsigned long startup_message_millis = 1250;
  bool startup;

  uint16_t number_color = ILI9341_WHITE;

  // initializer
  NumberDisplay(ILI9341_t3n &display, StateSignal &sig, String lab) : label(lab), screen(display), signal(sig) {};

  // begin the display and write startup message
  void begin(bool startup_screen = true);

  // check if the display needs to be updated, and update if necessary
  bool update(bool override = false);
};


void NumberDisplay::begin(bool startup_screen){
  // clear the screen and draw two borders
  screen.fillScreen(ILI9341_BLACK);
  screen.drawFastHLine(0,                  0, DISPLAY_WIDTH, ILI9341_GREEN);
  screen.drawFastHLine(0, DISPLAY_HEIGHT - 1, DISPLAY_WIDTH, ILI9341_GREEN);

  this->startup = true;

  this->last_val = 9999; // reset last val so it will automatically update upon the first invocation of update();

  if (startup_screen){
    this->startup_until = millis() + startup_message_millis; // set the delay for the startup screen
    screen.setFont(LiberationMono_72_Bold_Italic); // each char is 60 px wide
    screen.setCursor((DISPLAY_WIDTH - (this->label.length() * 60)) / 2 - 5, (DISPLAY_HEIGHT - 72) / 2);
    screen.print(this->label);
  }
}


bool NumberDisplay::update(bool override){

  this->current_val = round(this->signal.value()); // turn floating-point into an int

  // if we need to update the screen...
  if ((override || (this->current_val != this->last_val)) && (millis() > this->startup_until)) {
    this->last_val = this->current_val;

    // clear the screen except for the green bars and the label
    screen.fillRect(0, 40, DISPLAY_WIDTH, DISPLAY_HEIGHT - 85, ILI9341_BLACK);

    // write the mini-label, but only once
    if (this->startup){
      this->startup = false;

      screen.setCursor(2, 5);
      screen.setFont(LiberationMono_32_Bold_Italic);
      screen.setTextColor(ILI9341_WHITE);
      screen.print(this->label);
    }

    switch (this->current_val) {
      case -10:
        screen.drawBitmap(0, 0, big_num_neg10, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case -9:
        screen.drawBitmap(0, 0, big_num_neg9, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case -8:
        screen.drawBitmap(0, 0, big_num_neg8, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case -7:
        screen.drawBitmap(0, 0, big_num_neg7, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case -6:
        screen.drawBitmap(0, 0, big_num_neg6, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case -5:
        screen.drawBitmap(0, 0, big_num_neg5, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case -4:
        screen.drawBitmap(0, 0, big_num_neg4, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case -3:
        screen.drawBitmap(0, 0, big_num_neg3, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case -2:
        screen.drawBitmap(0, 0, big_num_neg2, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case -1:
        screen.drawBitmap(0, 0, big_num_neg1, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case 0:
        screen.drawBitmap(0, 0, big_num_0, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case 1:
        screen.drawBitmap(0, 0, big_num_1, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case 2:
        screen.drawBitmap(0, 0, big_num_2, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case 3:
        screen.drawBitmap(0, 0, big_num_3, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case 4:
        screen.drawBitmap(0, 0, big_num_4, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case 5:
        screen.drawBitmap(0, 0, big_num_5, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case 6:
        screen.drawBitmap(0, 0, big_num_6, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case 7:
        screen.drawBitmap(0, 0, big_num_7, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case 8:
        screen.drawBitmap(0, 0, big_num_8, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case 9:
        screen.drawBitmap(0, 0, big_num_9, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      case 10:
        screen.drawBitmap(0, 0, big_num_10, DISPLAY_WIDTH, DISPLAY_HEIGHT, this->number_color);
        return true;
        break;

      // There is no bitmap for the current number, print N/A
      default:
        screen.setFont(LiberationMono_96_Bold);
        display_left.setCursor(37, 73);

        if (this->current_val < -10){
          screen.setTextColor(ILI9341_RED); // below lower bound
        } else {
          screen.setTextColor(ILI9341_YELLOW); // above upper bound
        }
        screen.print("N/A");
        break;

    }
  }

  return false;
}

#endif
