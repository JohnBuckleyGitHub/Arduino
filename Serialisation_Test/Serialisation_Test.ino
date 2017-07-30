
#include <SoftwareSerial.h>
SoftwareSerial BTserial(12, 13); // RX | TX
const long baudRate = 115200;
int flTest;

 unsigned long lRate=1100;  //Number of microseconds between sample.  1130 is theoretical limit with 13 bytes.
 unsigned long lOffset=0; //Micors resets after 70 minutes
 unsigned long lTime;
 unsigned long lFudge=0;
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(baudRate);
  BTserial.begin(baudRate);
  flTest=0;
  lTime=micros();
}

  
void loop() {
  // put your main code here, to run repeatedly:
  byte bTest=49;  //ASCII 1
  if (Serial.available())
  {
    byte a=Serial.read();
    if (a == 49)
    { 
      flTest=1;
    }
    else if (a == 50)
    {
      flTest=0;
    }
  }
    if (BTserial.available())
  {
    byte a=BTserial.read();
    if (a == 49)
    { 
      flTest=1;
    }
    else if (a == 50)
    {
      flTest=0;
    }
  }
  if (flTest==1)
  { 
    //Aim to send three things:
    //Timestamp (4 bytes)
    //Unsigned long value (4 bytes)
    //Double (4 bytes)
    //Checksum - just a single byte adding up all the others.
    //So 13 bytes total:
    //Get timestamp:
    
    if (micros()<lTime)
    {
      //Overflow, so add offset:
      lOffset=0-1;
    }
    else
    {
      lOffset=0;
    }
    if (micros()>=(lTime+lRate-lFudge))//+lOffset)         //Note this wil never work because overflow.
    {
      lTime=micros();
    
      byte bStream[13];
      unsigned long lTime2=micros();
      unsigned long lTest= 123456789;
      float fTest=123.456789;
      //Double is composed of 4 bits:
      byte b[4];
      byte bCheck=0;
      LongToByteArray(b,lTime2);
      for (int i=0;i<4;i++)
        {
          bStream[i]=b[i];
          bCheck=bCheck ^ b[i];
        }
      LongToByteArray(b,lTest);
      for (int i=0;i<4;i++)
        {
          bStream[i+4]=b[i];
          bCheck=bCheck ^ b[i];
        }
      FloatToByteArray(b,fTest);
      for (int i=0;i<4;i++)
        {
          bStream[i+8]=b[i];
          bCheck=bCheck ^ b[i];  //bitwise xor.  Pretty basic..
        }
        bStream[12]=bCheck;
      Serial.write(bStream,13);
      BTserial.write(bStream,13);
    }
  }
}

void LongToByteArray(byte *b, unsigned long x){
//  b[0]=(x>>24) & 0xFF;
//  b[1]=(x>>16) & 0xFF;
//  b[2]=(x>>8) & 0xFF;
//  b[3]=x & 0xFF;
//Following returns with least significant byte first!
  * (unsigned long *) b=x;
}

void FloatToByteArray(byte *b, float x){
  * (float *) b=x;
}

