/* 
For reference BT transmission on 70byte packet is 20ms
Wire readregister is 1.5ms per address
*/


#include <math.h>
//#include <LiquidCrystal.h>
#include <OneWire.h> 
#include <Wire.h>
#include <SoftwareSerial.h> //For bluetooth
#include <FastCRC.h>
#include "BMP280.h"
#include <MS5611_kj.h>

#define ONE_WIRE_BUS 6

bool debug = false;
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
SoftwareSerial BT(8,9);   //Use pins 8 and 9 (Rx/Tx)

OneWire oneWire(ONE_WIRE_BUS); 
FastCRC16 CRC16;

#define TCAADDR 0x70

unsigned long BTDelay=1;  //Number of milliseconds between BT sample sending.
unsigned long BTTime=0;
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
//byte bF5=0x1C;
byte bF5=0x0;
byte bCom=0;


BMP280 bmp_baro_array[] = {(0x77),(0x76),(0x77),(0x76)}; //BMP280 object at address 0x76 & 0x77:
MS5611 ms_baro_array[] = {(0x77),(0x76),(0x77),(0x76)}; //MS5611 object at address 0x76 & 0x77:
uint8_t bmp_tloc[] = {0,0,1,1};  // multiplex locations of BMP sensors
uint8_t ms_tloc[] = {2,2,3,3};  // multiplex locations of MS sensors
const int bmp_sensorCount=(sizeof(bmp_baro_array)/sizeof(*bmp_baro_array));  // It would be nice if c had a length function
const int ms_sensorCount=(sizeof(ms_baro_array)/sizeof(*ms_baro_array));
MS5611Data ms_Data[ms_sensorCount];
BMP280Data BMP_Data[bmp_sensorCount];
unsigned long last = 0;
const int phases = 4;
int phase = 0;
double ms_pressSum[ms_sensorCount];


void setup() {
  Serial.begin(115200);
  BT.begin(38400);  //Setup bluetooth and timing
  lStart=millis();
  BTTime=micros();
  init_bmp();
  init_ms();
}

void loop() {
  if (TimeGovernor())
    {
    BTTime=micros();
    PressureDataToStream();
    //WriteDataToBT();
    phase++;
    if (phase >= phases){
        createBtPacket();
        writeDataToBT();
        phase = 0;
    }
  }
  }

void init_bmp() {
  for(int i=0; i < bmp_sensorCount; i++)
  {
    tcaselect(bmp_tloc[i]);
    bmp_baro_array[i].Get_Cal();
    bmp_baro_array[i].write(0xF4, bF4);
    bmp_baro_array[i].write(0xF5, bF5); 
  }
}

void init_ms() {
  for(int i=0; i < ms_sensorCount; i++)
    {
      ms_pressSum[i] = 0;
      tcaselect(ms_tloc[i]);
      ms_baro_array[i].begin(MS5611_ULTRA_HIGH_RES, MS5611_HIGH_RES, i);
      ms_Data[i] = ms_baro_array[i].InitDataStruct(); 
  }
  // Update MS temps
  for(int i=0; i < ms_sensorCount; i++){
      tcaselect(ms_tloc[i]);
      ms_baro_array[i].UpdateTempData(true);
    }
}

void PressureDataToStream()
{
    // Get MS data, request temps
  //long ms_data_sum = 0;
  for(int i=0; i < ms_sensorCount; i++){
    tcaselect(ms_tloc[i]);
    if (phase != 1) { // Temperature will not be requested on first phase
        ms_baro_array[i].GetData(true, true);
    }
    else {
        ms_baro_array[i].GetData(true, false);
    }
    ms_Data[i] = ms_baro_array[i].lastData;
    //ms_data_sum = ms_data_sum + ms_Data[i].Press;
    ms_pressSum[i] = ms_pressSum[i] + ms_Data[i].Press;
    }
  //long ms_avg = ms_data_sum / ms_sensorCount;
  // Update MS temps
  if (phase == 1) {
      for (int i = 0; i < ms_sensorCount; i++) {
          tcaselect(ms_tloc[i]);
          ms_baro_array[i].UpdateTempData(true);
          //ms_baro_array[i].PrintData(i);
      }
  }
  // Update BMP sensros
  //long bmp_data_sum = 0;
  for(int i=0; i < bmp_sensorCount; i++)
  {
    if (phase == 0){
      tcaselect(bmp_tloc[i]);
      BMP_Data[i] = bmp_baro_array[i].Get_Data();}
    //bmp_data_sum = bmp_data_sum + BMP_Data[i].Press;
  }
  //long bmp_avg = bmp_data_sum / (bmp_sensorCount * 16);
}

