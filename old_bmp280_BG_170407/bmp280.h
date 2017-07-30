#ifndef BMP280_H
#define BMP280_H

#include "Arduino.h"
//#define BMP280_Address 0x76 //Address of BMP280 sensor.

struct BMP280Data{
  long Temp;
  long Press;
};

class BMP280
{
  public:  
    BMP280 (byte Address);
    BMP280Data Get_Data();
    float CalcAlt(float dblPress);
    void write(byte reg, byte data);
    void read(byte reg, int count, byte* data);
    int32_t bmp280_compensate_T_int32(int32_t adc_T);
    long bmp280_compensate_P_int32(int32_t adc_P); 
    void Get_Cal();
    byte* Get_Settings();
  private:
    byte _address;
    int32_t t_fine;
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    const float Pb = 100000.0; //Pressure in Pa at "0 height".
    const float Lb = -0.0065; //Temp lapse rate (K/m)
    const double Tb = 298.0; //Temperature in K at sea level.
    const float hb = 0;// 0 altitude..
    const float R = 8.31432; //Universal gas constant
    const float M = 0.0289644; //Molar mass of air, kg/mol
    const float g = 9.80665; //Acceleartion due to gravity
};
#endif
