#ifndef CAN_READ_DRIVERDISPLAY_HPP
#define CAN_READ_DRIVERDISPLAY_HPP

#include "sigs_inside.hpp"
#include <FlexCAN_T4.h>
#include "user_message_display.hpp"

static CAN_message_t rxmsg;
extern UserMessageDisplay warning_message_display;


// ID 159 on bus 2
void read_PDM_159(CAN_message_t &imsg){
  PDM_pdmVoltAvg.set_can_value(imsg.buf[3] | imsg.buf[4] << 8);
}

void read_PDM_163(CAN_message_t &imsg){
  PDM_fanLeftPWM.set_can_value(imsg.buf[2]);
}




// ID 100 on bus 1 - M400 dataset 1
void read_M400_1519(CAN_message_t &imsg){
  // multiplexer first-bit
  switch (imsg.buf[0]) {

    case 0:
      M400_rpm.set_can_value(imsg.buf[4] << 8 | imsg.buf[5]);
      break;

    case 4:
      M400_engineTemp.set_can_value(imsg.buf[4] << 8 | imsg.buf[5]);
      break;

    case 5:
      M400_oilPressure.set_can_value(imsg.buf[4] << 8 | imsg.buf[5]);
      M400_oilTemp.set_can_value(imsg.buf[6] << 8 | imsg.buf[7]);
      break;

  }
}


// ID 101 on bus 1 - M400 dataset 2
void read_M400_1520(CAN_message_t &imsg){
  // multiplexer first-bit
  switch (imsg.buf[0]) {

    case 0:
      M400_groundSpeed.set_can_value(imsg.buf[6] << 8 | imsg.buf[7]);
      break;

  }
}



// function that reads the msg and then directs that data elsewhere
void read_can1(){
  if (cbus1.read(rxmsg)){

    switch (rxmsg.id) {
      case 1520:
        read_M400_1520(rxmsg);
        break;
      case 1519:
        read_M400_1519(rxmsg);
        break;
    } // end switch statement

  }
}


// function that reads the msg and then directs that data elsewhere
void read_can2(){
  if (cbus2.read(rxmsg)){

    switch (rxmsg.id) {
      case 159:
        read_PDM_159(rxmsg);
        break;
        
      case 163:
        read_PDM_163(rxmsg);
        break;

    } // end switch statement

  }
}

#endif
