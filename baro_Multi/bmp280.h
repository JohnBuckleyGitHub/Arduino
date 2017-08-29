  #ifndef BMP280_H
#define BMP280_H

#include "Arduino.h"
#include "sensorStruct.h"
//#define BMP280_Address 0x76 //Address of BMP280 sensor.

struct BMP280Data{
  long Temp;
  long Press;
  int Pcount;
  long Raw_P;
  long Raw_T;
};

class BMP280
{
  public:  
    BMP280 (byte Address);
    byte Get_Address();
    //BMP280Data Get_Data();
    sensorData Get_Data();
    float CalcAlt(float dblPress);
    void write(byte reg, byte data);
    void read(byte reg, uint8_t count, byte* data);
    double bmp280_compensate_T_double(int32_t adc_T);
    double bmp280_compensate_P_double(int32_t adc_P); 
    void Get_Cal();
    byte* Get_Settings();
    void BMP280::Print_Values();
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
    uint32_t pcount;
    const float Pb = 100000.0; //Pressure in Pa at "0 height".
    const float Lb = -0.0065; //Temp lapse rate (K/m)
    const double Tb = 298.0; //Temperature in K at sea level.
    const float hb = 0;// 0 altitude..
    const float R = 8.31432; //Universal gas constant
    const float M = 0.0289644; //Molar mass of air, kg/mol
    const float g = 9.80665; //Acceleartion due to gravity
};
#endif
