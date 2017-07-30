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
int i;
unsigned char x;

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
}
 
void loop()
{
  delay(200); 
    // Keep reading from HC-06 and send to Arduino Serial Monitor
   /* for loop execution */
   for( i = 0; i < 1000; i = i + 1 ){
      if (BTserial.available())
         {Serial.write(BTserial.read());}
      else
         {break;}
   }
    // Keep reading from Arduino Serial Monitor and send to HC-06
   for( i = 0; i < 1000; i = i + 1 ){
      if (Serial.available())
         {BTserial.write(Serial.read());}
      else
         {break;}
   }
   x = 256;
   BTserial.write(x);
   BTserial.write(x+1);
}


