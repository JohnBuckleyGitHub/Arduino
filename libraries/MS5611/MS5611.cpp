/*
MS5611.cpp - Class file for the MS5611 Barometric Pressure & Temperature Sensor Arduino Library.

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

#include "Arduino.h"

#include <Wire.h>
#include <math.h>

#include <MS5611.h>

bool msDebug = false;

MS5611::MS5611(byte address)
{
    MS5611_Address = address;
}

void MS5611::begin(ms5611_osr_t pressOsr, ms5611_osr_t tempOsr, int id_number)
{
    while (!initProcedure(pressOsr, tempOsr))
    {
        Serial.print("Could not find a valid MS5611 sensor at location ");
        Serial.print(id_number);
        Serial.print(", address ");
        Serial.print(MS5611_Address);
        Serial.println(", check wiring!");
        delay(500);
    }
}

bool MS5611::initProcedure(ms5611_osr_t pressOsr, ms5611_osr_t tempOsr)
{
    Wire.begin();
    reset();
    delay(100);
    readPROM();
    setOversampling(pressOsr, true);
    setOversampling(tempOsr, false);
    return true;
}

// Set oversampling value, times are in microseconds
// set isPressure to false for temperature
void MS5611::setOversampling(ms5611_osr_t osr, bool isPressure)
{
    switch (osr)
    {
	case MS5611_ULTRA_LOW_POWER:
	    ct = 600;
	    break;
	case MS5611_LOW_POWER:
	    ct = 1170;
	    break;
	case MS5611_STANDARD:
	    ct = 2280;
	    break;
	case MS5611_HIGH_RES:
	    ct = 4540;
	    break;
	case MS5611_ULTRA_HIGH_RES:
	    ct = 9040;
	    break;
    }
    if (isPressure == true){
        p_uosr = osr;
        p_ct = ct;}
    else{
        t_uosr = osr;
        t_ct = ct;}
}

// Get oversampling value
// ms5611_osr_t MS5611::getOversampling(void)
// {
//     return (ms5611_osr_t)uosr;
// }

void MS5611::reset(void)
{
    Wire.beginTransmission(MS5611_Address);

    Wire.write(MS5611_CMD_RESET);
    requestState = REQUEST_NONE;
    Wire.endTransmission();
}

void MS5611::readPROM(void)
{
    for (uint8_t offset = 0; offset < 6; offset++)
    {
	fc[offset] = readRegister16(MS5611_CMD_READ_PROM + (offset * 2));
    }
}

sensorData MS5611::InitDataStruct(void)
{
GetData(false, true);
return lastData;
}


void MS5611::GetData(bool reuseTemp, bool requestPress)
{
  lastData.Raw_P = readRawPressure();
  if (reuseTemp != true)
  {
    lastData.Raw_T = readRawTemperature();
    lastData.Temp = convertTemperature(true, lastData.Raw_T);
    }
  lastData.Press = convertPressure(true, lastData.Raw_P, lastData.Raw_T);
  lastData.Pcount++;
  if (requestPress == true) {
    requestRawPressure();}
  else {
    requestRawTemperature();}
}

void MS5611::PrintData(int location)
{
    Serial.print("Location, Address, Pressure, & Temp = (");
    Serial.print(location);
    Serial.print(", ");
    Serial.print(MS5611_Address);
    Serial.print(", ");
    Serial.print(lastData.Press);
    Serial.print(", ");
    Serial.print(lastData.Temp);
    Serial.print(")");
}

void MS5611::UpdateTempData(bool requestPress)
{
    lastData.Raw_T = readRawTemperature();
    lastData.Temp = convertTemperature(true, lastData.Raw_T);
    if (requestPress == true) {
        requestRawPressure();}
    else {
        requestRawTemperature();}
}

void MS5611::requestRawTemperature(void)
{
    Wire.beginTransmission(MS5611_Address);
    Wire.write(MS5611_CMD_CONV_D2 + t_uosr);
    requestState = REQUEST_TEMPERATURE;
    Wire.endTransmission();
    requestTime = micros();
}

uint32_t MS5611::readRawTemperature(void)
{
    if (requestState != REQUEST_TEMPERATURE) {
        requestRawTemperature();
        if (msDebug == true) {
            Serial.println("Temperature auto-request");
        }
    }
    uint32_t duration = micros() - requestTime;
    if (duration < t_ct){
        if (msDebug == true) {
            Serial.print("Time delay on Temp = ");
            Serial.println(t_ct - duration);
        }
        delayMicroseconds(t_ct - duration);
    }

    return readRegister24(MS5611_CMD_ADC_READ);
}

void MS5611::requestRawPressure(void)
{
    Wire.beginTransmission(MS5611_Address);
    Wire.write(MS5611_CMD_CONV_D1 + p_uosr);
    requestState = REQUEST_PRESSURE;
    Wire.endTransmission();
    requestTime = micros();
}

uint32_t MS5611::readRawPressure(void)
{
    if (requestState != REQUEST_PRESSURE) {
        requestRawPressure();
        if (msDebug == true) {
            Serial.println("Pressure auto-request");
        }
    }
    uint32_t duration = micros() - requestTime;
    if (duration < p_ct){
        if (msDebug == true) {
            Serial.print("Time delay on Pressure = ");
            Serial.println(p_ct - duration);
        }
        delayMicroseconds(p_ct - duration);
    }
    return readRegister24(MS5611_CMD_ADC_READ);
}

// int32_t MS5611::readPressure(bool compensation)
double MS5611::readPressure(bool compensation)
{
    uint32_t D1 = readRawPressure();
    uint32_t D2 = readRawTemperature();
    return convertPressure(compensation, D1, D2);
}


double MS5611::convertPressure(bool compensation, uint32_t D1, uint32_t D2)
{
    int32_t dT = D2 - (uint32_t)fc[4] * 256;

    int64_t OFF = (int64_t)fc[1] * 65536 + (int64_t)fc[3] * dT / 128;
    int64_t SENS = (int64_t)fc[0] * 32768 + (int64_t)fc[2] * dT / 256;

    if (compensation)
    {
	int32_t TEMP = 2000 + ((int64_t) dT * fc[5]) / 8388608;

	OFF2 = 0;
	SENS2 = 0;

	if (TEMP < 2000)
	{
	    OFF2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 2;
	    SENS2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 4;
	}

	if (TEMP < -1500)
	{
	    OFF2 = OFF2 + 7 * ((TEMP + 1500) * (TEMP + 1500));
	    SENS2 = SENS2 + 11 * ((TEMP + 1500) * (TEMP + 1500)) / 2;
	}

	OFF = OFF - OFF2;
	SENS = SENS - SENS2;
    }

    double P = (D1 * SENS / 2097152 - OFF) / 32768.0;

    return P;
}

double MS5611::readTemperature(bool compensation)
{
    uint32_t D2 = readRawTemperature();
    return convertTemperature(compensation, D2);
}


double MS5611::convertTemperature(bool compensation, uint32_t D2)
{
    int32_t dT = D2 - (uint32_t)fc[4] * 256;

    int32_t TEMP = 2000 + ((int64_t) dT * fc[5]) / 8388608;

    TEMP2 = 0;

    if (compensation)
    {
	if (TEMP < 2000)
	{
	    uint32_t longTwo = 2;
        TEMP2 = (dT * dT) / (longTwo << 30);
	}
    }

    TEMP = TEMP - TEMP2;

    return ((double)TEMP/100);
}

// Calculate altitude from Pressure & Sea level pressure
double MS5611::getAltitude(double pressure, double seaLevelPressure)
{
    return (44330.0f * (1.0f - pow((double)pressure/ (double)seaLevelPressure, 0.1902949f)));
}

// Calculate sea level from Pressure given on specific altitude
double MS5611::getSeaLevel(double pressure, double altitude)
{
    return ((double)pressure / pow(1.0f - ((double)altitude / 44330.0f), 5.255f));
}

// Read 16-bit from register (oops MSB, LSB)
uint16_t MS5611::readRegister16(uint8_t reg)
{
    uint16_t value;
    Wire.beginTransmission(MS5611_Address);
        Wire.write(reg);
        requestState = REQUEST_NONE;
    Wire.endTransmission();
    
    Wire.beginTransmission(MS5611_Address);
    uint8_t qty = 2;
    Wire.requestFrom(MS5611_Address, qty);
    while(!Wire.available()) {Serial.println(MS5611_Address);};
        uint8_t vha = Wire.read();
        uint8_t vla = Wire.read();
    Wire.endTransmission();
    value = vha << 8 | vla;
    return value;
}

// Read 24-bit from register (oops XSB, MSB, LSB)
uint32_t MS5611::readRegister24(uint8_t reg)
{
    uint32_t value;
    Wire.beginTransmission(MS5611_Address);
        Wire.write(reg);
    Wire.endTransmission();
    Wire.beginTransmission(MS5611_Address);
    uint8_t qty = 3;
    Wire.requestFrom(MS5611_Address, qty);
    while(!Wire.available()) {};
        uint8_t vxa = Wire.read();
        uint8_t vha = Wire.read();
        uint8_t vla = Wire.read();
    Wire.endTransmission();

    value = ((int32_t)vxa << 16) | ((int32_t)vha << 8) | vla;

    return value;
}
