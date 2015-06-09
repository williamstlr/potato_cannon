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
  
  int  writeToWire               =  1;
  int  writeValuesToSerialDebug  =  0; //doesn't work when the wire-write is enabled
  int calibrateThumbSticks       =  0;

void setup()
{
  Wire.begin(); // join i2c bus (address optional for master)
  pinMode(thumbstickRightHorizontalPin,INPUT);
  pinMode(thumbstickRightVerticalPin,INPUT);
  pinMode(triggerRight,INPUT);
  pinMode(aPin,INPUT_PULLUP);
  pinMode(bPin,INPUT_PULLUP);
  pinMode(yPin,INPUT_PULLUP);
  pinMode(xPin,INPUT_PULLUP);
  pinMode(12,OUTPUT);
  Serial.begin(9600);
}


void loop()
{
  delay(200);
  digitalWrite(12,HIGH);
  /*int horizontalMaxSpeed    =  255;
  int horizontalHalf        =  horizontalMaxSpeed/2;
  int verticalMaxSpeed      =  255;
  int verticalHalf          =  verticalMaxSpeed/2;
  */
  int thumbstickRightHorizontalData  =  constrain(map(analogRead(thumbstickRightHorizontalPin),200,850,0,horizontalMaxSpeed),0,horizontalMaxSpeed);
  int thumbstickRightVerticalData    =  constrain(map(analogRead(thumbstickRightVerticalPin),210,870,0,verticalMaxSpeed),0,verticalMaxSpeed);
  int rightTriggerData               =  constrain(map(analogRead(triggerRight),130,810,0,255),0,255);
  
  int a  =  digitalRead(aPin);
  int b  =  digitalRead(bPin);
  int y  =  digitalRead(yPin);
  int x  =  digitalRead(xPin);
  
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
