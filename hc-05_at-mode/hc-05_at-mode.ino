/*

AUTHOR: Hazim Bitar (techbitar)
DATE: Aug 29, 2013
LICENSE: Public domain (use at your own risk)
CONTACT: techbitar at gmail dot com (techbitar.com)

*/


#include <SoftwareSerial.h>

SoftwareSerial BTSerial(A4, A5); // RX | TX
int counter = 0;
bool sendReady = false;
int iterator = 0;

void setup()
{
  pinMode(9, OUTPUT);  // this pin will pull the HC-05 pin 34 (key pin) HIGH to switch module to AT mode
  digitalWrite(9, HIGH);
  Serial.begin(9600);
  Serial.println("Enter AT commands:");
  BTSerial.begin(38400);  // HC-05 default speed in AT command more
}

void loop()
{
  int beginTime = millis();
  // Keep reading from HC-05 and send to Arduino Serial Monitor
  //delay(1);
  //for (int i = 0; i <= 4; i++)
  while (BTSerial.available())
  {
    int data = BTSerial.read();
    

    if (data == 1243)
    {
      //Serial.write(data);
      sendReady = 1;
      Serial.println("Send Ready!");
    }

    else
    {
      
    }
    
  }

  //Serial.println("Exited while loop");

  if (sendReady = true)
  {
    
    sendReady = 0;
    Serial.print("Sending Data: #");
    Serial.print(iterator);
    Serial.print(",2,3,4,5,6,7");
    Serial.println();
    
    
    BTSerial.print("#");
    BTSerial.print(iterator);
    BTSerial.print(",2,3,4,5,6,7");
    BTSerial.println();
    iterator++;
  }

  // Keep reading from Arduino Serial Monitor and send to HC-05
  if (Serial.available())
  {
    BTSerial.write(Serial.read());
  }
    int endTime = millis();
    //Serial.print("Loop Duration: ");
    //Serial.println(endTime-beginTime);
} 
