int ANAVolt[21], SensVal[21];

int led = 13;
void setup() {
  // put your setup code here, to run once:
  pinMode(led, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(13, 1);
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
  ANAread[15] = analogRead(A14);
  ANAread[16] = analogRead(A15);
  ANAread[17] = analogRead(A16);
  ANAread[18] = analogRead(A17);
  ANAread[19] = analogRead(A18);
  ANAread[20] = analogRead(A19);
  ANAread[21] = analogRead(A20);

  Serial.print("\n*************");
    for (int i = 0; i < 21; i++) {
    ANAVolt[i] = ANAread[i] *  33000 / 8191 ;
    ANAVolt[i] = ANAVolt[i] / 2200.0000 * 3400.0000 ;
    Serial.print("\nANAVolt[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.print(ANAVolt[i]);
    }
  digitalWrite(13, 0);
  delay(500);
}
