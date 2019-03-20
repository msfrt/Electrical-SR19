

#include <FlexCAN.h>

FlexCAN CANbus(1000000);
static CAN_message_t msg;

int ANAVolt[21], SensVal[21];
unsigned long SendTimmer1000Hz = 0;
unsigned long  SendTimmer500Hz = 0, SendTimmer200Hz = 0, SendTimmer100Hz = 0;
unsigned long   SendTimmer50Hz = 0,  SendTimmer20Hz = 0,  SendTimmer10Hz = 0;

// -------------------------------------------------------------
void setup(void)
{
  CANbus.begin();
  analogReadResolution(13);
  pinMode(13, OUTPUT);
  digitalWrite(13, 1);
  delay(1000);
  digitalWrite(13, 0);
  Serial.print("Gua - Send\n\n");

  //AT/AV Pull up 1K resistor to 3.3 activation, should be 1 for deactive, 0 for active
//  pinMode(5, OUTPUT);
//  pinMode(6, OUTPUT);
//  pinMode(7, OUTPUT);
//  pinMode(8, OUTPUT);
//  digitalWrite(5, 1); // A13
//  digitalWrite(6, 1); // A18
//  digitalWrite(7, 1); // A19
//  digitalWrite(8, 1); // A20

}// -------------------------------------------------------------

void loop(void)
{

  READ_ANA();             // Read from Analog voltage pins
  ANAVoltCalibration();   // convert from 3.3v to 5v scall 1.2K and 2.2K resistor volt divider

  // ***** sensor calibration *****                       //factor, min rate, name
  SensVal[0]  = ( ANAVolt[ 0] - 25000 ) /  2500.0000 *   500; //  0.01, 100Hz, Rectifier Current 50B
//  SensVal[1]  = ( ANAVolt[ 1] -  5000 ) / 40000.0000 *  8000; //   0.1,  10Hz, Brake Rotor Temp Rear
  SensVal[2]  = ( ANAVolt[ 2] -  5000 ) / 40000.0000 * 20000; //   0.1,  20Hz, Brake Pressure 2

//SensVal[1]  = 50;
//SensVal[3]  = 1;
  
  SensVal[3]  = ( ANAVolt[ 3] -  5000 ) / 40000.0000 *  1500; //   0.1,  10Hz, Tire Temp RLO
//  SensVal[4]  = ( ANAVolt[ 4] -  5000 ) / 40000.0000 *  1500; //   0.1,  10Hz, Tire Temp RLM
//  SensVal[5]  = 20000; //   0.1,  10Hz, Tire Temp RLI
//  SensVal[5]  = ( ANAVolt[ 5] -  5000 ) / 40000.0000 *  1500; //   0.1,  10Hz, Tire Temp RLI
  SensVal[3]  = ( ANAVolt[ 3] -  4000 ) * 0.06 + 320; //   0.1,  10Hz, Tire Temp RLO
  SensVal[4]  = ( ANAVolt[ 4] -  4000 ) * 0.06 + 320; //   0.1,  10Hz, Tire Temp RLM
  SensVal[5]  = ( ANAVolt[ 5] -  4000 ) * 0.06 + 320; //   0.1,  10Hz, Tire Temp RLI
  SensVal[6]  = ( ANAVolt[ 6]         ) / 50000.0000 *  7500; //   0.1, 100Hz, Linear Pot RL
  SensVal[7]  = ( ANAVolt[ 7]         ) / 50000.0000 *  7500; //   0.1, 100Hz, Linear Pot RR
  SensVal[8]  = ( ANAVolt[ 8] -  5000 ) / 40000.0000 *  1500; //   0.1,  10Hz, Tire Temp RRO
  SensVal[9]  = ( ANAVolt[ 9] -  5000 ) / 40000.0000 *  1500; //   0.1,  10Hz, Tire Temp RRM
  SensVal[10] = ( ANAVolt[10] -  5000 ) / 40000.0000 *  1500; //   0.1,  10Hz, Tire Temp RRI
  SensVal[11] = ( ANAVolt[11] - 25000 ) /  2500.0000 *  1000; //  0.01, 100Hz, ESP Motor Current 100B
  SensVal[12] = BoschTempHS(ANAVolt[12] / 10, 5000, 470) ;    //   0.1,  10Hz, Water Inlet Temp // **AT**
  SensVal[13] = 0 ;                                           // **AT/AV**
  SensVal[15] = BoschTempHS(ANAVolt[15] / 10, 5000, 470) ;    //   0.1,  10Hz, Water Outlet Temp // **AT**
  SensVal[16] = 0 ;                                           // **AT**
  SensVal[17] = 0 ;                                           // **AT**
  SensVal[18] = 0 ;                                           // ------------, 2nd BAT Current // **AT/AV**
  SensVal[19] = 0 ;                                           // **AT/AV**
  SensVal[20] = 0 ;                                           // ------------, 2nd BAT Volt    // **AT/AV**


  //unsigned long currentTime = millis();

  if ( millis() - SendTimmer500Hz >=  2 ) {
    SendTimmer500Hz = millis();
    CANMUX(); // get sensor value to Can message posision and send

  }

}

