#ifndef CAN_SEND_DRIVERDISPLAY_HPP
#define CAN_SEND_DRIVERDISPLAY_HPP

#include <FlexCAN_T4.h>
#include <EasyTimer.h>
#include <BoardTemp.h>
#include "sigs_inside.hpp"


static CAN_message_t msg;


void send_DD_10(){
  // the following line initializes a counter that is specific to PDM_10, but is static, so it's only created once.
  static StateCounter ctr;

  // update message ID and length
  msg.id = 210;
  msg.len = 8;

  // since the last calculation event, this records the sensor data into the actual signals. The calculations are
  // done automatically. why don't we simply just throw the sensors into the can message buffer, you ask? Well, the
  // signals are global and are used elsewhere for other important PDM functions.
  DD_boardTemp = board_temp.value();
  DD_teensyTemp = tempmonGetTemp();
  DD_requestDRS = 0; // unimplemented


  // load up the message buffer
  msg.buf[0] = ctr.value();
  msg.buf[1] = 0;
  msg.buf[2] = DD_boardTemp.can_value();
  msg.buf[3] = DD_boardTemp.can_value() >> 8;
  msg.buf[4] = DD_teensyTemp.can_value();
  msg.buf[5] = DD_teensyTemp.can_value() >> 8;
  msg.buf[6] = DD_requestDRS.can_value();
  msg.buf[7] = DD_requestDRS.can_value() >> 8;

  // send the message
  cbus2.write(msg);
}



void send_can1(){
  // hey, there's nothing in here! (for now, at least)
}

void send_can2(){

  static EasyTimer DD_10_timer(50);
  if (DD_10_timer.isup()){
    send_DD_10();
  }

}

#endif
