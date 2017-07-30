// Basic Bluetooth sketch HC-06_01
// Connect the Hc-06 module and communicate using the serial monitor
//
// The HC-06 defaults to AT mode when first powered on.
// The default baud rate is 9600
// The Hc-06 requires all AT commands to be in uppercase. NL+CR should not be added to the command string
//
 
#include "Wire.h"
#include "stdlib.h"
#include "string.h"
#define SDP_ADD_1 0x40  //decimal address of sensor 1

#include <SoftwareSerial.h>

static int tStart=0;  //Miliseconds since logging started.
const int pressVoltPin = A0;  // connected to the pressure sensors output
const long baudRate = 115200;
//const long baudRate = 9600;

SoftwareSerial BTserial(2, 3); // RX | TX
// Connect the HC-06 TX to the Arduino RX on pin 2. 
// Connect the HC-06 RX to the Arduino TX on pin 3 through a voltage divider.
// 
 
 
void setup() 
{
    Serial.begin(baudRate);
    Serial.println("Enter AT commands:");
 
    // HC-06 default serial speed is 9600
    BTserial.begin(baudRate);
    Wire.begin();     // create a wire object
    tStart=millis();  //Setup start time.  Not necessary as this runs at startup, but will change that later.  
}
 
void loop()
{
    if(tStart < 99){
      tStart++;
    } else {
      tStart = 0;
    }
    char chount[2] = {toascii(tStart + 0), '\0'};
    char ss[2];
//    strcpy(ss, "00");
//    strcpy(ss, chount);
//    strcat(ss, "00");
//    Serial.write(00);
//    Serial.println(ss);
    Serial.write("z");
        delay(200);
    Serial.write("o");
            delay(200);
    Serial.println("x");
//    Serial.write(00);
    // int bytesSent = BTserial.println(strResult);
    delay(200);
      
    // Keep reading from HC-06 and send to Arduino Serial Monitor
    if (BTserial.available())
    {  
         Serial.write(BTserial.read());
         Serial.println("for fuck's sake!!!!!");
    }
 
    // Keep reading from Arduino Serial Monitor and send to HC-06
    if (Serial.available())
    {
        //
       BTserial.write(Serial.read());
    }

    // int bytesSent = BTserial.print(ss);
    // Serial.print(bytesSent, DEC);
    

 
}


