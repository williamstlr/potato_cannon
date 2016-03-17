/*
  Code for controlling a potato cannon with an arduino.
  Written by Tyler Williams, June 2015
  Code released to the public domain
*/


#include <LiquidCrystal595.h>    // include the shiftregister LCD library
#include <SoftwareSerial.h>


SoftwareSerial BTSerial(A4,A5);

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
long fuelFillTime    =  2000;  //How long to keep the fuel valve open after the button is pressed.
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
int menuLastScreen       =  0;
int menuNumberOfScreens  =  6;

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
int horizontalMotionRaw   =  0;
int verticalMotionRaw     =  0;
int mappedPanLocation; 
int mappedTiltLocation;

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

//debugging
int printMovementDirection  =  0;    //1 for on, 0 for off
int controllerInputDebug    =  0;
int printXYLocation         =  0;
int printBounceForce        =  0;
int wirePrintReceived       =  0;
int printRawValues          =  1;
int printNormalizedValues   =  0;


void setup()
{
  //Communication setup
  Serial.begin(9600);
  BTSerial.begin(9600);  
  Serial.println("Serial connection established");
  Serial.println("Controller should send '#' followed immediately by comma-delmited values");
  Serial.println("Example #1,2,3,4,5,6,7");

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

  
}//End setup()

void loop()
{
  long beginTime = millis();
  delay(1);
  
  readAndMapPanAndTilt();    //Needs to be before overRotationBounce()
  readSerial();
  normalizeMotionValues();
  //sendPosition();

  miscDebugging();
  lcdMenu();
  keepOutputPinsLow();                                              
  overRotationBounce();
  moveCannon();
  watchFire();
  watchFuelButton();
  watchFanButton();
  warningBuzzer();
  fanTimer();
  fuelTimer();
  

/*
  long endTime = millis();

  long totalTime = endTime - beginTime;
  Serial.print(beginTime);
  Serial.print('-');
  Serial.print(endTime);
  Serial.print('=');
  Serial.print(totalTime);
  Serial.print(" i=");
  Serial.print(i);
  i = i+1;
  Serial.println();
*/ 
  

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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////FUNCTIONS////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void readSerial()
{
      //Controller waits until it receives '1243' to send the values, hopefully this will keep things more in sync and prevent the serial line from filling up.
      BTSerial.print('$');
      //Serial.println("$");
      delay(50); //may or may not be needed. Try some testing with this.
      
      while (BTSerial.available())
      {
        //Serial.println(BTSerial.read());
        
        if (BTSerial.read()   == '#')
        {
          horizontalMotionRaw =  BTSerial.parseInt();
          verticalMotionRaw   =  BTSerial.parseInt();
          fire                =  BTSerial.parseInt();
          a                   =  BTSerial.parseInt();
          b                   =  BTSerial.parseInt();
          y                   =  BTSerial.parseInt();
          x                   =  BTSerial.parseInt();
          char delimiter      =  BTSerial.read();
          delay(5);
        }//end if

        else
        {
          BTSerial.read();
        }
        
      }//end while
      
}

void normalizeMotionValues()
{
  horizontalMotion    =  ((map(horizontalMotionRaw,0,255,0,horizontalMaxSpeed))-horizontalHalf)*-2;
  verticalMotion      =  constrain((((verticalMotionRaw)-verticalHalf)*2),-255,255);
}

void readAndMapPanAndTilt()
{
  panLocation       =  analogRead(sensorPinPan);   //Get pan value
  tiltLocation      =  analogRead(sensorPinTilt);  //Get tilt value
  
  mappedPanLocation   = constrain(map(panLocation,rightTurnMax,leftTurnMax,0,127),0,127);  //Do some math to get the pan value within an acceptable range, it should be
  mappedTiltLocation  = constrain(map(tiltLocation,tiltUpMax,tiltDownMax,0,127),0,127);    //a value 0-255 so that it will fit in an I2C transaction 
}

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


}//ends fanTimer
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

  horizontalMotionRaw =  0;
  verticalMotionRaw   =  0;
  fire                =  0;
  a                   =  1;
  b                   =  1;
  y                   =  1;
  x                   =  1;
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
    Serial.print("controllerInputDebug debugging is active: ");
    String horizontalMessage = "Horizontal Motion = ";
    String verticalMessage    =" Vertical Motion = ";
    Serial.println(horizontalMessage + horizontalMotion + verticalMessage + verticalMotion);
  }
  
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
    Serial.print("wirePrintReceived debugging active: ");
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

  if(printRawValues ==1)
  {
    Serial.print("Raw values: ");
    Serial.print(horizontalMotionRaw); 
    Serial.print(',');
    Serial.print(verticalMotionRaw);
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

  if(printNormalizedValues ==1)
  {
    Serial.print("Raw values: ");
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

void sendPosition()
{
  Serial.print(panLocation);
  Serial.print(',');
  Serial.print(tiltLocation);
  Serial.println();
}//end sendPosition()

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
  
}//end LCD Menu


