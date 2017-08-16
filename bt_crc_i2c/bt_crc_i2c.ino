#include "Wire.h"
#include "stdlib.h"
//#define SDP_ADD_1 0x25  //decimal address of sensor 1
//#define SDP_ADD_2 0x22  //decimal address of sensor 1
//#define SDP_ADD_3 0x23  //decimal address of sensor 1
//#define SDP_ADD_4 0x24  //decimal address of sensor 1
#include <SoftwareSerial.h>
#include <FastCRC.h>
FastCRC16 CRC16;
SoftwareSerial BTserial(8, 9); // RX | TX

const long baudRate = 115200;
int flTest;

 unsigned long lRate=1100;  //Number of microseconds between sample.  1130 is theoretical limit with 13 bytes.
 unsigned long lOffset=0; //Micors resets after 70 minutes
 unsigned long lTime;
 unsigned long lFudge=0;
 unsigned long count=0;
 unsigned long half_long= 0xffffffff/2;
 unsigned long total_count = 0;
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(baudRate);
  BTserial.begin(baudRate);
  Wire.begin();     // create a wire object
  flTest=0;
  lTime=millis();
}

  
void loop() {
  // put your main code here, to run repeatedly:
  total_count++;
//  Serial.print("Total count: ");
//  Serial.println(total_count);
//  Serial.println("Sending TX to sensor");
    unsigned long fP1 = half_long + read_SDP610(0x21);  //the "1" is the number 1, not letter l.. 
    unsigned long fP2 = half_long + read_SDP610(0x22);  
    unsigned long fP3 = half_long + read_SDP610(0x26);  
    unsigned long fP4 = half_long + read_SDP610(0x24);  
//  Serial.println("Received RX from sensor");
  // end  of new code
  delay(20);
  byte bTest=49;  //ASCII 1
  Serial.println("Checking serial input");
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
      count=0;
    }
  }
//  Serial.println("Serial input checked, now caclulating packet");
  flTest = 1;
  if (flTest==1)
  { 
    //Aim to send three things:
    //Timestamp (4 bytes)
    //Unsigned long value (4 bytes)
    //Double (4 bytes)
    //Checksum - just a single byte adding up all the others.
    //So 13 bytes total:
    //Get timestamp:
    
    if (millis()<lTime)
    {
      //Overflow, so add offset:
      lOffset=0-1;
    }
    else
    {
      lOffset=0;
    }
    

      lTime=millis();
      byte bStream[22];
      unsigned long lTime2=millis();
//      float fTest=23.45;
      //Double is composed of 4 bits:
      byte b[4];
      byte b2[2];
      LongToByteArray(b,lTime2);
      for (int i=0;i<4;i++)
        {
          bStream[i]=b[i];
        }
      LongToByteArray(b,fP1);
      for (int i=0;i<4;i++)
        {
          bStream[i+4]=b[i];
        }
      LongToByteArray(b,fP2);
      for (int i=0;i<4;i++)
        {
          bStream[i+8]=b[i];
        }
      LongToByteArray(b,fP3);
      for (int i=0;i<4;i++)
        {
          bStream[i+12]=b[i];
        }
      LongToByteArray(b,fP4);
      for (int i=0;i<4;i++)
        {
          bStream[i+16]=b[i];
        }
//      LongToByteArray(b,count);
//      for (int i=0;i<4;i++)
//        {
//          bStream[i+20]=b[i];
//        }
      /* for (int i=0;i<12;i++){
        bStream[i]=i+1; 
      } */
      int checksumValue = CRC16.ccitt(bStream, sizeof(bStream)-2);
      Serial.println("The four pressure values are: ");
      double disp1 = ((double(fP1)-half_long)/60);
      double disp2 = ((double(fP2)-half_long)/60);
      double disp3 = ((double(fP3)-half_long)/60);
      double disp4 = ((double(fP4)-half_long)/60);
      Serial.println(disp1);
      Serial.println(disp2);
      Serial.println(disp3);
      Serial.println(disp4);
      IntToByteArray(b2, checksumValue);
      bStream[20] = b2[0];
      bStream[21] = b2[1];
//      Serial.write(bStream,22);
//      Serial.println();
      BTserial.write(bStream,22);
//      Serial.write(bStream, 22);
      count++;
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

void IntToByteArray(byte *b, unsigned int x){
//  b[0]=(x>>8) & 0xFF;
//  b[1]=x & 0xFF;
//Following returns with least significant byte first!
  * (unsigned int *) b=x;
}

void FloatToByteArray(byte *b, float x){
  * (float *) b=x;
}

int read_SDP610(int address) {
  byte bHigh;   
  byte bLow;
  //start the communication with IC with the address xx
//  Serial.print("Beginning request transmission on: ");
//  Serial.println(address);
  Wire.beginTransmission(address); 
  //Send request to trigger measurement:
  Wire.write(0xF1);
  //end transmission
//    return 1000;
  Wire.endTransmission();
//  Serial.print("Finished request transmission on: ");
//  Serial.println(address);
  //request to start reading:
  //request 2 bytes from address xx
  //SDP returns two bytes, most significant first.
//    return 1000;
//  Serial.print("Beginning request data on:");
//  Serial.println(address);
  Wire.requestFrom(address, 2);
  // Serial.print((int) address);
  //wait for response
  bool wire_available = 1;
  unsigned long begin_wait_time = millis();
  while(Wire.available() == 0){
    if ((millis() - begin_wait_time) > 100){
      wire_available = 0;
      Serial.print("wire unavailable on: ");
      Serial.println(address);
      break;
    }
  }
    //Read the two bytes containing result:
    if (wire_available = 1){
//      Serial.print("wire available on: ");
//      Serial.println(address);
      bHigh = Wire.read();   
      bLow = Wire.read(); }
    else {
      bHigh = 0;
      bLow = 0;
    }
    //We need to convert this into a twos compliment integer.
    //Sensor is 12 bit by default, so number is first 11 bit - 12th bit
    //We put the two byes together, then use bitwise and to mask bits we dont want.to put the two together.  
    //To put bytes together, we shift high byte to the left by 8 bits, then add on low byte
    //short data type is a 16 bit value integer:
    short iResult = (bHigh<<8)|bLow;
//    Serial.print("Result = ");
//    Serial.println(iResult);
  //Turns out this does the twos complement automatically, but if you wanted to do manually it would look like this:
 // short iResult2=(((bHigh<<8)|bLow) & 2047) - (((bHigh<<8)|bLow) & 2048);
  
  //we divide this by scale factor to get output in Pa.
  //For 25Pa sensor the scale factor is 1200.
  //For 125Pa sensor the scale factor is 240.
  //For 500Pa sensor the scale factor is 60Pa.
  return iResult;
}
