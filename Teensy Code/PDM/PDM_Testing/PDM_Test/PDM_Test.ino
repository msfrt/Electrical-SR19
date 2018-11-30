#include <FlexCAN.h>
#include <kinetis_flexcan.h>

int waterPump = 250;
int fan = 150;

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(A9, INPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);

  delay(3000);
}

void loop()
{
  //Read VIN
  
  //Serial.println(analogRead(A9));

  //Write water pump value to pin 5
  
  analogWrite(5, waterPump);

  //Write fan value to pin 6
  analogWrite(6, fan);
//  delay(500);
//  digitalWrite(13, HIGH);
//  delay(500);
//  digitalWrite(13, LOW);
}
