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
  
  int  writeToWire               =  0;
  int  writeValuesToSerialDebug  =  1; //doesn't work when the wire-write is enabled
  int calibrateThumbSticks       =  0;
  
  //Initialize variables for data to be sent. The are bytes because i2c specifcally requests bytes.
  //if an int data type is used, two bytes will be sent for every value and it will really screw
  //with the receiving end of the i2c transaction
  byte thumbstickRightHorizontalData  =  0;
  byte thumbstickRightVerticalData    =  0;
  byte rightTriggerData               =  0;
  
  byte a  =  0;
  byte b  =  0;
  byte y  =  0;
  byte x  =  0;

void setup()
{
  Wire.begin(1); // join i2c bus (address optional for master)
  pinMode(thumbstickRightHorizontalPin,INPUT);
  pinMode(thumbstickRightVerticalPin,INPUT);
  pinMode(triggerRight,INPUT);
  pinMode(aPin,INPUT_PULLUP);
  pinMode(bPin,INPUT_PULLUP);
  pinMode(yPin,INPUT_PULLUP);
  pinMode(xPin,INPUT_PULLUP);
  pinMode(12,OUTPUT);
  Serial.begin(9600);
  Serial.println("Serial connection established");
  Serial.println("waiting for data...");
  Wire.onRequest(sendData);
}

void sendData()
{
    //Wire.beginTransmission(); // transmit to device #4
    //Serial.println("Writing to i2c bus...");
    Wire.write(thumbstickRightHorizontalData);              // sends one byte  
    Wire.write(thumbstickRightVerticalData);
    Wire.write(rightTriggerData);
    Wire.write(a);
    Wire.write(b);
    Wire.write(y);
    Wire.write(x);
    //Serial.println("Finished");
    //Wire.endTransmission();    // stop transmitting
}


void loop()
{
  delay(100);
  digitalWrite(12,HIGH); //Turns the green status light on controller on

  thumbstickRightHorizontalData  =  constrain(map(analogRead(thumbstickRightHorizontalPin),200,850,0,horizontalMaxSpeed),0,horizontalMaxSpeed);
  thumbstickRightVerticalData    =  constrain(map(analogRead(thumbstickRightVerticalPin),210,870,0,verticalMaxSpeed),0,verticalMaxSpeed);
  rightTriggerData               =  constrain(map(analogRead(triggerRight),130,810,0,255),0,255);
  
  a  =  digitalRead(aPin);
  b  =  digitalRead(bPin);
  y  =  digitalRead(yPin);
  x  =  digitalRead(xPin);
  
  
  /*
  //Write values to the Wire line
  if(writeToWire == 1)
  {
    Wire.beginTransmission(4); // transmit to device #4
    Wire.write(thumbstickRightHorizontalData);              // sends one byte  
    Wire.write(thumbstickRightVerticalData);
    Wire.write(rightTriggerData);
    Wire.write(a);
    Wire.write(b);
    Wire.write(y);
    Wire.write(x);
    Wire.endTransmission();    // stop transmitting
  }
  */
  if(writeValuesToSerialDebug == 1)
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
  
  if(calibrateThumbSticks == 1)
  {
    Serial.print(analogRead(A0));
    Serial.print(",");
    Serial.print(thumbstickRightHorizontalData);
    Serial.println();
  }

  //x++;
  
}
