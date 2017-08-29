#include "Arduino.h"
#include "BMP280.h"
#include <Wire.h>
//31/3/17 BLAH

 BMP280::BMP280(byte Address)
{
  _address=Address;
  //start wire library
  Wire.begin();
  pcount = 0;
}

byte BMP280::Get_Address()
{
  return _address;
}


sensorData BMP280::Get_Data()
{
  //struct BMP280Data myData;
  struct sensorData myData;
  byte bData[6];
  read (0xF7, 6, bData);
  uint32_t adc_P;
  uint32_t adc_T;
  adc_T = ((uint32_t)bData[3] << 12) + ((uint32_t)bData[4] << 4) + ((uint32_t)bData[5] >> 4);
  adc_P = ((uint32_t)bData[0] << 12) + ((uint32_t)bData[1] << 4) + ((uint32_t)bData[2] >> 4);
  //adc_P = ((uint32_t)bData[0] << 16) + ((uint32_t)bData[1] << 8) + ((uint32_t)bData[2] >> 0);  // Byte shifted << 4
  myData.Temp = bmp280_compensate_T_double(adc_T);
  myData.Press = bmp280_compensate_P_double(adc_P);
  myData.Pcount = pcount++;
  myData.Raw_P = adc_P;
  myData.Raw_T = adc_T;
  return myData;
}


////Returns altitude
float BMP280::CalcAlt(float dblPress)
{
  float dblCalc, a, b, c,d;
  //result=hb+Tb/Lb*pow((dblPress/Pb),((-R*Lb/g/M)-1.0));
  //a = -R;
  a = Tb / Lb;
  b = dblPress/Pb;
  c = -R * Lb / g / M;
  dblCalc = hb + a * (pow(b, c)-1);
  return dblCalc;
}

void BMP280::write(byte reg, byte data) {
  // Send output register address
  Wire.beginTransmission(_address);
  Wire.write(reg);
  // Connect to device and send byte
  Wire.write(data); // low byte
  Wire.endTransmission();
}

void BMP280::read(byte reg, uint8_t count, byte* data) {
  int i = 0;
  byte c;
  // Send input register address
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.endTransmission();
  // Connect to device and request bytes
  Wire.beginTransmission(_address);
  Wire.requestFrom(uint8_t(_address), count);
  while (Wire.available()) { // slave may send less than requested
    c = Wire.read(); // receive a byte as character
    data[i] = c;
    i++;
  }
  Wire.endTransmission();
}

// Returns integer representation of temperature in degC.  2406 means 24.06degC
//double BMP280::bmp280_compensate_T_int32(int32_t adc_T)
//{
//  int32_t var1p, var1, var2p, var2, T;
//  var1p = (adc_T >> 3) - ((int32_t)dig_T1 << 1);
//  var1 = var1p * ((int32_t)dig_T2) >> 11;
//  var2p = ((adc_T >> 4) - ((int32_t)dig_T1));
//  var2 = ((var2p * var2p) >> 12)* ((int32_t)dig_T3) >> 14;
//  t_fine = var1 + var2;
//  //T = (t_fine * 5 + 128) >> 8;
//  T = (t_fine * 5 + 128);
//  //double out_T = T / (256 * 10);
//  double out_T = ((t_fine * 5 + 128) >> 8);
//  return out_T;
//}

// Returns temperature in DegC, double precision. Output value of “51.23” equals 51.23 DegC.
double BMP280::bmp280_compensate_T_double(int32_t adc_T)
{
    double var1, var2, T;
    var1 = ((((double)adc_T) / 16384.0) - ((double)dig_T1) / 1024.0) * ((double)dig_T2);
    var2 = (((((double)adc_T) / 131072.0) - ((double)dig_T1) / 8192.0) *
        ((((double)adc_T) / 131072.0) - ((double)dig_T1) / 8192.0)) * ((double)dig_T3);
    t_fine = (int32_t)(var1 + var2);
    T = (var1 + var2) / 5120.0;
    return T;
}

