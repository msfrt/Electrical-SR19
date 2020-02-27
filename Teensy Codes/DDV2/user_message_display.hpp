#ifndef THE_CARRIER_PIGEON_SENDS_A_MESSAGE_HPP
#define THE_CARRIER_PIGEON_SENDS_A_MESSAGE_HPP

#include <ILI9341_t3n.h>

struct UserMessageDisplay{

  String label;
  String message;
  char *messagearray = nullptr; // Should be a pointer to an array >= 8 chars in length (>=64 bits)
  ILI9341_t3n &screen;
  unsigned long message_display_time = 10000; // millis that the message is on the screen
  unsigned long message_display_until;
  uint16_t color = ILI9341_WHITE;

  // initializer
  UserMessageDisplay(ILI9341_t3n &display, char *message_pointer, String lab, uint16_t color = ILI9341_WHITE) :
                     label(lab), messagearray(message_pointer), screen(display), color(color) {};

  // begin showing the display message.
  void begin();

  // doesn't actually write anything to the screen, but returns true if the display should still be active. The
  // parameter, display_on, can be set to false if you want to turn the display off before the timer is up.
  bool show(bool display_on = true);
};


void UserMessageDisplay::begin(){
  static int message_len = 0;

  // prevent the rapid-firing of messages by only writing once in the specified period
  if (millis() > message_display_until){
    message_display_until = millis() + message_display_time;

    // clear the screen and draw two borders
    screen.fillScreen(ILI9341_BLACK);
    screen.drawFastHLine(0,                  0, DISPLAY_WIDTH, ILI9341_GREEN);
    screen.drawFastHLine(0, DISPLAY_HEIGHT - 1, DISPLAY_WIDTH, ILI9341_GREEN);


    screen.setCursor(2, 5);
    screen.setFont(LiberationMono_32_Bold_Italic);
    screen.setTextColor(ILI9341_WHITE);
    screen.print(label);


    screen.setFont(LiberationMono_48_Bold);
    screen.setTextColor(color);

    message_len = 0;
    for (int i = 0; (i < 10) && (messagearray[i] != '\0'); i++){
      message_len++;
    }

    screen.setCursor((8 - message_len) * (DISPLAY_WIDTH / 16), DISPLAY_HEIGHT/2 - 48 / 2);
    screen.print(messagearray);
  }
}


bool UserMessageDisplay::show(bool display_on){

  if (!display_on){
    message_display_until = millis();
  }

  if (millis() < message_display_until){
    return true;
  }

  return false;
}


#endif
