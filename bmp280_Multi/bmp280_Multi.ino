
#include <math.h>
//#include <LiquidCrystal.h>
#include <OneWire.h> 
#include <Wire.h>
#include <SoftwareSerial.h> //For bluetooth
#include <FastCRC.h>
#include "BMP280.h"

#define ONE_WIRE_BUS 6


//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
SoftwareSerial BT(8,9);   //Use pins 8 and 9 (Rx/Tx)

OneWire oneWire(ONE_WIRE_BUS); 
FastCRC16 CRC16;

int flStream;

#define TCAADDR 0x70

unsigned long BTDelay=50;  //Number of milliseconds between BT sample sending.
unsigned long LCDDelay = 100; //Number of milliseconds between LCD refresh    
unsigned long BTTime=0;
unsigned long LCDTime=0;
unsigned long lTime;   //Time in milloseconds for sampling offsets;
unsigned long lStart; //Time in milloseconds at start of logging;
const unsigned int NBytes = 70; //Number of bytes in packet
unsigned long NPackets=0;
unsigned long DelayTime=0;
byte bStream[NBytes];
float display_pressure;

//Global variables to store values which we then pass to BT and LCD:
int32_t Press=0;
int32_t Temp=0;

//Settings values
byte bF4=0x57;
byte bF5=0x1C;
byte bCom=0;


BMP280 baro_array[] = {(0x77),(0x76),(0x77),(0x76),(0x77),(0x76),(0x77),(0x76)}; //BMP280 object at address 0x76 & 0x77:
uint8_t tloc[] = {0,0,1,1,2,2,3,3};  // multiplex locations of BMP sensors
unsigned int sensorCount=(sizeof(baro_array)/sizeof(*baro_array));  // It would be nice if c had a length function
unsigned long last = 0;

void setup() {
  Serial.begin(115200);
 
  
  BT.begin(115200);  //Setup bluetooth and timing
  lStart=millis();
  BTTime=micros();
  init_bmp();
//  pinMode(10, OUTPUT);  //LED to say when we are logging.
  flStream=false;
//  digitalWrite(10,LOW);
}

void loop() {
  //Get data from BMP280 and put into stream:
//  BMPDataToStream();
  if (TimeGovernor())
    {
    BTTime=micros();
    BMPDataToStream();
    WriteDataToBT();
    }
  }

void init_bmp() {
  for(int i=0; i < sensorCount; i++)
  {
    tcaselect(tloc[i]);
    baro_array[i].Get_Cal();
    baro_array[i].write(0xF4, bF4);
    baro_array[i].write(0xF5, bF5); 
  }
}

void BMPDataToStream()
{
  //Gets data from BMP280 and puts into stream.
  //Get data from BMP280:
  BMP280Data MyData[sensorCount];
  long data_sum = 0;
  for(int i=0; i < sensorCount; i++)
  {
    tcaselect(tloc[i]);
    MyData[i] = baro_array[i].Get_Data(); 
    data_sum = data_sum + MyData[i].Press;
  }
  long avg = data_sum / sensorCount;
  int packetLoc = 0;
//  UnsignedLongToPacketLoc(bStream, packetLoc++, millis()-lStart);
  LongToPacketLoc(bStream, packetLoc++, millis()-lStart);
  unsigned long cur = millis()-lStart;
//  Serial.println(millis());
//  Serial.println(cur - last);
//  if ((cur - last) > 50) {
//    Serial.println("\n\n\n\nBang Bang Bang Bang!!!!!!!!!!!!!!!!!!!!\n\n\n");
//  }
  last = cur;
//  Serial.println("");
  for(int i=0; i < sensorCount; i++){
    LongToPacketLoc(bStream, packetLoc++, MyData[i].Temp);
//    if (i == 0) {
//      Serial.println("\n");
//      Serial.println(MyData[i].Raw_P);
//      display_pressure = MyData[i].Press / 16.0;
//      Serial.println(display_pressure);} 
    LongToPacketLoc(bStream, packetLoc++, MyData[i].Press);
  }
}

boolean TimeGovernor(){
  delay(60);
  return true;
  if ((micros()-BTTime)> (BTDelay * 1000))
  {
    return true;
  }
  else{
    if (micros() < BTTime) {
      DelayTime = (BTDelay * 1000) - (micros() + (4294967295 - BTTime));
    }
    else {
      DelayTime = (BTDelay * 1000) - (micros()-BTTime);
    }
    int milliDelay = DelayTime / 1000;
    int microDelay = DelayTime % 1000;
    delay(milliDelay);
    delayMicroseconds(microDelay);
    return false;
  }
}

void WriteDataToBT()
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

void tcaselect(uint8_t i)
{
  if (i > 7) return; 
//  delay(10);
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
//  delay(10);
}

void UnsignedLongToPacketLoc(byte *bStream, int location, uint16_t value){
  int bLength = 4;
  byte b[bLength];
  UnsignedLongToByteArray(b,value);
  for (int i=0;i<bLength;i++)
  {
    bStream[i + (location * bLength)] = b[i];
  }
}

void LongToPacketLoc(byte *bStream, int location, long value){
  int bLength = 4;
  byte b[bLength];
  LongToByteArray(b,value);
  for (int i=0;i<bLength;i++)
  {
    bStream[i + (location * bLength)] = b[i];
  }
}

void ShortToPacketLoc(byte *bStream, int location, uint16_t value){
  int bLength = 2;
  byte b[bLength];
  ShortToByteArray(b,value);
  for (int i=0;i<bLength;i++)
  {
    bStream[i + (location * bLength)] = b[i];
  }
}

void UnsignedLongToByteArray(byte *b, uint32_t x){
//Following returns with least significant byte first!
  * (uint16_t *) b=x;
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

