#include <Wire.h>
#include <LiquidCrystal595.h>    // include the shiftregister LCD library

////I2C stuff
//#define i2c_data 7
int i2c_data_length = 0;
byte data[24];    //These are bytes because I2C works with individual bytes per transaction
byte data_index;
byte state;
int i2cDetected =0;

#define i2c_write_data 2    //Number of bytes that I want to send
byte writeData[i2c_write_data];
byte writeDataIndex;
byte mappedPanLocation;    //Bytes to work with I2C
byte mappedTiltLocation;


//Motor Enable Pins
/*
Motor Pin Connections 
 IN1 and IN2 go to VCC - These need to be constantly HIGH
 EN1 and IN2 go to motorPinLeft and motorPinRight
 VCC and GRND go where you think they would
 */

//Settings that can be changed
int horizontalMaxSpeed    =  75;
int verticalMaxSpeed      =  255;
long fanRunTime      =  10000;  //How long to run the fan for after the button is pressed
float fuelFillTime    =  2000;  //How long to keep the fuel valve open after the button is pressed.
int leftTurnMax      =  850;   //How far the cannon can turn left
int rightTurnMax     =  150;   //How far the cannon can turn right
int tiltDownMax      =  875;
int tiltUpMax        =  530;

//Initialize Pin Values
int fuelPin           =  13;
int fanPin            =  12;
int motorPinLeft      =  10;
int motorPinRight     =  9;
int motorPinUp        =  6;
int motorPinDown      =  5;
int buzzerPin         =  3;
int ignitionOutput    =  2;
int sensorPinPan      =  A3;  //Horizontal Potentiometer on the underside of the base
int sensorPinTilt     =  A2;

//LCD pins
int lcdClockPin    =  11;
int lcdLatchPin    =  7;
int lcdDataPin     =  8;
LiquidCrystal595 lcd(lcdDataPin,lcdLatchPin,lcdClockPin);     // datapin, latchpin, clockpin


//Variables for menu
int menuCurrentScreen    =  0;
int menuNumberOfScreens  =  7;

//Some variables for keeping track of timers
long previousFanMillis  =  0;
unsigned long currentFanMillis  =  0;
long previousFuelMillis  =  0;
unsigned long currentFuelMillis  =  fuelFillTime;
int fanTimerRunning  =  0;
int fuelTimerRunning =  0;
int propaneActive    =  0;


//Divides horizontal and vertical in half.
int horizontalHalf        =  horizontalMaxSpeed/2;
int verticalHalf          =  verticalMaxSpeed/2;

//Initializing some variables
int horizontalMotion      =  0;
int verticalMotion        =  0;
int fire                  =  0;
int a                     =  1;  //Buttons have inverted logic so 1 is not pressed, 0 is pressed.
int b                     =  1;
int y                     =  1;
int x                     =  1;
int panLocation           =  500;
int tiltLocation          =  900; //set at 900 so things don't freak out when it starts out at 0

//button edge detection
int buttonPushCounterY    =  0;   
int buttonStateY          =  0;    
int lastButtonStateY      =  0;    
int buttonPushCounterB    =  0;   
int buttonStateB          =  0;    
int lastButtonStateB      =  0;  

//When horizontalMaxSpeed is 255 horiz (I'm not sure what I meant by that)
int horizontalNoiseThreshold  =  horizontalMaxSpeed*.31;
int verticalNoiseThreshold    =  80;

#define sendDataNumber 1
int  sendDataPosition = 0;
int  sendDataBuffer[sendDataNumber];

//Angle calculation variables
int verticalAngle = 0;
int horizontalAngle = 0;


//debugging
int printMovementDirection  =  0;    //1 for on, 0 for off
int controllerInputDebug    =  0;
int printXYLocation         =  0;
int printBounceForce        =  0;
int wirePrintReceived       =  1;

