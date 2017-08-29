/* 
For reference BT transmission on 70byte packet is 20ms
Wire readregister is 1.5ms per address
*/


#include <math.h>
#include <OneWire.h> 
#include <Wire.h>
#include <SoftwareSerial.h> //For bluetooth
#include "BMP280.h"
#include <MS5611.h>
#include "packetHandler.h"

#define ONE_WIRE_BUS 6

bool debug = false;
SoftwareSerial BT(8, 9);   //Use pins 8 and 9 (Rx/Tx)

OneWire oneWire(ONE_WIRE_BUS); 

#define TCAADDR 0x70

unsigned long BTDelay=5;  //Number of milliseconds between BT sample sending.
unsigned long BTTime=0;
unsigned long lTime;   //Time in milloseconds for sampling offsets;
unsigned long lStart; //Time in milloseconds at start of logging;
unsigned long NPackets=0;
unsigned long DelayTime=0;
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

const uint8_t bmp_sensorCount=(sizeof(bmp_baro_array)/sizeof(*bmp_baro_array));  // It would be nice if c had a length function
const uint8_t ms_sensorCount=(sizeof(ms_baro_array)/sizeof(*ms_baro_array));

sensorData ms_Data[ms_sensorCount];
sensorData BMP_Data[bmp_sensorCount];
// Setup Conversion/Average Struct
sensorData msAvgData[ms_sensorCount];
unsigned long last = 0;
const int phases = 4;
int phase = 0;
double ms_pressSum[ms_sensorCount];

dataPacket BTpacket;



void setup() {
  Serial.begin(115200);
  Serial.print("NPacketBytes = ");
  Serial.println(BTpacket.PacketByteCount);
  BTpacket.debugPrint();
  BT.begin(115200);  //Setup bluetooth and timing
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
    phase++;
    if (phase >= phases){
        createBtPacket();
        BTpacket.sendBTpacket(&BT);
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
  for(int i=0; i < ms_sensorCount; i++){
    tcaselect(ms_tloc[i]);
    if (phase != 1) { // Temperature will not be requested on first phase
        ms_baro_array[i].GetData(true, true);
    }
    else {
        ms_baro_array[i].GetData(true, false);
    }
    ms_Data[i] = ms_baro_array[i].lastData;
    ms_pressSum[i] = ms_pressSum[i] + ms_Data[i].Press;
    }
 
  // Update MS temps
  if (phase == 1) {
      for (int i = 0; i < ms_sensorCount; i++) {
          tcaselect(ms_tloc[i]);
          ms_baro_array[i].UpdateTempData(true);
          //ms_baro_array[i].PrintData(i);
      }
  }
  // Update BMP sensors
  for(int i=0; i < bmp_sensorCount; i++)
  {
    if (phase == 0){
      tcaselect(bmp_tloc[i]);
      BMP_Data[i] = bmp_baro_array[i].Get_Data();}
  }
}

void createBtPacket() {
    printAverages();
    BTpacket.resetPacketLoc();
    BTpacket.setTimeData(millis() - lStart);
    BTpacket.setBaroData(bmp_sensorCount, BMP_Data);
    // Convert data for MS sensors
    for (int i = 0; i < ms_sensorCount; i++) {
        msAvgData[i].Temp = ms_Data[i].Temp;
        msAvgData[i].Press = ms_pressSum[i] / float(phases);
        ms_pressSum[i] = 0;
    }
    BTpacket.setBaroData(ms_sensorCount, msAvgData);
    BTpacket.setChecksum();
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
    double output = bmpPressSum / (float(bmp_sensorCount));
    Serial.println(output);
    output = bmpTempSum / (float(bmp_sensorCount));
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

void tcaselect(uint8_t i)
{
  if (i > 7) return; 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}



