#include <Wire.h>

int thumbstickRightHorizontalPin    =  A0;  //White
int thumbstickRightVerticalPin      =  A1;  //Yellow
int triggerRight                    =  A2;  //Green
int  aPin                           =  5;
int  bPin                           =  4;
int  yPin                           =  3;
int  xPin                           =  2;

//Max speed for horizontal and vertical movement
int horizontalMaxSpeed     =  255;
int horizontalHalf    =  horizontalMaxSpeed/2;
int verticalMaxSpeed       =  255;
int verticalHalf      =  verticalMaxSpeed/2;

//Debugging options. Set to 1 to enable
int  writeValuesToSerialDebug  =  1; //doesn't work when the wire-write is enabled
int calibrateThumbSticks       =  0;

//Initialize variables for data to be sent. The are bytes because i2c specifcally requests bytes.
//if an int data type is used, two bytes will be sent for every value and it will really screw
//with the receiving end of the i2c transaction
int thumbstickRightHorizontalData  =  0;
int thumbstickRightVerticalData    =  0;
int rightTriggerData               =  0;
int a  =  0;
int b  =  0;
int y  =  0;
int x  =  0;

//Some setup for input smoothing
const int numReadings = 10;
int hReadings[numReadings];
int vReadings[numReadings];
int readIndex = 0;
int hTotal = 0;
int vTotal = 0;
int hAverage = 0;
int vAverage = 0;
  
void setup()
{
  pinMode(thumbstickRightHorizontalPin,INPUT);
  pinMode(thumbstickRightVerticalPin,INPUT);
  pinMode(triggerRight,INPUT);
  pinMode(aPin,INPUT_PULLUP);
  pinMode(bPin,INPUT_PULLUP);
  pinMode(yPin,INPUT_PULLUP);
  pinMode(xPin,INPUT_PULLUP);
  pinMode(12,OUTPUT);
  Serial.begin(250000);
  Serial.println("Serial connection established");
  Serial.println("Send '1243' to receive values");

//Sets all of the readings to zero
  for (int i = 0; i < numReadings; i++)
  {
    hReadings[i] = 0;
    vReadings[i] = 0;
  }
}


void loop()
{
  //delay(10);

  //Calculate the average of the analog thumbsticks
  hTotal = hTotal - hReadings[readIndex];
  vTotal = vTotal - vReadings[readIndex];
  hReadings[readIndex] = analogRead(thumbstickRightHorizontalPin);
  vReadings[readIndex] = analogRead(thumbstickRightVerticalPin);
  hTotal = hTotal + hReadings[readIndex];
  vTotal = vTotal + vReadings[readIndex];
  readIndex++;

  if (readIndex >= numReadings)
  {
    readIndex = 0;
  }

  hAverage = hTotal / numReadings;
  vAverage = vTotal / numReadings;

  //Turns the green status light on controller on
  digitalWrite(12,HIGH); 

  //Reads the data from the sensors
  //thumbstickRightHorizontalData  =  constrain(map(analogRead(thumbstickRightHorizontalPin),200,850,0,horizontalMaxSpeed),0,horizontalMaxSpeed);
  //thumbstickRightVerticalData    =  constrain(map(analogRead(thumbstickRightVerticalPin),210,870,0,verticalMaxSpeed),0,verticalMaxSpeed);
  thumbstickRightHorizontalData  =  constrain(map(hAverage,200,850,0,horizontalMaxSpeed),0,horizontalMaxSpeed);
  thumbstickRightVerticalData    =  constrain(map(vAverage,210,870,0,verticalMaxSpeed),0,verticalMaxSpeed);
  rightTriggerData               =  constrain(map(analogRead(triggerRight),130,810,0,255),0,255);
  a  =  digitalRead(aPin);
  b  =  digitalRead(bPin);
  y  =  digitalRead(yPin);
  x  =  digitalRead(xPin);
  

  //Waits until it receives 1243 and then it will send it's values back
  while (Serial.available() > 0)
    {
    if (Serial.parseInt() == 1243)
    {
      Serial.print(thumbstickRightHorizontalData);
      Serial.print(",");
      Serial.print(thumbstickRightVerticalData);
      Serial.print(",");
      Serial.print(rightTriggerData);
      Serial.print(",");
      Serial.print(a);
      Serial.print(",");
      Serial.print(b);
      Serial.print(",");
      Serial.print(y);
      Serial.print(",");
      Serial.print(x);
      Serial.println();
    }

    else
    {
      break;
    }
  }
  if(calibrateThumbSticks == 1)
  {
    Serial.print(analogRead(A0));
    Serial.print(",");
    Serial.print(thumbstickRightHorizontalData);
    Serial.println();
  }
  
}//end of main loop
