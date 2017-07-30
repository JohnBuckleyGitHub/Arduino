
#include <math.h>
#include <LiquidCrystal.h>
#include <OneWire.h> 
#include <SoftwareSerial.h> //For bluetooth
#include <FastCRC.h>
#include "BMP280.h"
#include <Wire.h>

#define ONE_WIRE_BUS 6

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
SoftwareSerial BT(8,9);   //Use pins 8 and 9 (Rx/Tx)

OneWire oneWire(ONE_WIRE_BUS); 
FastCRC16 CRC16;

int flStream;

unsigned long BTDelay=100;  //Number of milliseconds between BT sample sending.
unsigned long BTTime=0;
unsigned long LCDTime=0;
unsigned long lTime;   //Time in milliseconds for sampling offsets;
unsigned long lStart; //Time in milliseconds at start of logging;
const unsigned int NBytes = 22; //Number of bytes in packet
unsigned long NPackets=0;
byte bStream[NBytes];

//Global variables to store values which we then pass to BT and LCD:
int32_t Press=0;
int32_t Temp=0;

//Settings values
byte bF4=0x57;
byte bF5=0xC;
byte bCom=0;

//BMP280 object at address 0x76:
BMP280 myBMP280_A(0x76);
BMP280 myBMP280_B(0x77);

void setup() {
  //start wire library
  Wire.begin();
  //Start LCD library - my display is 20x4
//  lcd.begin(20, 4);
  Serial.begin(9600);
  Serial.println ("Reset");
  //BMP280 setup - sensor 1:
  //Get calibration figures:
  GetBMP280Cal(myBMP280_A);
  GetBMP280Cal(myBMP280_B);
  //Send starting config.  THis also puts into streaming mode.
  writeBMP20(myBMP280_A,0xF4, bF4);
  writeBMP20(myBMP280_A,0xF5, bF5);
  writeBMP20(myBMP280_B,0xF4, bF4);
  writeBMP20(myBMP280_B,0xF5, bF5);  
    
  //Setup bluetooth and timing:
  BT.begin(38400);
  lStart=millis();
  pinMode(10, OUTPUT);  //LED to say when we are logging.
  flStream=false;
  digitalWrite(10,LOW);
  StartStreaming();
}

void loop() {
  //Check for incoming bluetooth message:
  HandleBTIncoming();
//  //Get data from BMP280 and put into stream:
  BMPDataToStream();
  if ((millis()-BTTime)>BTDelay)
  {
    BTTime=millis();
    WriteDataToBT();
  }
}

void HandleBTIncoming()
{
  //Serial.println ("Checking BT..");
  //Handle incoming bluetooth message:
  if (BT.available())
  {
    byte a=BT.read();
    Serial.println(a);
    bCom=a;
//    switch (a){
//      case 2: StopStreaming();
//              break;
//      case 1: StartStreaming();
//              break;
//      case 3: SendSettings();
//              break;
//      case 4: ReceiveSettings();
//              break;
//      case 50: StopStreaming();
//              break;
//      case 49: StartStreaming();
//              break;
//    }
  }
}

void StartStreaming()
{
      lStart=millis();
      flStream=1;
      digitalWrite(10,HIGH);
      NPackets=0;
}

void StopStreaming()
{
      flStream=0;
      digitalWrite(10,LOW);
}

void SendSettings()
{
  byte* bSettings;
  bSettings = GetBMP280Settings(myBMP280_A);
  BT.write(bSettings,2);
  
}

void ReceiveSettings()
{
  //Expect to receive two values: 0xF4 and 0xF5:
  //Should modify this in due course to include a checksum..
  bF4=BT.read();
  bF5=BT.read();
  //Write data to BMP280:
  writeBMP20(myBMP280_A,0xF4, bF4);
  writeBMP20(myBMP280_A,0xF5, bF5);
  writeBMP20(myBMP280_B,0xF4, bF4);
  writeBMP20(myBMP280_B,0xF5, bF5);
}

