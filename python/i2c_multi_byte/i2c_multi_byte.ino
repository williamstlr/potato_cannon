#include <Wire.h>

#define BUFFER_SIZE 7
#define SLAVE_ADDRESS 0x04
int number[BUFFER_SIZE];
int state = 0;
int count = 0;



void setup()
{
  pinMode(13,OUTPUT);
  Serial.begin(9600);
  Wire.begin(SLAVE_ADDRESS);
  
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
  
  Serial.println("Ready!");
}

void loop()
{
  delay(100);
  
  Serial.print("horizontalMotion = ");
  Serial.println(number[0]);
  Serial.print("verticalMotion = ");
  Serial.println(number[1]);
  Serial.print("fire = ");
  Serial.println(number[2]);
  
 

 Serial.println(); 

 
}

void receiveData(int byteCount)
{
  
 while (Wire.available())
  {
    
    number[count] = Wire.read();
    count ++;
    
    if(count == BUFFER_SIZE)
    {
      count = 0;
    }
  }
  
}

void sendData()
{
  
  for (int i = 0; i < BUFFER_SIZE;i++)
  {
    Wire.write(number[i]);
  }
  
  
}

