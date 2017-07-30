// Basic Bluetooth sketch HC-06_01
// Connect the Hc-06 module and communicate using the serial monitor
//
// The HC-06 defaults to AT mode when first powered on.
// The default baud rate is 9600
// The Hc-06 requires all AT commands to be in uppercase. NL+CR should not be added to the command string
//
 
#include "stdlib.h"
#include "string.h"

#include <SoftwareSerial.h>

const long baudRate = 115200;
static long startPacket=0xFFFFFFFF;
static long tStart=0;  //Miliseconds since logging started.
static long temp=20;   //Raw temperature
static long pBaro=1000000;  //Raw Differential Pressure
long packet32[4] = {startPacket, tStart, temp, pBaro};
uint8_t packet8[16] = { 0, 0, 0, 0, 
                        0, 0, 0, 0,
                        0, 0, 0, 0,
                        0, 0, 0, 0};
int i;
long count = 0;

SoftwareSerial BTserial(12, 13); // RX | TX
// Connect the HC-06 TX to the Arduino RX on pin 12. 
// Connect the HC-06 RX to the Arduino TX on pin 13 through a voltage divider.
// 
 
 
void setup() 
{
    Serial.begin(baudRate);
    Serial.println("Enter Text:");
 
    // HC-06 default serial speed is 9600
    BTserial.begin(baudRate);
    tStart=millis();
}
 
void loop()
{
  long tNow=millis()-tStart;
  packet32[1] = tNow;
  delay(10); 
  count++;
  packet32[3] = count;
    // Keep reading from HC-06 and send to Arduino Serial Monitor
   /* for loop execution */
   for( i = 0; i < 1000; i = i + 1 ){
      if (BTserial.available())
         {Serial.write(BTserial.read());}
      else
         {break;}
   }
    // Keep reading from Arduino Serial Monitor and send to HC-06
   for( i = 0; i < 1000; i++ ){
      if (Serial.available())
         {BTserial.write(Serial.read());}
      else
         {break;}
   }
   for( i = 0; i<4; i++ ){
    packet8[i * 4 ] = (packet32[i] ) >> 24;
    packet8[i * 4 + 1] = (packet32[i]) >> 16;
    packet8[i * 4 + 2] = (packet32[i]) >> 8;
    packet8[i * 4 + 3] = (packet32[i]);
    for(int j = 0; j< 4; j++){
      Serial.print(i);
      Serial.print("  ");
      Serial.print((i * 4 + j));
      Serial.print("  ");
      Serial.println(packet8[i * 4 + j]);
      BTserial.write(packet8[i * 4 + j]);}
   }
}


