#ifndef LAP_TIMER_SCREENS_HPP
#define LAP_TIMER_SCREENS_HPP

bool lap_timer_screen(ILI9341_t3n &left, ILI9341_t3n &right, StateSignal &lap_trigger_sig,
                      float time_ary[4], float time_diff_ary[4], int lap_nums_ary[4]) {
  static unsigned long last_lap_trigger_millis = 0;
  static unsigned long lap_duration_millis = 0; // the new lap time
  static unsigned long current_time_millis;
  static int current_lap = 0;
  static bool display_on = false; // used to return when the display should be on

  static float lap_seconds; // the most recent lap time
  static float last_lap_seconds = 0; // old lap time
  static float lap_difference = 0; // time difference

  static const unsigned long laptime_display_duration = 6000; // how long the lap time should stay up on the screen
  static const unsigned long trigger_timeout_duration = 5000; // number of milliseconds before another lap can be triggered
  static unsigned long no_trigger_until_millis = 0; // used in conjuction with timeout_duration. No need to change this
  static unsigned long display_until_millis = 0; // used in conjuction with laptime_display_duration.


  // update the current time;
  current_time_millis = millis();


  // we should trigger a new lap
  if ((lap_trigger_sig.value() != 0) && (current_time_millis > no_trigger_until_millis)){

    no_trigger_until_millis = current_time_millis + trigger_timeout_duration; // set timout duration to prevent double-triggers
    display_until_millis = current_time_millis + laptime_display_duration; // set the time to display the screen until
    current_lap++; // increment the lap counter

    // calculate the lap time ---------------------
    lap_duration_millis = current_time_millis - last_lap_trigger_millis;
    last_lap_trigger_millis = current_time_millis; // update the last trigger

    lap_seconds = static_cast<float>(lap_duration_millis) / 1000.0;
    lap_difference = lap_seconds - last_lap_seconds;
    last_lap_seconds = lap_seconds; // update for the next time we trigger a lap


    // display the lap time and lap difference-----------------------------

    // prep screen
    left.fillScreen(ILI9341_BLACK);
    left.drawFastHLine(0,                  0, DISPLAY_WIDTH, ILI9341_GREEN);
    left.drawFastHLine(0, DISPLAY_HEIGHT - 1, DISPLAY_WIDTH, ILI9341_GREEN);

    // display header
    left.setCursor(2, 6);
    left.setFont(LiberationMono_32_Bold_Italic);
    left.setTextColor(ILI9341_WHITE);
    left.print("LAP TIME");

    // prep laptime font and color
    left.setFont(LiberationMono_60_Bold); // about 24px wide
    left.setTextColor(ILI9341_WHITE);

    // positioning to center the time on the screen
    if (lap_seconds >= 100){
      left.setCursor(10, 95);
    } else if (lap_seconds >= 10) {
      left.setCursor(35, 95);
    } else {
      left.setCursor(58, 95);
    }

    // print the lap time
    left.print(lap_seconds, 2);

    // prep screen
    right.fillScreen(ILI9341_BLACK);
    right.drawFastHLine(0,                  0, DISPLAY_WIDTH, ILI9341_GREEN);
    right.drawFastHLine(0, DISPLAY_HEIGHT - 1, DISPLAY_WIDTH, ILI9341_GREEN);

    // display header
    right.setCursor(2, 6);
    right.setFont(LiberationMono_32_Bold_Italic);
    right.setTextColor(ILI9341_WHITE);
    right.print("LAP DIFF");

    // font selection for time
    right.setFont(LiberationMono_60_Bold); // about 24px wide

    // color selection
    if (lap_difference > 0){
      right.setTextColor(ILI9341_RED);
    } else {
      right.setTextColor(ILI9341_GREEN);
    }

    // positioning to center the time on the screen
    if (lap_difference < 0){
      if (lap_difference < -10){
        right.setCursor(10, 95);
      } else {
        right.setCursor(35, 95);
      }
    } else {
      if (lap_difference < 10){
        right.setCursor(35, 95);
      } else {
        right.setCursor(10, 95);
      }

      right.print("+");
    }

    right.print(lap_difference);


    // edit the time arrays to include the new calculations --------------------------

    // move everything we had back one index (Prof. Onsay, don't hate me for doing it this way :////)
    for (int i = 3; i > 0; i--){
      time_ary[i] = time_ary[i - 1];
    }
    time_ary[0] = lap_seconds;

    for (int i = 3; i > 0; i--){
      time_diff_ary[i] = time_diff_ary[i - 1];
    }
    time_diff_ary[0] = lap_difference;

    for (int i = 3; i > 0; i--){
      lap_nums_ary[i] = lap_nums_ary[i - 1];
    }
    lap_nums_ary[0] = current_lap;


    // set the display state to ON
    display_on = true;

  // the lap has been triggered, now we just need to continue showing the display
  } else if (current_time_millis < display_until_millis){
    display_on = true;

  // laptime display should not be on
  } else {
    display_on = false;
  }


  return display_on;
}


