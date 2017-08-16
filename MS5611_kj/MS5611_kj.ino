/*
  MS5611 Barometric Pressure & Temperature Sensor. Simple Example
  Read more: http://www.jarzebski.pl/arduino/czujniki-i-sensory/czujnik-cisnienia-i-temperatury-ms5611.html
  GIT: https://github.com/jarzebski/Arduino-MS5611
  Web: http://www.jarzebski.pl
  (c) 2014 by Korneliusz Jarzebski
*/

#include <Wire.h>
#include <MS5611_kj.h>
#define TCAADDR 0x70

MS5611 ms5611(0x76);

double referencePressure;

void setup() 
{
  Serial.begin(115200);
  tcaselect(2);
  // Initialize MS5611 sensor
  Serial.println("Initialize MS5611 Sensor");

  
  
  while(!ms5611.begin(MS5611_ULTRA_HIGH_RES))
  {
    Serial.println("Could not find a valid MS5611 sensor, check wiring!");
    delay(500);
  }

  // Get reference pressure for relative altitude
  referencePressure = ms5611.readPressure();
  ms5611.requestRawPressure();
  ms5611.requestRawTemperature();

  // Check settings
  checkSettings();
}

void checkSettings()
{
  Serial.print("Oversampling: ");
  Serial.println(ms5611.getOversampling());
}

void loop()
{
  // Read raw values
  ms5611.requestRawTemperature();
  uint32_t rawTemp = ms5611.readRawTemperature();
  ms5611.requestRawPressure();
  uint32_t rawPressure = ms5611.readRawPressure();;
  // Read true temperature & Pressure
  ms5611.requestRawTemperature();
  double realTemperature = ms5611.readTemperature();
  ms5611.requestRawPressure();
  double realPressure = ms5611.readPressure();
  // Calculate altitude
  double absoluteAltitude = ms5611.getAltitude(realPressure);
  double relativeAltitude = ms5611.getAltitude(realPressure, referencePressure);

  Serial.println("--");

  Serial.print(" rawTemp = ");
  Serial.print(rawTemp);
  Serial.print(", realTemp = ");
  Serial.print(realTemperature);
  Serial.println(" *C");

  Serial.print(" rawPressure = ");
  Serial.print(rawPressure);
  Serial.print(", realPressure = ");
  Serial.print(realPressure);
  Serial.println(" Pa");

  Serial.print(" absoluteAltitude = ");
  Serial.print(absoluteAltitude);
  Serial.print(" m, relativeAltitude = ");
  Serial.print(relativeAltitude);    
  Serial.println(" m");


  delay(100);
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