// Returns pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
//double BMP280::bmp280_compensate_P_int32(int32_t adc_P)
//{
//  int64_t var1, var2;
////  unsigned long p;
//  unsigned long long p;
//  long pp;
//  var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
////  var1 = (((int32_t)135655) >> 1) - (int32_t)64000;  // Locks in Temperature at 26.5°C
//  var2 = (((var1 >> 2) * (var1 >> 2)) >> 11 ) * ((int32_t)dig_P6);
//  var2 = var2 + ((var1 * ((int32_t)dig_P5)) << 1);
//  var2 = (var2 >> 2) + (((int32_t)dig_P4) << 16);
//  var1 = (((dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13 )) >> 3) + ((((int32_t)dig_P2) * var1) >> 1)) >> 18;
//  var1 = ((((32768 + var1)) * ((int32_t)dig_P1)) >> 15);
//  if (var1 == 0)
//  {
//    return 0; // avoid exception caused by division by zero
//  }
////  p = (((uint32_t)(((int32_t)1048576) - adc_P) - (var2 >> 12))) * 3125;
//  p = (uint64_t)(((int64_t)1048576 << 4) - adc_P);
//  p = (p - (var2 >> 8)) * 3125;
//// if (p < 0x80000000)
//  if ((p >> 4) < 0x80000000)
//  {
//    p = (p << 1) / ((uint32_t)var1);
//  }
//  else
//  {
//    p = (p / (uint32_t)var1) * 2;
//  }
////  var1 = (((int64_t)dig_P9) * ((int64_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
//  var1 = (((int64_t)dig_P9) * ((int64_t)(((p >> 7) * (p >> 7)) >> 13))) >> 12;
//  var2 = (int64_t)(p >> 6);
//  var2 = var2 * (int64_t)dig_P8;
//  var2 = var2 >> 13;
//  p = (uint64_t)((int64_t)p + ((var1 + var2 + dig_P7) >> 0));
//  double out_p = p / 16;
//  return out_p;
//}

// Returns pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
double BMP280::bmp280_compensate_P_double(int32_t adc_P)
{
    double var1, var2, p;
    //var1 = ((double)t_fine / 2.0) - 64000.0;
    var1 = ((double)135655 / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
    var2 = var2 + var1 * ((double)dig_P5) * 2.0;
    var2 = (var2 / 4.0) + (((double)dig_P4) * 65536.0);
    var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + ((double)dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0)*((double)dig_P1);
    if (var1 == 0.0)
    {
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576.0 - (double)adc_P;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double)dig_P9) * p * p / 2147483648.0;
    var2 = p * ((double)dig_P8) / 32768.0;
    p = p + (var1 + var2 + ((double)dig_P7)) / 16.0;
    return p;
}

void BMP280::Get_Cal()
{
  byte bCal[6];
  read(0x88, 24, bCal);
  Serial.print("bCal = ");
  Serial.write(bCal, 6);
  Serial.println("");
  dig_T1 = ((uint16_t)bCal[1] << 8) + (uint16_t)bCal[0];
  dig_T2 = ((int16_t)bCal[3] << 8) + (int16_t)bCal[2];
  dig_T3 = ((int16_t)bCal[5] << 8) + (int16_t)bCal[4];
  dig_P1 = ((uint16_t)bCal[7] << 8) + (uint16_t)bCal[6];
  dig_P2 = ((int16_t)bCal[9] << 8) + (int16_t)bCal[8];
  dig_P3 = ((int16_t)bCal[11] << 8) + (int16_t)bCal[10];
  dig_P4 = ((int16_t)bCal[13] << 8) + (int16_t)bCal[12];
  dig_P5 = ((int16_t)bCal[15] << 8) + (int16_t)bCal[14];
  dig_P6 = ((int16_t)bCal[17] << 8) + (int16_t)bCal[16];
  dig_P7 = ((int16_t)bCal[19] << 8) + (int16_t)bCal[18];
  dig_P8 = ((int16_t)bCal[21] << 8) + (int16_t)bCal[20];
  dig_P9 = ((int16_t)bCal[23] << 8) + (int16_t)bCal[22];
  Serial.println("Cal values");
  Serial.println(dig_T1);
  Serial.println(dig_T2);
  Serial.println(dig_T3);
  Serial.println(dig_P1);
  Serial.println(dig_P2);
  Serial.println(dig_P3);
  Serial.println(dig_P4);
  Serial.println(dig_P5);
  Serial.println(dig_P6);
  Serial.println(dig_P7);
  Serial.println(dig_P8);
  Serial.println(dig_P9);
}

byte* BMP280 :: Get_Settings()
{
  byte b[2];
  read(0xF4, 2, b);
  return b;
}

void BMP280::Print_Values()
{
  sensorData thisData = Get_Data();
  Serial.println("Sensor");
//  Serial.println(_address);
  Serial.println(thisData.Temp);
  Serial.println(thisData.Press);
//  Serial.println(thisData.Raw_P);
//  Serial.println(thisData.Raw_T);
//  Serial.println(thisData.Pcount);
}

