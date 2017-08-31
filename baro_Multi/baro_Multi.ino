/* 
For reference BT transmission on 70byte packet is 20ms
Wire readregister is 1.5ms per address
*/


#include <math.h>
#include <OneWire.h> 
#include <Wire.h>
#include <SoftwareSerial.h> //For bluetooth
#include <BMP280.h>
#include <MS5611.h>
#include <SDP610.h>
#include <packetHandler.h>

#define ONE_WIRE_BUS 6

bool debug = false;
SoftwareSerial BT(8, 9);   //Use pins 8 and 9 (Rx/Tx)

OneWire oneWire(ONE_WIRE_BUS); 

#define TCAADDR 0x70

//Timing Values
unsigned long btDelay=5;  //Number of milliseconds between BT sample sending.
unsigned long btTime=0;
// unsigned long lTime;   //Time in milloseconds for sampling offsets;
unsigned long lStart; //Time in milloseconds at start of logging;
unsigned long delayTime=0;

//Dp Sensors
int _resolution = 13;
uint16_t timeout = 100;
SDP610 dpArray[] = { { 0x26,timeout,_resolution },{ 0x21,timeout,_resolution }};
const uint8_t dpSensorCount = (sizeof(dpArray) / sizeof(*dpArray));
uint16_t dpData[dpSensorCount];


// Baro sensors
//Settings values
byte bF4=0x57;
//byte bF5=0x1C;
byte bF5=0x0;
byte bCom=0;
BMP280 bmpBaroArray[] = {(0x77),(0x76),(0x77),(0x76)}; //BMP280 object at address 0x76 & 0x77:
MS5611 msBaroArray[] = {(0x77),(0x76),(0x77),(0x76)}; //MS5611 object at address 0x76 & 0x77:

uint8_t bmpTloc[] = {0,0,1,1};  // multiplex locations of BMP sensors
uint8_t ms_tloc[] = {2,2,3,3};  // multiplex locations of MS sensors

const uint8_t bmpSensorCount=(sizeof(bmpBaroArray)/sizeof(*bmpBaroArray));  // It would be nice if c had a length function
const uint8_t msSensorCount=(sizeof(msBaroArray)/sizeof(*msBaroArray));

sensorData ms_Data[msSensorCount];
sensorData BMP_Data[bmpSensorCount];
// Setup Conversion/Average Struct
sensorData msAvgData[msSensorCount];
unsigned long last = 0;
const int phases = 4;
int phase = 0;
double ms_pressSum[msSensorCount];

dataPacket btPacket;


void setup() {
  Serial.begin(115200);
  btPacket.setDataPacket(dpSensorCount, bmpSensorCount, msSensorCount);
  Serial.print("NPacketBytes = ");
  Serial.println(btPacket.NPacketBytes);
  btPacket.debugPrint();
  delay(1000);
  BT.begin(115200);  //Setup bluetooth and timing
  lStart=millis();
  btTime=micros();
  init_dp();
  init_bmp();
  init_ms();
}

void loop() {
  if (time_governor())
    {
    btTime=micros();
    pressure_data_to_stream();
    phase++;
    if (phase >= phases){
        create_bt_packet();
        btPacket.sendBTpacket(&BT);
        phase = 0;
    }
  }
  }

void init_dp()
{
    for (int i = 0; i < dpSensorCount; i++)
    {
        dpArray[i].SetResolution();
    }
}

void init_bmp() {
  for(int i=0; i < bmpSensorCount; i++)
  {
    tcaselect(bmpTloc[i]);
    bmpBaroArray[i].Get_Cal();
    bmpBaroArray[i].write(0xF4, bF4);
    bmpBaroArray[i].write(0xF5, bF5); 
  }
}

void init_ms() {
  for(int i=0; i < msSensorCount; i++)
    {
      ms_pressSum[i] = 0;
      tcaselect(ms_tloc[i]);
      msBaroArray[i].begin(MS5611_ULTRA_HIGH_RES, MS5611_HIGH_RES, i);
      ms_Data[i] = msBaroArray[i].InitDataStruct(); 
  }
  // Update MS temps
  for(int i=0; i < msSensorCount; i++){
      tcaselect(ms_tloc[i]);
      msBaroArray[i].UpdateTempData(true);
    }
}

void pressure_data_to_stream()
{
    // Get MS data, request temps
  for(int i=0; i < msSensorCount; i++){
    tcaselect(ms_tloc[i]);
    if (phase != 1) { // Temperature will not be requested on first phase
        msBaroArray[i].GetData(true, true);
    }
    else {
        msBaroArray[i].GetData(true, false);
    }
    ms_Data[i] = msBaroArray[i].lastData;
    ms_pressSum[i] = ms_pressSum[i] + ms_Data[i].Press;
    }
  // Update SDP data
  if (phase == 2)
  {
      for (int i = 0; i < dpSensorCount; i++)
      {
          dpArray[i].Measure();
          dpData[i] = dpArray[i].Get_Data();
      }
  }
  // Update MS temps
  if (phase == 1) {
      for (int i = 0; i < msSensorCount; i++) {
          tcaselect(ms_tloc[i]);
          msBaroArray[i].UpdateTempData(true);
      }
  }
  // Update BMP sensors
  if (phase == 0) 
  {
    for(int i=0; i < bmpSensorCount; i++)
        {
          tcaselect(bmpTloc[i]);
          BMP_Data[i] = bmpBaroArray[i].Get_Data();
        }
  }
 }

void create_bt_packet() {
    print_averages();
    btPacket.resetPacketLoc();
    btPacket.setTimeData(millis() - lStart);
    btPacket.setDpData(dpSensorCount, dpData);
    btPacket.setBaroData(bmpSensorCount, BMP_Data);
    // Convert data for MS sensors
    for (int i = 0; i < msSensorCount; i++) {
        msAvgData[i].Temp = ms_Data[i].Temp;
        msAvgData[i].Press = ms_pressSum[i] / float(phases);
        ms_pressSum[i] = 0;
    }
    btPacket.setBaroData(msSensorCount, msAvgData);
    btPacket.setChecksum();
}

void print_averages() {
    double bmpTempSum = 0;
    double bmpPressSum = 0;
    double msTempSum = 0;
    double msPressSum = 0;
    for (int i = 0; i < bmpSensorCount; i++) {
        bmpTempSum =  bmpTempSum + BMP_Data[i].Temp;
        bmpPressSum = bmpPressSum + BMP_Data[i].Press;
    }
    for (int i = 0; i < msSensorCount; i++) {
        msTempSum = msTempSum + ms_Data[i].Temp;
        msPressSum = msPressSum + (ms_pressSum[i] / float(phases));
    }
    Serial.print("bmp:");
    double output = bmpPressSum / (float(bmpSensorCount));
    Serial.println(output);
    output = bmpTempSum / (float(bmpSensorCount));
    Serial.println(output);
    Serial.print("ms:");
    output = msPressSum / float(msSensorCount);
    Serial.println(output);
}

boolean time_governor(){
    if (debug == true) {
        Serial.print("Loop time was:  ");
        Serial.println((micros() - btTime) / 1000);
    }
  if ((micros()-btTime)> (btDelay * 1000))
  {
    return true;
  }
  else{
    if (micros() < btTime) {
      delayTime = (btDelay * 1000) - (micros() + (4294967295 - btTime));
    }
    else {
      delayTime = (btDelay * 1000) - (micros()-btTime);
    }
    int milliDelay = delayTime / 1000;
    int microDelay = delayTime % 1000;
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