// similar to big number display and info screen. this is like a fusion of the two
struct LapTimeDisplay{

  int x_buff_px = 1;
  int y_buff_px = 12;
  int char_width_px = 35;

  String label;
  ILI9341_t3n &screen;
  int *lap_nums_ary; // 4 INTEGERS LONG
  float *lap_times_ary; // 4 FLOATS LONG
  int last_lap = -1; // used to store the last lap number printed, and to determine if the screen needs to be updated
  unsigned long startup_until;
  unsigned long startup_message_millis = 1250;
  bool startup;
  bool colorful; // if laptimes should be green/red

  // initializer
  LapTimeDisplay(ILI9341_t3n &display, int *nums_ary, float *times_ary, String lab, bool colors) : label(lab),
                 screen(display), lap_nums_ary(nums_ary), lap_times_ary(times_ary), colorful(colors) {};

  // begin the display and write startup message
  void begin(bool startup_screen = true);

  // check if the display needs to be updated, and update if necessary
  bool update(bool override = false);

  int get_xpos(float value);

};


void LapTimeDisplay::begin(bool startup_screen){
  // clear the screen and draw two borders
  screen.fillScreen(ILI9341_BLACK);
  screen.drawFastHLine(0,                  0, DISPLAY_WIDTH, ILI9341_GREEN);
  screen.drawFastHLine(0, DISPLAY_HEIGHT - 1, DISPLAY_WIDTH, ILI9341_GREEN);
  
  this->last_lap = -1;

  if (startup_screen){
    this->startup_until = millis() + this->startup_message_millis;
    this->startup = true;

    screen.setTextColor(ILI9341_WHITE);
    screen.setFont(LiberationMono_72_Bold_Italic); // each char is 60 px wide
    screen.setCursor((DISPLAY_WIDTH - (this->label.length() * 60)) / 2 - 5, (DISPLAY_HEIGHT - 72) / 2);
    screen.print(this->label);

} else {
  this->update(true); // print the screen right now!
}

}


int LapTimeDisplay::get_xpos(float val){
  int num_digits = 0;

  if (val >= 100){
    num_digits = 6;
  } else if (val >= 10){
    num_digits = 5;
  } else if (val >= 0){
    num_digits = 4;
  } else if (val > -10){
    num_digits = 5;
  } else if (val > -100){
    num_digits = 6;
  } else {
    num_digits = 7;
  }

  if (this->colorful && val > 0){
    num_digits++;
  }

  return DISPLAY_WIDTH - (num_digits * this->char_width_px);
}


