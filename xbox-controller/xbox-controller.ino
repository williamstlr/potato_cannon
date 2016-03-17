#include <SoftwareSerial.h>

SoftwareSerial BTSerial(10, 11); // RX | TX


int cannonReady = 0;
char currentChar = ' ';

//Pin Setup
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
int  writeValuesToSerialDebug  =  0; //doesn't work when the wire-write is enabled
int calibrateThumbSticks       =  0;
bool serialPrintValues  = true;

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

//Delete These
int iterator = 0;


void setup()
{
  //Starts serial ports
  Serial.begin(9600);
  BTSerial.begin(38400);  

  //Sets pin modes
  pinMode(thumbstickRightHorizontalPin,INPUT);
  pinMode(thumbstickRightVerticalPin,INPUT);
  pinMode(triggerRight,INPUT);
  pinMode(aPin,INPUT_PULLUP);
  pinMode(bPin,INPUT_PULLUP);
  pinMode(yPin,INPUT_PULLUP);
  pinMode(xPin,INPUT_PULLUP);
  pinMode(12,OUTPUT);

  //Sets all of the readings to zero
  for (int i = 0; i < numReadings; i++)
  {
    hReadings[i] = 0;
    vReadings[i] = 0;
  }

  //Serial.println("sendReady= " + sendReady);
}//end setup


void loop()
{
  //Turns the green status light on controller on
  digitalWrite(12,HIGH); 

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

  //Serial.print("BTSerial avaible: ");
  //Serial.println(BTSerial.available());
  while (BTSerial.available())
  {
    //Serial.println("BTSerial available: " + BTSerial.available());
    int data = 0;
    currentChar = BTSerial.read();
    Serial.write(currentChar);

    if (currentChar == '$')
    {
      cannonReady = 1;
      Serial.println("\nSend Ready!");
    }//if


    else
    {
      BTSerial.read();
      cannonReady = 0;
    }
    delay(1);
  }//while
  
  if (cannonReady == 1)
  {
    cannonReady = 0;
    Serial.println("Running cannonReady loop");
    
    BTSerial.print("#");
    BTSerial.print(thumbstickRightHorizontalData);
    BTSerial.print(",");
    BTSerial.print(thumbstickRightVerticalData);
    BTSerial.print(",");
    BTSerial.print(rightTriggerData);
    BTSerial.print(",");
    BTSerial.print(a);
    BTSerial.print(",");
    BTSerial.print(b);
    BTSerial.print(",");
    BTSerial.print(y);
    BTSerial.print(",");
    BTSerial.print(x);
    BTSerial.println();

    if (serialPrintValues == true)
    {
      Serial.print("#");
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

  }//end sendReady if

  if(calibrateThumbSticks == 1)
  {
    Serial.print(analogRead(A0));
    Serial.print(",");
    Serial.print(thumbstickRightHorizontalData);
    Serial.println();
  }
}//end loop

