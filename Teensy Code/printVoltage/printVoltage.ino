#include <SD.h>
#include <SPI.h>

const int buttonPin = 0;
const int ledPin = 13;
const int chipSelect = BUILTIN_SDCARD;

unsigned long sendTimer2000Hz = 0;

File logFile;
int logNum = 0;
char fileName[16];

double Vcc;
double Vout;
double seconds;

void setup()
{
  Serial.begin(115200);
  analogReadResolution(13);
  
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  
  //digitalWrite(buttonPin, HIGH);

  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect))
    Serial.println("initialization failed!");

  else
    Serial.println("initialization done.");

//  Serial.println("Clearing log file...");
//
//  if (!SD.remove("test.txt"))
//    Serial.println("No log to remove.");
//
//  else
//    Serial.println("Log removed.");

snprintf(fileName, sizeof(fileName), "log-%03i.txt", logNum);
while(SD.exists(fileName))
{
  logNum++;
  snprintf(fileName, sizeof(fileName), "log-%03i.txt", logNum);
}

  logFile = SD.open(fileName, FILE_WRITE);
  if (logFile)
  {
    logFile.println("Time [s]\tAmp [A]");
    logFile.close();
  }
}

void loop() 
{
  if((digitalRead(buttonPin)==LOW))
  {
    if(micros() - sendTimer2000Hz > 500)
    {
      sendTimer2000Hz = micros();
      digitalWrite(ledPin, HIGH);
  
      seconds = (double)micros() / (double)1000000;
      Vcc = (analogRead(A4) * 33000 / 8191) / 2200.0000 * 3400.0000;
      Vout = (Vcc - 24400) / 400;
  
      //Serial.print("Time: ");
      Serial.print(seconds, 6);
      //Serial.print(", Vcc: ");
      //Serial.print(Vcc);
      //Serial.print(", Vout: ");
      Serial.print("  ");
      Serial.println(Vout);
      
      //writeToFile(seconds, Vout);
      
      digitalWrite(ledPin, LOW);
    }
  }
}

void writeToFile(double seconds, double voltage)
{
  logFile = SD.open(fileName, FILE_WRITE);
  if (logFile) 
  {
    logFile.print(seconds, 6);
    logFile.print('\t');
    logFile.println(voltage);
    logFile.close();
  } 

  else 
  {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
