
#include <math.h>
//#include <LiquidCrystal.h>
#include <OneWire.h> 
#include <SoftwareSerial.h> //For bluetooth
#include <FastCRC.h>
#include "BMP280.h"

#define ONE_WIRE_BUS 6

//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
SoftwareSerial BT(8,9);   //Use pins 8 and 9 (Rx/Tx)

OneWire oneWire(ONE_WIRE_BUS); 
FastCRC16 CRC16;

int flStream;

unsigned long BTDelay=10;  //Number of milliseconds between BT sample sending.
//unsigned long LCDDelay = 100; //Number of milliseconds between LCD refresh    
unsigned long BTTime=0;
//unsigned long LCDTime=0;
unsigned long lTime;   //Time in milloseconds for sampling offsets;
unsigned long lStart; //Time in milloseconds at start of logging;
const unsigned int NBytes = 14; //Number of bytes in packet
unsigned long NPackets=0;
byte bStream[NBytes];
const long baudRate = 115200;
//Global variables to store values which we then pass to BT and LCD:
int32_t Press=0;
int32_t Temp=0;

//Settings values
byte bF4=0x57;
byte bF5=0xC;
byte bCom=0;

//BMP280 object at address 0x76:
BMP280 myBMP280_A(0x76);
BMP280 myBMP280_B(0x76);

void setup() {
  //Start LCD library - my display is 20x4
//  lcd.begin(20, 4);
  
//  BMP280 setup - sensor 1:
//  Get calibration figures:
  myBMP280_A.Get_Cal();
  //Send starting config.  THis also puts into streaming mode.
//  myBMP280_A.write(0xF4, bF4);
//  myBMP280_A.write(0xF5, bF5);
//  
//  //BMP280 setup - sensor 2:
//  //Get calibration figures:
//  myBMP280_A.Get_Cal();
//  //Send starting config.  THis also puts into streaming mode.
//  myBMP280_A.write(0xF4, bF4);
//  myBMP280_A.write(0xF5, bF5);
    
  //Setup bluetooth and timing:
  BT.begin(baudRate);
  Serial.begin(baudRate);
  lStart=millis();
  pinMode(10, OUTPUT);  //LED to say when we are logging.
  flStream=false;
//  digitalWrite(10,LOW);
}

void loop() {
  //Check for incoming bluetooth message:
//  HandleBTIncoming();
  //Get data from BMP280 and put into stream:
//  BMPDataToStream();
//  if ((millis()-BTTime)>BTDelay)
//  {
//    BTTime=millis();
//    WriteDataToBT();
//  }
  Serial.println("Enter AT commands:");
//  Have removed the LCD:
//  if ((millis()-LCDTime)>LCDDelay)
//  {
//    LCDTime=millis();
//    RefreshLCD();
//  }
}

void HandleBTIncoming()
{
  //Handle incoming bluetooth message:
  if (BT.available())
  {
    byte a=BT.read();
    bCom=a;
    switch (a){
      case 2: StopStreaming();
              break;
      case 1: StartStreaming();
              break;
      case 3: SendSettings();
              break;
      case 4: ReceiveSettings();
              break;
      //Buckley Android code sending ascii - 49/50 for 1/2..
      case 50: StopStreaming();
              break;
      case 49: StartStreaming();
              break;
    }
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
  bSettings = myBMP280_A.Get_Settings();
  BT.write(bSettings,2);
  
}

void ReceiveSettings()
{
  //Expect to receive two values: 0xF4 and 0xF5:
  //Should modify this in due course to include a checksum..
  bF4=BT.read();
  bF5=BT.read();
  //Write data to BMP280:
  myBMP280_A.write(0xF4, bF4);
  myBMP280_B.write(0xF5, bF5);
}

void BMPDataToStream()
{
  //Gets data from BMP280 and puts into stream.
  //Get data from BMP280:
  BMP280Data MyData=myBMP280_A.Get_Data();
  
  Press = MyData.Press;
  Temp=MyData.Temp;
  
  //Build byte array to send over bluetooth:
  //Timestamp (4 bytes)
  //Temperature as float (4 bytes)
  //Pressure as float (4 bytes)
  //Checksum - two bytes
  //So 14 bytes total:
  //Each value is 32 bits, so need to convert them to byte arrays first:
  byte b[4];
  //Write time in microseconds:
  UnsignedLongToByteArray(b,millis()-lStart);
  for (int i=0;i<4;i++)
  {
    bStream[i]=b[i];
  }
  //Temperature:
  LongToByteArray(b,Temp);
  for (int i=0;i<4;i++)
  {
    bStream[i+4]=b[i];
  }
  //Pressure:
  LongToByteArray(b,Press);
  for (int i=0;i<4;i++)
  {
    bStream[i+8]=b[i];
  }
}

//void RefreshLCD()
//{
//  lcd.clear();
//  lcd.setCursor(0, 0);
//  lcd.print("T1=");
//  lcd.print(Temp);
//  lcd.print("C");
//  lcd.setCursor(0, 1);
//  lcd.print("P = ");
//  lcd.print(Press);
//  lcd.print("Pa");
//  lcd.setCursor(0, 2);
//  lcd.print ("bF4=");
//  lcd.print (bF4);
//  lcd.print (" bF5=");
//  lcd.print (bF5);
//  lcd.setCursor(0, 3);
//  float fTime=(float)(micros()-lStart)/1000000.0;
//  lcd.print (" ");
//  lcd.print (fTime,2);
//  
//}

void WriteDataToBT()
{
  if (flStream==1)
  {
    //Checksum using fastcrc 16 bit ccitt scheme.
    byte bTest[4];
    for (byte i=0;i<4;i++)
    {
      bTest[i]=i;
    }
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

void UnsignedLongToByteArray(byte *b, uint16_t x){
//  b[0]=(x>>24) & 0xFF;
//  b[1]=(x>>16) & 0xFF;
//  b[2]=(x>>8) & 0xFF;
//  b[3]=x & 0xFF;
//Following returns with least significant byte first!
  * (uint16_t *) b=x;
}

void LongToByteArray(byte *b, long x){
//  b[0]=(x>>24) & 0xFF;
//  b[1]=(x>>16) & 0xFF;
//  b[2]=(x>>8) & 0xFF;
//  b[3]=x & 0xFF;
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