// ************Functions************

static void CAN_DATA_SEND(int id, int len) {
  msg.len = len;  //CAN message length unit: Byte (8 bits)
  msg.id = id; //CAN ID
  CANbus.write(msg);  // this is send the static
}

static void READ_ANA() {
  //---- Read analog pin ----------
  int ANAread[21];
  ANAread[0] = analogRead(A0);
  ANAread[1] = analogRead(A1);
  ANAread[2] = analogRead(A2);
  ANAread[3] = analogRead(A3);
  ANAread[4] = analogRead(A4);
  ANAread[5] = analogRead(A5);
  ANAread[6] = analogRead(A6);
  ANAread[7] = analogRead(A7);
  ANAread[8] = analogRead(A8);
  ANAread[9] = analogRead(A9);
  ANAread[10] = analogRead(A10);
  ANAread[11] = analogRead(A11);
  ANAread[12] = analogRead(A12);
  ANAread[13] = analogRead(A13);
  ANAread[15] = analogRead(A15);
  ANAread[16] = analogRead(A16);
  ANAread[17] = analogRead(A17);
  ANAread[18] = analogRead(A18);
  ANAread[19] = analogRead(A19);
  ANAread[20] = analogRead(A20);

  for (int i = 0; i < 21; i++) {
    ANAVolt[i] = ANAread[i] *  33000 / 8191 ;
    //Serial.print("\nANAVolt[");
    //Serial.print(i);
    //Serial.print("]: ");
    //Serial.print(ANAVolt[i]);
  }

}

static void ANAVoltCalibration() {
//  Serial.print("\n*******");
  for (int i = 0; i < 21; i++) {
    ANAVolt[i] = ANAVolt[i] / 2200.0000 * 3400.0000 ;

//    Serial.print("\nANAVolt[");
//    Serial.print(i);
//    Serial.print("]: ");
//    Serial.print(ANAVolt[i]);
  }
}


