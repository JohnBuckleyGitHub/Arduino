#include <Wire.h>
#include <FastCRC.h>
#include "SDP610.h"

FastCRC16 CRC16;
int _resolution = 12;
SDP610 sdp_array[] = {{0x30,100,_resolution},{0x31,100,_resolution},{0x30,100,_resolution},{0x31,100,_resolution},{0x30,100,_resolution},{0x31,100,_resolution},{0x30,100,_resolution},{0x31,100,_resolution}};
byte sdp_sensorcount = 8;
byte NBytes = 4+1+sdp_sensorcount*2+2;  //4 bytes time + 2 bytes per sensor + 1 byte logging flag + 2 byte checksum + TEMP START BYTE
byte* byteArray = NULL;
byte bLog=0;
int inPin=2;
int16_t val = 0;
unsigned long lStart; //Time in milloseconds at start of logging;
unsigned long NPackets=0; //Number of packets sent
bool flStream=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin (115200);
  Wire.begin();  
  pinMode(inPin,INPUT);
  byteArray = new byte[NBytes];
  pinMode(10, OUTPUT);  //LED to say when we are logging.
  //Set resolution for all sensors:
  for (int i=0;i<sdp_sensorcount;i++)
  {
    sdp_array[i].SetResolution();
  }
  //Flash three times just to show we are ready to go:
  for (int i=0;i<3;i++)
  {
    digitalWrite(10,HIGH);
    delay(100);
    digitalWrite(10,LOW);
    delay(100);
  }
  
}

void loop() {
  HandleIncoming();
  if (flStream)
  {
    //4 bytes for time, 2 bytes for each sensor + 2 bytes for checksum + 1 for sampling flag coming from tunnel.
    //Put time in, in milliseconds:
    LongToPacketLoc(byteArray,0,(micros()-lStart));
    //Logging flag:
    byteArray [4]=digitalRead(inPin);
    //Send commands to all sensors to take measurement:
    for (int i=0;i<sdp_sensorcount;i++)
    {
      sdp_array[i].Measure();
    //}
    //Get measurements:
    //for (int i=0;i<sdp_sensorcount;i++)
    //{
      val = sdp_array[i].Get_Data();
      Int16ToPacketLoc(byteArray,i*2+5,val);
     //Serial.println(val);
    }
    //Checksum:
    UInt16ToPacketLoc(byteArray,5+sdp_sensorcount*2,CRC16.ccitt(byteArray, NBytes-2));
    //Write to serial data stream:
    Serial.write(byteArray,NBytes);
  }
}

void HandleIncoming()
{
  //Handle incoming bluetooth message:
  if (Serial.available())
  {
    byte a=Serial.read();
    switch (a){
      case 1: StopStreaming();
              break;
      case 2: StartStreaming();
              break;
    }
  }
}

void StartStreaming()
{
      lStart=micros();
      flStream=1;
      digitalWrite(10,HIGH);
      NPackets=0;
}

void StopStreaming()
{
      flStream=0;
      digitalWrite(10,LOW);
}



void UInt16ToPacketLoc(byte *bStream, int location, uint16_t value){
  int bLength = 2;
  byte b[bLength];
  UInt16ToByteArray(b,value);
  for (int i=0;i<bLength;i++)
  {
    bStream[i + location] = b[i];
  }
}

void UInt16ToByteArray (byte *b,int16_t x)
{
  * (int16_t *) b=x;
}

void Int16ToPacketLoc(byte *bStream, int location, uint16_t value){
  int bLength = 2;
  byte b[bLength];
  Int16ToByteArray(b,value);
  for (int i=0;i<bLength;i++)
  {
    bStream[i + location] = b[i];
  }
}

void Int16ToByteArray (byte *b,int16_t x)
{
  * (int16_t *) b=x;
}

void LongToPacketLoc(byte *bStream, int location, long value){
  int bLength = 4;
  byte b[bLength];
  LongToByteArray(b,value);
  for (int i=0;i<bLength;i++)
  {
    bStream[i + location] = b[i];
  }
}

void LongToByteArray(byte *b, long x){
//Following returns with least significant byte first!
  * (long *) b=x;
}