bool LapTimeDisplay::update(bool override){


  // if we need to update the screen...
  if ((override || (this->lap_nums_ary[0] != this->last_lap)) && (millis() > this->startup_until)) {
    this->last_lap = this->lap_nums_ary[0];
    this->startup = false;

    // reset screen --------------------
    screen.fillScreen(ILI9341_BLACK);
    screen.drawFastHLine(0, (DISPLAY_HEIGHT / 4) * 0,     DISPLAY_WIDTH, ILI9341_GREEN);
    screen.drawFastHLine(0, (DISPLAY_HEIGHT / 4) * 1,     DISPLAY_WIDTH, ILI9341_GREEN);
    screen.drawFastHLine(0, (DISPLAY_HEIGHT / 4) * 2,     DISPLAY_WIDTH, ILI9341_GREEN);
    screen.drawFastHLine(0, (DISPLAY_HEIGHT / 4) * 3,     DISPLAY_WIDTH, ILI9341_GREEN);
    screen.drawFastHLine(0, (DISPLAY_HEIGHT / 4) * 4 - 1, DISPLAY_WIDTH, ILI9341_GREEN);


    // draw labels --------------------
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextWrap(false);
    screen.setFont(LiberationMono_40_Bold);

    screen.setCursor(this->x_buff_px, this->y_buff_px);
    screen.print(this->lap_nums_ary[0]); screen.print(":");
    screen.setCursor(this->x_buff_px, (DISPLAY_HEIGHT / 4) * 1 + this->y_buff_px);
    screen.print(this->lap_nums_ary[1]); screen.print(":");
    screen.setCursor(this->x_buff_px, (DISPLAY_HEIGHT / 4) * 2 + this->y_buff_px);
    screen.print(this->lap_nums_ary[2]); screen.print(":");
    screen.setCursor(this->x_buff_px, (DISPLAY_HEIGHT / 4) * 3 + this->y_buff_px);
    screen.print(this->lap_nums_ary[3]); screen.print(":");

    // write the times -----------------
    if (!this->colorful){
      screen.setCursor(this->get_xpos(lap_times_ary[0]), (DISPLAY_HEIGHT / 4) * 0 + this->y_buff_px);
      screen.print(lap_times_ary[0]);
      screen.setCursor(this->get_xpos(lap_times_ary[1]), (DISPLAY_HEIGHT / 4) * 1 + this->y_buff_px);
      screen.print(lap_times_ary[1]);
      screen.setCursor(this->get_xpos(lap_times_ary[2]), (DISPLAY_HEIGHT / 4) * 2 + this->y_buff_px);
      screen.print(lap_times_ary[2]);
      screen.setCursor(this->get_xpos(lap_times_ary[3]), (DISPLAY_HEIGHT / 4) * 3 + this->y_buff_px);
      screen.print(lap_times_ary[3]);

    } else {

      // set color AND print the times
      screen.setCursor(this->get_xpos(lap_times_ary[0]), (DISPLAY_HEIGHT / 4) * 0 + this->y_buff_px);
      if (lap_times_ary[0] > 0){
        screen.setTextColor(ILI9341_RED);
        screen.print("+");
      } else {
        screen.setTextColor(ILI9341_GREEN);
      }
      screen.print(lap_times_ary[0]);


      screen.setCursor(this->get_xpos(lap_times_ary[1]), (DISPLAY_HEIGHT / 4) * 1 + this->y_buff_px);
      if (lap_times_ary[1] > 0){
        screen.setTextColor(ILI9341_RED);
        screen.print("+");
      } else {
        screen.setTextColor(ILI9341_GREEN);
      }
      screen.print(lap_times_ary[1]);


      screen.setCursor(this->get_xpos(lap_times_ary[2]), (DISPLAY_HEIGHT / 4) * 2 + this->y_buff_px);
      if (lap_times_ary[2] > 0){
        screen.setTextColor(ILI9341_RED);
        screen.print("+");
      } else {
        screen.setTextColor(ILI9341_GREEN);
      }
      screen.print(lap_times_ary[2]);


      screen.setCursor(this->get_xpos(lap_times_ary[3]), (DISPLAY_HEIGHT / 4) * 3 + this->y_buff_px);
      if (lap_times_ary[3] > 0){
        screen.setTextColor(ILI9341_RED);
        screen.print("+");
      } else {
        screen.setTextColor(ILI9341_GREEN);
      }
      screen.print(lap_times_ary[3]);

    }



    return true;
  }

  return false;
}



#endif