void createBtPacket() {
    printAverages();
    int packetLoc = 0;
    LongToPacketLoc(bStream, packetLoc++, millis() - lStart);
    for (int i = 0; i < bmp_sensorCount; i++) {
        LongToPacketLoc(bStream, packetLoc++, BMP_Data[i].Temp);
        LongToPacketLoc(bStream, packetLoc++, BMP_Data[i].Press);
    }
    for (int j = bmp_sensorCount; j < (bmp_sensorCount + ms_sensorCount); j++) {
        int i = j - bmp_sensorCount;
        LongToPacketLoc(bStream, packetLoc++, long(100 * ms_Data[i].Temp));
        long ms_pressAvgLong = long(16 * ms_pressSum[i] / float(phases));
        LongToPacketLoc(bStream, packetLoc++, ms_pressAvgLong);
        ms_pressSum[i] = 0;
    }
}

void printAverages() {
    double bmpTempSum = 0;
    double bmpPressSum = 0;
    double msTempSum = 0;
    double msPressSum = 0;
    for (int i = 0; i < bmp_sensorCount; i++) {
        bmpTempSum =  bmpTempSum + BMP_Data[i].Temp;
        bmpPressSum = bmpPressSum + BMP_Data[i].Press;
    }
    for (int i = 0; i < ms_sensorCount; i++) {
        msTempSum = msTempSum + ms_Data[i].Temp;
        msPressSum = msPressSum + (ms_pressSum[i] / float(phases));
    }
    Serial.print("bmp:");
    double output = bmpPressSum / (float(bmp_sensorCount)*16);
    //Serial.println(bmpPressSum);
    Serial.println(output);
    Serial.print("ms:");
    output = msPressSum / float(ms_sensorCount);
    Serial.println(output);
}

boolean TimeGovernor(){
    if (debug == true) {
        Serial.print("Loop time was:  ");
        Serial.println((micros() - BTTime) / 1000);
    }
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
    if (debug == true) {
        Serial.print("Delay  :");
        Serial.println(milliDelay);
    }
    delay(milliDelay);
    delayMicroseconds(microDelay);
    return false;
  }
}

void writeDataToBT()
{
    //Checksum using fastcrc 16 bit ccitt scheme.
    uint16_t checksum=CRC16.ccitt(bStream,NBytes-2);
    byte b[2];
    b[0]=(checksum>>8) & 0xFF;
    b[1]=checksum & 0xFF;
    UnsignedLongToByteArray(b,checksum);
    bStream[NBytes-2]=b[0];
    bStream[NBytes-1]=b[1];
    BT.write(bStream,NBytes);
//    NPackets++;
}

void tcaselect(uint8_t i)
{
  if (i > 7) return; 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

void UnsignedLongToPacketLoc(byte *bStream, int location, uint16_t value){
  const int bLength = 4;
  byte b[bLength];
  UnsignedLongToByteArray(b,value);
  for (int i=0;i<bLength;i++)
  {
    bStream[i + (location * bLength)] = b[i];
  }
}

void LongToPacketLoc(byte *bStream, int location, long value){
  const int bLength = 4;
  byte b[bLength];
  LongToByteArray(b,value);
  for (int i=0;i<bLength;i++)
  {
    bStream[i + (location * bLength)] = b[i];
  }
}

void ShortToPacketLoc(byte *bStream, int location, uint16_t value){
  const int bLength = 2;
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

