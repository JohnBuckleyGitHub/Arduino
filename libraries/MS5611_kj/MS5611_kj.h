/*
MS5611.h - Header file for the MS5611 Barometric Pressure & Temperature Sensor Arduino Library.

Version: 1.0.0
(c) 2014 Korneliusz Jarzebski
www.jarzebski.pl

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MS5611_kj_h
#define MS5611_kj_h

#include "Arduino.h"

// #define MS5611_ADDRESS                (0x76)

#define MS5611_CMD_ADC_READ           (0x00)
#define MS5611_CMD_RESET              (0x1E)
#define MS5611_CMD_CONV_D1            (0x40)
#define MS5611_CMD_CONV_D2            (0x50)
#define MS5611_CMD_READ_PROM          (0xA2)

typedef enum
{
    MS5611_ULTRA_HIGH_RES   = 0x08,
    MS5611_HIGH_RES         = 0x06,
    MS5611_STANDARD         = 0x04,
    MS5611_LOW_POWER        = 0x02,
    MS5611_ULTRA_LOW_POWER  = 0x00
} ms5611_osr_t;

typedef enum
{
    REQUEST_NONE      = 0,
    REQUEST_PRESSURE        = 1,
    REQUEST_TEMPERATURE     = 2
} requestType;

struct MS5611Data{
  int Pcount;
  double Temp;
  double Press;
  uint32_t Raw_P;
  uint32_t Raw_T;
};

class MS5611
{
    public:
    MS5611(byte Address);
    // bool begin(ms5611_osr_t osr = MS5611_ULTRA_HIGH_RES);
    byte MS5611_Address;
    // bool fastCycle;  // if true, conversion request must be made manually
    MS5611Data InitDataStruct(void);
    // MS5611Data myData;
    void GetData(MS5611Data& myData, bool reuseTemp, bool requestPress);
    void UpdateTempData(MS5611Data& myData, bool requestPress);
    bool begin(ms5611_osr_t pressOsr, ms5611_osr_t tempOsr);
    void requestRawTemperature(void);
    void requestRawPressure(void);
	uint32_t readRawTemperature(void);
	uint32_t readRawPressure(void);
	double readTemperature(bool compensation = true);
	double readPressure(bool compensation = true);
    double convertTemperature(bool compensation, uint32_t D1);
    double convertPressure(bool compensation, uint32_t D1, uint32_t D2);
	double getAltitude(double pressure, double seaLevelPressure = 101325);
	double getSeaLevel(double pressure, double altitude);
	void setOversampling(ms5611_osr_t osr, bool isPressure);
	ms5611_osr_t getOversampling(void);

    private:

	uint16_t fc[6];
    uint16_t ct;
    uint16_t p_ct;
	uint16_t t_ct;
    uint8_t p_uosr;
	uint8_t t_uosr;
	int32_t TEMP2;
	int64_t OFF2, SENS2;

	void reset(void);
	void readPROM(void);

	uint16_t readRegister16(uint8_t reg);
	uint32_t readRegister24(uint8_t reg);
    uint32_t requestTime;
    uint8_t requestState;

};

#endif