static void CANMUX() {


  //if ( millis() - SendTimmer500Hz >=  2 ) {
  //SendTimmer500Hz = millis();
  msg.buf[0] = SensVal[6];
  msg.buf[1] = SensVal[6] >> 8; //Linear Pot RL
  msg.buf[2] = SensVal[7];
  msg.buf[3] = SensVal[7] >> 8; //Linear Pot RR
  msg.buf[4] = SensVal[0];
  msg.buf[5] = SensVal[0] >> 8; //Rectifier Current
  msg.buf[6] = SensVal[11];
  msg.buf[7] = SensVal[11] >> 8; //ESP Motor Current
  //CAN_DATA_SEND(0x30, 8); // 500Hz

  msg.buf[0] = 0;
  msg.buf[1] = 0 >> 8;
  msg.buf[2] = 0;
  msg.buf[3] = 0 >> 8;
  msg.buf[4] = 0;
  msg.buf[5] = 0 >> 8;
  msg.buf[6] = 0;
  msg.buf[7] = 0 >> 8;
  //CAN_DATA_SEND(0x31, 8); //500Hz
  //}

  if ( millis() - SendTimmer50Hz >=  20 ) {
    SendTimmer50Hz = millis();
    msg.buf[0] = SensVal[2];
    msg.buf[1] = SensVal[2] >> 8; //Brake Pressure Rear
    msg.buf[2] = 0;
    msg.buf[3] = 0 >> 8;
    msg.buf[4] = 0;
    msg.buf[5] = 0 >> 8;
    msg.buf[6] = 0;
    msg.buf[7] = 0 >> 8;
    //CAN_DATA_SEND(0x32, 8); //50Hz
  }

  if ( millis() - SendTimmer10Hz >=  100 ) {
//    SendTimmer10Hz = millis();
//    msg.buf[0] = SensVal[1];
//    msg.buf[1] = SensVal[1] >> 8; //Brake Rotor Temp Rear
//    msg.buf[2] = SensVal[3];
//    msg.buf[3] = SensVal[3] >> 8; //Tire Temp RLO
//    msg.buf[4] = SensVal[4];
//    msg.buf[5] = SensVal[4] >> 8; //Tire Temp RLM
//    msg.buf[6] = SensVal[5];
//    msg.buf[7] = SensVal[5] >> 8; //Tire Temp RLI
//    CAN_DATA_SEND(0x33, 8);  //10Hz

    SendTimmer10Hz = millis();
    msg.buf[0] = SensVal[1];
    msg.buf[1] = SensVal[1] >> 8; //Brake Rotor Temp Rear
    msg.buf[2] = SensVal[3];
    msg.buf[3] = SensVal[3] >> 8; //Tire Temp RLO
    msg.buf[4] = 0;
    msg.buf[5] = 0;
    msg.buf[6] = 0;
    msg.buf[7] = 0;
    CAN_DATA_SEND(0x33, 8);  //10Hz



    msg.buf[0] = SensVal[8];
    msg.buf[1] = SensVal[8] >> 8; //Tire Temp RRO
    msg.buf[2] = SensVal[9];
    msg.buf[3] = SensVal[9] >> 8; //Tire Temp RRM
    msg.buf[4] = SensVal[10];
    msg.buf[5] = SensVal[10] >> 8; //Tire Temp RRI
    msg.buf[6] = 0;
    msg.buf[7] = 0 >> 8;
    //CAN_DATA_SEND(0x34, 8);  //10Hz

    msg.buf[0] = SensVal[12];
    msg.buf[1] = SensVal[12] >> 8; //Water Inlet Temp
    msg.buf[2] = SensVal[15];
    msg.buf[3] = SensVal[15] >> 8; //Water Outlet Temp
    msg.buf[4] = 0;
    msg.buf[5] = 0 >> 8;
    msg.buf[6] = 0;
    msg.buf[7] = 0 >> 8;
    //CAN_DATA_SEND(0x35, 8);  //10Hz


    //display the SensVal in current sending cycle
      Serial.print("\n*******");
      for (int i = 0; i < 21; i++) {
      Serial.print("\nSensVal[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.print(SensVal[i]);
      }

  
//      Serial.print("\nANAVolt[");
//      Serial.print(i);
//      Serial.print("]: ");
//      Serial.print(ANAVolt[i]);
  

      
  }
}

int BoschTempHS( int Input_mV, int Vs_mV, int Rpullup ) { //
  float cTemp[] = { -55, -35, -20, -10, 0, 10, 20, 25, 30, 40, 50, 60, 70, 80, 90, 100, 120, 140, 160, 180, 200, 220, 240, 260, 280, 300};
  float cR[]    = { 519910, 158090, 71668, 44087, 27936, 18187, 12136, 10000, 8284, 5774, 4103, 2967, 2182, 1629, 1234, 946.6, 578.1, 368.8, 244.4, 167.6, 118.5, 86.08, 64.08, 48.76, 37.86, 29.94 };
  float cVolt[sizeof(cTemp) / 4];

  if (sizeof(cTemp) != sizeof(cR)) {
    return -9999;
  }
  else {
    for (int i = 0; i < ((int) sizeof(cTemp) / 4); i++  ) {
      cVolt[i] = cR[i] / (cR[i] + (float) Rpullup) * (float) Vs_mV ;
      //Serial.println(cVolt[i]);
    }
  }

  if ((float) Input_mV > cVolt[0]) {
    return (int) ((cTemp[0] - 0.1) * 10);
  }
  else if ((float) Input_mV <  cVolt[ sizeof(cVolt) / 4 - 1 ] ) {
    return (int) (( cTemp[ sizeof(cTemp) / 4 - 1 ] + 0.1 ) * 10);
  }
  else {
    for (int i = 0; i < (int) (sizeof(cVolt) / 4 - 1); i++) {
      if ( ( (float) Input_mV <=  cVolt[i]) &&  ( (float) Input_mV >= cVolt[i + 1]) ) {
        return (int) ( ( ( (float) Input_mV - cVolt[i + 1]) / (cVolt[i] - cVolt[i + 1]) * ( cTemp[i] - cTemp[i + 1] ) + cTemp[i + 1]      ) * 10 );

      }
    }
  }
  return -7777;
}

int BoschTempH( int Input_mV, int Vs_mV, int Rpullup ) { //
  float cTemp[] = {   -40,   -30,   -20,  -10,    0,   10,   20,   30,   40,  50,  60,  70,  80,  90, 100, 110, 120, 130, 140, 150 };
  float cR[]    = { 45313, 26114, 15462, 9397, 5896, 3792, 2500, 1707, 1175, 834, 596, 436, 323, 243, 187, 144, 113,  89,  71,  57 };
  float cVolt[sizeof(cTemp) / 4];

  if (sizeof(cTemp) != sizeof(cR)) {
    return -9999;
  }
  else {
    for (int i = 0; i < ((int) sizeof(cTemp) / 4); i++  ) {
      cVolt[i] = cR[i] / (cR[i] + (float) Rpullup) * (float) Vs_mV ;
      //Serial.println(cVolt[i]);
    }
  }

  if ((float) Input_mV > cVolt[0]) {
    return (int) ((cTemp[0] - 0.1) * 10);
  }
  else if ((float) Input_mV <  cVolt[ sizeof(cVolt) / 4 - 1 ] ) {
    return (int) (( cTemp[ sizeof(cTemp) / 4 - 1 ] + 0.1 ) * 10);
  }
  else {
    for (int i = 0; i < (int) (sizeof(cVolt) / 4 - 1); i++) {
      if ( ( (float) Input_mV <=  cVolt[i]) &&  ( (float) Input_mV >= cVolt[i + 1]) ) {
        return (int) ( ( ( (float) Input_mV - cVolt[i + 1]) / (cVolt[i] - cVolt[i + 1]) * ( cTemp[i] - cTemp[i + 1] ) + cTemp[i + 1]      ) * 10 );

      }
    }
  }
  return -7777;
}