void BMPDataToStream()
{
  //Gets data from BMP280 and puts into stream.
  //Get data from BMP280:
  BMP280Data MyData_A=GetBMP280Data(myBMP280_A);
  BMP280Data MyData_B=GetBMP280Data(myBMP280_B);
  //Serial.println(MyData_A.Temp);
  //Build byte array to send over bluetooth:
  //Timestamp (4 bytes)
  //Temperature sensor A as float (4 bytes)
  //Pressure sensor A as float (4 bytes)
  //Temperature sensor B as float (4 bytes)
  //Pressure sensor B as float (4 bytes)
  //Checksum - two bytes
  //So 22 bytes total:
  //Each value is 32 bits, so need to convert them to byte arrays first:
  byte b[4];
  //Write time in microseconds:
  unsigned long lTime=millis()-lStart;
  UnsignedLongToByteArray(b,lTime);
  for (int i=0;i<4;i++)
  {
    bStream[i]=b[i];
  }
  //Temperature sensor A:
  LongToByteArray(b,MyData_A.Temp);
  for (int i=0;i<4;i++)
  {
    bStream[i+4]=b[i];
  }
  //Pressure sensor A:
  LongToByteArray(b,MyData_A.Press);
  for (int i=0;i<4;i++)
  {
    bStream[i+8]=b[i];
  }
  //Temperature sensor B:
  LongToByteArray(b,MyData_B.Temp);
  for (int i=0;i<4;i++)
  {
    bStream[i+12]=b[i];
  }
  //Pressure sensor B:
  LongToByteArray(b,MyData_B.Press);
  for (int i=0;i<4;i++)
  {
    bStream[i+16]=b[i];
  }
}

void WriteDataToBT()
{
  if (flStream==1)
  {
    uint16_t checksum=CRC16.ccitt(bStream,NBytes-2);
    byte b[2];
    b[0]=(checksum>>8) & 0xFF;
    b[1]=checksum & 0xFF;
    UnsignedLongToByteArray(b,checksum);
    bStream[NBytes-2]=b[0];
    bStream[NBytes-1]=b[1];
    BT.write(bStream,NBytes);
    NPackets++;
  }
}

void UnsignedLongToByteArray(byte *b, unsigned long x){
//Following returns with least significant byte first!
  b[0]=(byte)x;
  b[1]=(byte)(x>>8);
  b[2]=(byte)(x>>16);
  b[3]=(byte)(x>>24);
}

void LongToByteArray(byte *b, long x){
//Following returns with least significant byte first!
  * (long *) b=x;
}

void ShortToByteArray (byte *b,uint16_t x)
{
  * (uint16_t *) b=x;
}

void FloatToByteArray(byte *b, float x){
  * (float *) b=x;
}


void writeBMP20(BMP280 myBMP280, byte reg, byte data) {
  // Send output register address
  Wire.beginTransmission(myBMP280.Address);
  Wire.write(reg);
  // Connect to device and send byte
  Wire.write(data);
  Wire.endTransmission();
}

void readBMP280(BMP280 myBMP280,byte reg, int count, byte *data) {
  int i = 0;
  byte c;
  // Send input register address
  Wire.beginTransmission(myBMP280.Address);
  Wire.write(reg);
  Wire.endTransmission();
  // Connect to device and request bytes
  Wire.beginTransmission(myBMP280.Address);
  Wire.requestFrom(myBMP280.Address, count);
  while (Wire.available()) { // slave may send less than requested
    c = Wire.read(); // receive a byte as character
    data[i] = c;
    i++;
  }
  Wire.endTransmission();
}

void GetBMP280Cal(BMP280 &myBMP280){
  byte bCal[24];
  readBMP280(myBMP280,0x88, 24, bCal);
  myBMP280.Set_Cal(bCal);
}

byte* GetBMP280Settings(BMP280 myBMP280)
{
  byte b[2];
  readBMP280(myBMP280,0xF4, 2, b);
  return b;
}

BMP280Data GetBMP280Data(BMP280 myBMP280){
  byte bData[6];
  readBMP280(myBMP280,0xF7, 6, bData);
  BMP280Data myBMP280Data = myBMP280.Get_Data(bData);
  return myBMP280Data;
}

