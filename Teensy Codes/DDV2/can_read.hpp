#ifndef CAN_READ_DRIVERDISPLAY_HPP
#define CAN_READ_DRIVERDISPLAY_HPP

#include "sigs_inside.hpp"
#include <FlexCAN_T4.h>
#include "user_message_display.hpp"

static CAN_message_t rxmsg;
extern UserMessageDisplay warning_message_display;


// ID 410 on bus 2
void read_ATCCF_10(CAN_message_t &imsg){
  ATCCF_brakeBias.set_can_value(imsg.buf[2] | imsg.buf[3] << 8);
}

// ID 411 on bus 2
void read_ATCCF_11(CAN_message_t &imsg){
  ATCCF_brakePressureF.set_can_value(imsg.buf[2] | imsg.buf[3] << 8);
  ATCCF_brakePressureR.set_can_value(imsg.buf[4] | imsg.buf[5] << 8);
}


// ID 261 on bus 2
void read_PDM_11(CAN_message_t &imsg){
  PDM_pdmVoltAvg.set_can_value(imsg.buf[2] | imsg.buf[3] << 8);
}


// ID 274 on bus 2
void read_PDM_24(CAN_message_t &imsg){
  PDM_fanLeftPWM.set_can_value(imsg.buf[2]);
}


// ID 710 on bus 2
void read_USER_10(CAN_message_t &imsg){
  USER_fanLeftOverride.set_can_value(imsg.buf[0]);
  USER_fanRightOverride.set_can_value(imsg.buf[1]);
  USER_wpOverride.set_can_value(imsg.buf[2]);
  USER_brakeLightOverride.set_can_value(imsg.buf[3]);
}


// ID 711 on bus 2
void read_USER_11(CAN_message_t &imsg){
  USER_driverSignal.set_can_value(imsg.buf[0]);
}

// ID 281 on bus 2
void read_PDM_281(CAN_message_t &imsg){
  PDM_driverDisplayLEDs.set_can_value(imsg.buf[0]);
}


// ID 712 or ID 280 on bus 2 - the 64-bit message
char obd_message[9] = ""; // <= 8 characters!!! One extra char in definition for the null-terminator.
void read_driver_message(CAN_message_t &imsg){
  for (int i = 0; i < 8; i++){
    obd_message[i] = imsg.buf[i];
  }
  obd_message[8] = '\0'; // just for safety

  warning_message_display.begin();
}



// ID 100 on bus 1 - M400 dataset 1
void read_M400_100(CAN_message_t &imsg){
  // multiplexer first-bit
  switch (imsg.buf[0]) {

    case 1:
      M400_groundSpeed.set_can_value(imsg.buf[2] << 8 | imsg.buf[3]);
      break;

    case 2:
      M400_tcPowerReduction.set_can_value(imsg.buf[6] << 8 | imsg.buf[7]);
      break;

    case 4:
      M400_ignCutLevelTotal.set_can_value(imsg.buf[2] << 8 | imsg.buf[3]);
      M400_rpm.set_can_value(imsg.buf[4] << 8 | imsg.buf[5]);
      break;

    case 5:
      M400_gear.set_can_value(imsg.buf[2] << 8 | imsg.buf[3]);
      break;

  }
}


// ID 101 on bus 1 - M400 dataset 2
void read_M400_101(CAN_message_t &imsg){
  // multiplexer first-bit
  switch (imsg.buf[0]) {

    case 2:
      M400_batteryVoltage.set_can_value(imsg.buf[6] << 8 | imsg.buf[7]);
      break;

    case 3:
      M400_engineTemp.set_can_value(imsg.buf[6] << 8 | imsg.buf[7]);
      break;

    case 10:
      M400_fuelPressure.set_can_value(imsg.buf[2] << 8 | imsg.buf[3]);
      break;

    case 15:
      M400_oilPressure.set_can_value(imsg.buf[6] << 8 | imsg.buf[7]);
      break;

    case 16:
      M400_oilTemp.set_can_value(imsg.buf[2] << 8 | imsg.buf[3]);
      break;
  }
}



// function that reads the msg and then directs that data elsewhere
void read_can1(){
  if (cbus1.read(rxmsg)){

    switch (rxmsg.id) {
      case 100:
        read_M400_100(rxmsg);
        break;
      case 101:
        read_M400_101(rxmsg);
        break;
    } // end switch statement

  }
}


// function that reads the msg and then directs that data elsewhere
void read_can2(){
  if (cbus2.read(rxmsg)){

    switch (rxmsg.id) {
      case 261:
        read_PDM_11(rxmsg);
        break;
      case 274:
        read_PDM_24(rxmsg);
        break;
      case 281:
        read_PDM_281(rxmsg);
        break;
      case 280:
      case 712:
        read_driver_message(rxmsg);
        break;
      case 410:
        read_ATCCF_10(rxmsg);
        break;
      case 411:
        read_ATCCF_11(rxmsg);
        break;
      case 710:
        read_USER_10(rxmsg);
        break;
      case 711:
        read_USER_11(rxmsg);
        break;
    } // end switch statement

  }
}

#endif
