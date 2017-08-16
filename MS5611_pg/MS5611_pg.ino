#include <MS5611_pg.h>

MS5611 baro;
int32_t pressure;
double temperature;
uint32_t rawTemperature;

void setup() {
  // Start barometer
  baro = MS5611();
  baro.begin();
  // Start serial (UART)
  Serial.begin(9600);
  delay(2);
}

void loop() {
  // Read pressure
  pressure = baro.getPressure();
  // Send pressure via serial (UART);
  Serial.println(pressure);
  temperature = baro.getTemperature();
  rawTemperature = baro.getRawTemperature();
  // Send pressure via serial (UART);
  Serial.println(rawTemperature);
  Serial.println(temperature);
}
