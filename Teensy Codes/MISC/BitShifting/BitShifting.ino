/*
 * Written by:    Dave Yonkers
 * Created :      01/30/2019
 * Modified By:   Dave Yonkers
 * Last Modified: 01/30/2019 1:07 AM
 * Version:       0.0
 * Purpose:       Test bitwise operators
 * Description:   Trying to utilize the different bits in a byte
 */




const int ledPin = 13;

 void setup() {
  // initialize the serial
  Serial.begin(9600);

  // set the LED pin to an output and turn it on
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);


  

}

void loop() {

  uint8_t numberOne = 11; // input a number between 0 - 15
  uint8_t numberTwo = 0; // input a number between 0 - 3
  uint8_t numberThree = 3; // input a number between 0 - 3


  // arrange the numbers into one 8 bit number

  numberOne = numberOne << 4; // shift numberOne to the first four bits
  numberTwo = numberTwo << 2; // shift numberTwo to the 5th and 6th bit
                              // numberThree is already in the 7th and 8th bits

  // combine all of the 1s in the numbers to compile a final output
  uint8_t finalNumber = numberOne | numberTwo | numberThree;


  //Serial.println(finalNumber);

  // SIGNED INTEGER TEST - enter an integer from -32767 to +32767
  int inputValue = -10305;

  uint8_t rxData1 = inputValue >> 8;
  uint8_t rxData2 = inputValue;

  int16_t conventional = rxData1 * 256 + rxData2;
  Serial.print("Conventional: "); Serial.println(conventional);

  int16_t bitShifting = (rxData1 << 8) | rxData2;
  Serial.print(" bitShifting: "); Serial.println(bitShifting);

  delay(1000);


}