void setup()
{
  //Communication setup
  Wire.begin();                // join i2c bus as the master
  Wire.onReceive(receiveEvent); // register event                  **Remove me** No longer needed now that this is the I2C Master
  Wire.onRequest(sendData);                                        //This probably isn't needed either
  Serial.begin(9600);
  Serial.println("Serial connection established");
  Serial.println("waiting for data...");

  //Pin setup
  pinMode(sensorPinPan,INPUT);
  pinMode(fuelPin,OUTPUT);
  pinMode(fanPin,OUTPUT);
  //pinMode(sensorPinTilt,INPUT);    //Why is this commented out?
  pinMode(motorPinLeft,OUTPUT);
  pinMode(motorPinRight,OUTPUT);
  pinMode(motorPinUp,OUTPUT);
  pinMode(motorPinDown,OUTPUT);
  pinMode(ignitionOutput,OUTPUT);
  
  //LCD setup
  lcd.begin(16,2);
 
  
}

void loop()
{
  delay(10);
  
  //This can probably be moved to it's own function                    *Change me*
  panLocation       =  analogRead(sensorPinPan);   //Get pan value
  tiltLocation      =  analogRead(sensorPinTilt);  //Get tilt value
  
  mappedPanLocation   = constrain(map(panLocation,rightTurnMax,leftTurnMax,0,127),0,127);  //Do some math to get the pan value within an acceptable range, it should be
  mappedTiltLocation  = constrain(map(tiltLocation,tiltUpMax,tiltDownMax,0,127),0,127);    //a value 0-255 so that it will fit in an I2C transaction
  
  //Fill I2C send buffer
  sendDataBuffer[0] = mappedPanLocation;
  sendDataBuffer[1] = mappedTiltLocation;
  
  //Request 7-bytes from device 1 on the I2C bus
  Wire.requestFrom(1,7);
  i2c_data_length = Wire.available(); //This determines the number of bytes coming down the I2C pipe, needed
  
  while(Wire.available())
  {
    
   
   //int someData = Wire.read();
   //Serial.println("Something is coming through");                         *Remove me*
   //Serial.println(someData);
   
   
   data[data_index++] = Wire.read();

    if(data_index >= i2c_data_length)
    {
      data_index = 0;

      horizontalMotion    =  ((map(data[0],0,255,0,horizontalMaxSpeed))-horizontalHalf)*-2;
      verticalMotion      =  constrain((((data[1])-verticalHalf)*2),-255,255);
      fire              =  data[2];
      a                 =  data[3];
      b                 =  data[4];
      y                 =  data[5];
      x                 =  data[6];
      
      //Filters out some controller noise
      if (verticalMotion >= -100 && verticalMotion <= 100)    
      {
        verticalMotion = 0;
      };
      
      //Filters out some controller noise
      if (horizontalMotion >= -20 && horizontalMotion <=20)  
      {
        horizontalMotion = 0;
      };
      
    }//end data_index if
    
  }//end while wire.available

  miscDebugging();
  
  
  if (i2cDetected == 0)                            //*This needs work*
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("I2C Connection");
    lcd.setCursor(0,1);
    lcd.print("Not Detected");
  }  
  if (i2cDetected == 1)
  {
    lcdMenu();
  }


  
 
  

  ////////////////////Debugging////////////////////                                  **TODO, move all debugging functions to another file**
  //////////////Uncomment to Enable////////////////

  //Print running timers for the Fuel
  //printFuelTimerDebug();          //Appears to be working

  //Print running timers for the Fan
  //printFanTimerDebug();           //Appears to be working

  //Print the trigger for the fan timer
  //printFanTimerTriggerDebug();    //Appears to be working

  //Print triggers for fuel timer
  //printFuelTimerTriggerDebug();   //Appears to be working
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}//End of loop()
void receiveEvent(int howMany) //This is what runs when the I2C connection is UP  *Remove this whole function once everything is migrated to loop()*
{
  readInputs();
  
  /*
  keepOutputPinsLow();                                                **TODO, move all these into loop()**
  overRotationBounce();
  moveCannon();
  watchFire();
  watchFuelButton();
  watchFanButton();
  warningBuzzer();
  fanTimer();
  fuelTimer();
  i2cDetected = 1;
  */
  
  
  //(0-100)(-1)
  //(100-100)(-1)
  
  //int invertVertical = (map(panLocation,tiltDownMax,tiltUpMax,0,100)-100)-1;
  //verticalAngle = map(verticalAngle,0,100,36,60);
  //horizontalAngle = 0;
  //Serial.print(invertVertical);
  
  
}// End of receiveEvent loop

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////FUNCTIONS////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printFanTimerDebug()
{

  Serial.print(currentFanMillis);
  Serial.print('-');
  Serial.print(previousFanMillis);
  Serial.print('=');
  Serial.print(currentFanMillis - previousFanMillis);
  Serial.println();

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//print fanTimer trigger
void printFanTimerTriggerDebug()
{
  Serial.print("x = ");
  Serial.print(x);
  Serial.print(" fanTimerRunning= ");
  Serial.print(fanTimerRunning);
  Serial.println();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void fanTimer()
{

  currentFanMillis  =  millis();
  if(fanTimerRunning == 1)
  {
    digitalWrite(fanPin,HIGH);

    if(printMovementDirection == 1)
    {
      Serial.println("Fan On");
    }

    if(currentFanMillis - previousFanMillis > fanRunTime)
    {
      previousFanMillis  =  currentFanMillis;
      fanTimerRunning   =  0;
    } 
  }

  if(fanTimerRunning == 0)
  {
    digitalWrite(fanPin,LOW);
    previousFanMillis  =  currentFanMillis;
  }

  /* else
   {
   digitalWrite(fanPin,LOW);
   }
   */
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Fuel Timer
void fuelTimer()
{

  currentFuelMillis  =  millis();
  if(fuelTimerRunning == 1)
  {
    digitalWrite(fuelPin,HIGH);
    //////////////////////////////////
    if(printMovementDirection == 1)  //debugging
    {
      Serial.println("Valve Open");
    }
    //////////////////////////////////
    if(currentFuelMillis - previousFuelMillis > fuelFillTime)
    {
      previousFuelMillis  =  currentFuelMillis;
      fuelTimerRunning   =  0;
    } 
    //tone(buzzerPin, 400);
  }

  if(fuelTimerRunning == 0)
  {

    digitalWrite(fuelPin,LOW);
    previousFuelMillis  =  currentFuelMillis;
    //noTone(buzzerPin);

  }
  /*
  else
   {
   digitalWrite(fuelPin,LOW);
   }
   */
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//print fuelTimer trigger
void printFuelTimerTriggerDebug()
{
  Serial.print("a = ");
  Serial.print(a);
  Serial.print(" fuelTimerRunning= ");
  Serial.print(fuelTimerRunning);
  Serial.println();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void printFuelTimerDebug()
{

  Serial.print(currentFuelMillis);
  Serial.print('-');
  Serial.print(previousFuelMillis);
  Serial.print('=');
  Serial.print(currentFuelMillis - previousFuelMillis);
  Serial.println();

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Read Inputs
void readInputs()
{
  i2c_data_length = Wire.available();
  //Serial.println(i2c_data_length);
  while(Wire.available())
  {
   
    data[data_index++] = Wire.read();

    if(data_index >= i2c_data_length)
    {
      data_index = 0;

      horizontalMotion    =  ((map(data[0],0,255,0,horizontalMaxSpeed))-horizontalHalf)*-2;
      verticalMotion      =  constrain((((data[1])-verticalHalf)*2),-255,255);
      fire              =  data[2];
      a                 =  data[3];
      b                 =  data[4];
      y                 =  data[5];
      x                 =  data[6];
      
      
      if (verticalMotion >= -100 && verticalMotion <= 100)
      {
        verticalMotion = 0;
      };
      
      
      
      if (horizontalMotion >= -20 && horizontalMotion <=20)
      {
        horizontalMotion = 0;
      };
    }//end data_index if

  }//End wire.available while loop
  
}//end readInputs

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void keepOutputPinsLow()
{

  //Keep pins set to LOW so that it doesn't keep going indefinitely, this may not be nessecary.
  digitalWrite(motorPinLeft,LOW);
  digitalWrite(motorPinRight,LOW);
  digitalWrite(motorPinUp,LOW);
  digitalWrite(motorPinDown,LOW);
  digitalWrite(ignitionOutput,LOW);

}

void moveCannon()
{

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  

  //Turn Left
  if (horizontalMotion <-(horizontalNoiseThreshold) & panLocation < leftTurnMax)
  {
    analogWrite(motorPinLeft,abs(horizontalMotion));
    if (printMovementDirection == 1)
    {
      Serial.println("Left");
    }

  }
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Turn Right
  if (horizontalMotion > horizontalNoiseThreshold & panLocation > rightTurnMax)
  {
    analogWrite(motorPinRight,abs(horizontalMotion));
    if(printMovementDirection == 1)
    {
      Serial.println("Right");
    }
  }
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Move Down
  if (verticalMotion < -(verticalNoiseThreshold)  & tiltLocation > tiltUpMax)
  {
    analogWrite(motorPinDown,abs(verticalMotion));
    if(printMovementDirection == 1)
    {
      Serial.println("Down");
    }
  }
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Move Up
  if (verticalMotion > verticalNoiseThreshold & tiltLocation < tiltDownMax)
  {
    analogWrite(motorPinUp,abs(verticalMotion));
    if(printMovementDirection == 1)
    {
      Serial.println("Up");
    }
  }
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void overRotationBounce()
{

  //Over-rotation bounce



    int printBounceForceLeft    = -(horizontalMaxSpeed - (map(panLocation,150,850,0,horizontalMaxSpeed))) + 35;
  int printBounceForceRight   = -(map(panLocation,150,850,0,horizontalMaxSpeed)) + 35;




  if (panLocation > leftTurnMax)
  {
    analogWrite(motorPinRight,printBounceForceLeft);
    if (printBounceForce == 1)
    { 
      String bounceLeftMessage = "Bounce force left = ";
      Serial.println(bounceLeftMessage + printBounceForceLeft);
    }

  }

  if (panLocation < rightTurnMax)
  {
    analogWrite(motorPinLeft,printBounceForceRight);
    if (printBounceForce == 1)
    { 
      String bounceRightMessage = "Bounce force right = ";
      Serial.println(bounceRightMessage + printBounceForceRight);
    }
  }


}

void watchFire()
{

  //Watch for trigger pull
  if (fire < 80)
  {
    digitalWrite(ignitionOutput,HIGH);
    //Serial.println("Fire!");
    propaneActive = 0;
  }

}

void watchFuelButton()
{

  //Propane Fill
  if(a == 0) //(logic inverted)
  {
    fuelTimerRunning  =  1;
    propaneActive = 1;
  }

}

void watchFanButton()
{

  //Blow out the combustion chamber 
  if(x == 0) //(logic inverted)
  {
    fanTimerRunning  =  1;     
  }

}

void miscDebugging()
{

  //Debugging
  if (controllerInputDebug == 1)
  {
    String horizontalMessage = "Horizontal Motion = ";
    String verticalMessage    =" Vertical Motion = ";
    Serial.println(horizontalMessage + horizontalMotion + verticalMessage + verticalMotion);
    //Serial.println(verticalMessage + verticalMotion);
  }
  //Serial.println(analogRead(controllerPinHorizontal));
  if (printXYLocation == 1)
  {
    Serial.print("X = ");
    Serial.print(panLocation);
    Serial.print(" Y = ");
    Serial.print(tiltLocation);
    Serial.println();
  }

  if(wirePrintReceived ==1)
  {
    Serial.print(horizontalMotion); 
    Serial.print(',');
    Serial.print(verticalMotion);
    Serial.print(',');
    Serial.print(fire);
    Serial.print(',');
    Serial.print(a);
    Serial.print(',');
    Serial.print(b);
    Serial.print(',');
    Serial.print(y);
    Serial.print(',');
    Serial.print(x);
    Serial.println();
  }

}

void warningBuzzer()
{
  if (propaneActive ==1)
  {
    tone(buzzerPin,400);
  }

  else
  {
    noTone(buzzerPin);
  }
}

void sendData()
{
  
   if (sendDataPosition > sendDataNumber)
  {
    sendDataPosition = 0;
  }
  
  Wire.write(sendDataBuffer[sendDataPosition++]);
 
 
  
}

void lcdMenu()
{
  
  // read the pushbutton input pin:
  buttonStateY = y;

  // compare the buttonState to its previous state
  if (buttonStateY != lastButtonStateY) {
    // if the state has changed, increment the counter
    if (buttonStateY == HIGH) {

      buttonPushCounterY++;
    }
    else {
      menuCurrentScreen++;

    }
  }
  // save the current state as the last state,
  //for next time through the loop
  lastButtonStateY = buttonStateY;

 
  
  //If button is pressed, advance to the next screen
  /*if (y == 0)
  {
    menuCurrentScreen = menuCurrentScreen++;
  }
  */
  //return to the first screen
  if (menuCurrentScreen > menuNumberOfScreens)
  {
    menuCurrentScreen = 0;
  }
  
  
    if (menuCurrentScreen == 0)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("A=Fuel X=Fan");
    lcd.setCursor(0,1);
    lcd.print("Y=Next B=Change");
  }//End 0
  
  
   if (menuCurrentScreen == 1)
  {
 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Fuel Timer");
      lcd.setCursor(0,1);
      lcd.print(fuelFillTime/1000);
      lcd.print(" seconds");
  
      if(menuCurrentScreen == 1)
     { 
          // read the pushbutton input pin:
          buttonStateB = b;
        
          // compare the buttonState to its previous state
          if (buttonStateB != lastButtonStateB) 
          {
           
            if (buttonStateB == HIGH) {
              
              buttonPushCounterB++; 
            }
            else {
             
              Serial.println("off");
              fuelFillTime = fuelFillTime + 500;
              
              if (fuelFillTime > 6000)
              {
                fuelFillTime = 1000;
              }
            }
          }//end button state compare
  
        lastButtonStateB = buttonStateB;
     }//end if for button B
  }//End 1  
  
  //Shows controller inputs
  if (menuCurrentScreen == 2)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Input X = ");
    lcd.print(horizontalMotion);
    lcd.setCursor(0,1);
    lcd.print("Input Y = ");
    lcd.print(verticalMotion);
  }//End 2
  
  
  if (menuCurrentScreen == 3)
  {
 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("A = ");
  lcd.print(a);
  lcd.print(" B = ");
  lcd.print(b);
  lcd.setCursor(0,1);
  lcd.print("Y = ");
  lcd.print(y);
  lcd.print(" X = ");
  lcd.print(x);
 
  }//End 3
  
  
  
  if (menuCurrentScreen == 4)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Pan = ");
    lcd.print(panLocation);
    lcd.setCursor(0,1);
    lcd.print("Tilt = ");
    lcd.print(tiltLocation); 
  }//End 4
  
 
if (menuCurrentScreen == 5)
  {
 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Uptime");
  lcd.setCursor(0,1);
  lcd.print(millis()/1000);
 
  }//end menu 5
  
  if (menuCurrentScreen == 6)
  {
 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Horiz. Strength");
  lcd.setCursor(0,1);
  lcd.print(horizontalMaxSpeed);
  //lcd.print("Fix Me");
  
  if (menuCurrentScreen == 6)
  {
       // read the pushbutton input pin:
          buttonStateB = b;
        
          // compare the buttonState to its previous state
          if (buttonStateB != lastButtonStateB) {
           
            if (buttonStateB == HIGH) {
             
              buttonPushCounterB++; 
            }
            else {
             
              Serial.println("off");
              horizontalMaxSpeed = horizontalMaxSpeed + 5;
              horizontalHalf        =  horizontalMaxSpeed/2;

              
              if (horizontalMaxSpeed > 75)
              {
                horizontalMaxSpeed = 50;
              }
            }
      }
      
      lastButtonStateB = buttonStateB;
  }//end buttonB "if"
  
 
  }//end menu 6
  
  if (menuCurrentScreen == 7)
  {
 
  
  //panLocation
  
  
  //tiltLocation
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("V angle = ");
  lcd.print(verticalAngle);
  //lcd.print(" degrees");
  lcd.setCursor(0,1);
  lcd.print("H angle = ");
  lcd.print(horizontalAngle);
  //lcd.print(" degrees");
 
  }//end menu 7
  
  
  
}//end LCD Menu


