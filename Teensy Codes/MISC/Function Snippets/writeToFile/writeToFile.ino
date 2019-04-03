#include <SD.h>
#include <SPI.h>

File logFile;

// change this to match your SD shield or module;
// Teensy 3.5 & 3.6 on-board: BUILTIN_SDCARD
const int chipSelect = BUILTIN_SDCARD;

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect))
    Serial.println("initialization failed!");
    
  Serial.println("initialization done.");
}

void loop()
{
  // put your main code here, to run repeatedly:
  for (int i = 0; i <= 10; i++)
  {
    writeToFile(i);
    Serial.println(i);
  }
}

void writeToFile(int value)
{
  logFile = SD.open("test.txt", FILE_WRITE);
  if (logFile) 
  {
    logFile.println(value);
    logFile.close();
  } 

  else 
  {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
