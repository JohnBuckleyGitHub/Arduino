// Basic Bluetooth sketch HC-06_01
// Connect the Hc-06 module and communicate using the serial monitor
//
// The HC-06 defaults to AT mode when first powered on.
// The default baud rate is 9600
// The Hc-06 requires all AT commands to be in uppercase. NL+CR should not be added to the command string
//
 
#include "Wire.h"
#include "stdlib.h"
#define SDP_ADD_1 0x40  //decimal address of sensor 1

#include <SoftwareSerial.h>

static int tStart=0;  //Miliseconds since logging started.
const int pressVoltPin = A0;  // connected to the pressure sensors output


SoftwareSerial BTserial(2, 3); // RX | TX
// Connect the HC-06 TX to the Arduino RX on pin 2. 
// Connect the HC-06 RX to the Arduino TX on pin 3 through a voltage divider.
// 
 
 
void setup() 
{
    Serial.begin(9600);
    Serial.println("Enter AT commands:");
 
    // HC-06 default serial speed is 9600
    BTserial.begin(9600);
    Wire.begin();     // create a wire object
    tStart=millis();  //Setup start time.  Not necessary as this runs at startup, but will change that later.  
}
 
void loop()
{
    float fP1 = read_SDP610(SDP_ADD_1);  //the "1" is the number 1, not letter l.. 
    float fP2 = read_SDP1000(pressVoltPin);
    float diff = fP2/fP1;
    long tNow=millis()-tStart;
    //Following concatenates an output so we get <milliseconds|pressure>
    //Converting float to string is a pain in the arse.
    //Can write directly to serial stream, but I'm trying to write to serial in a single call.
    char outstr[15];
    char outstr2[15];
    char outstr3[15];
    dtostrf(fP1,5,2,outstr);
    dtostrf(fP2,5,2,outstr2);
    dtostrf(diff,5,2,outstr3);
    String str=outstr;
    String str2=outstr2;
    String str3=outstr3;
    str.trim();
    String strResult = "time, " + String(tNow) + " , SDP600 , " + str + " , SDP1000 , " + str2+ " , diff , " + str3;
    Serial.println(strResult);
    // int bytesSent = BTserial.println(strResult);
    delay(10);
      
    // Keep reading from HC-06 and send to Arduino Serial Monitor
    if (BTserial.available())
    {  
         Serial.write(BTserial.read());
    }
 
    // Keep reading from Arduino Serial Monitor and send to HC-06
    if (Serial.available())
    {
        //
       BTserial.write(Serial.read());
    }

    int bytesSent = BTserial.print(strResult);

 
}

int read_SDP610(int address) {
  //start the communication with IC with the address xx
  Wire.beginTransmission(address); 
  //Send request to trigger measurement:
  Wire.write(0xF1);
  //end transmission
  Wire.endTransmission();
  //request to start reading:
  //request 2 bytes from address xx
  //SDP returns two bytes, most significant first.
  Wire.requestFrom(address, 2);
  // Serial.print((int) address);
  //wait for response
  while(Wire.available() == 0);
  //Read the two bytes containing result:
  byte bHigh = Wire.read();   
  byte bLow = Wire.read(); 
  //We need to convert this into a twos compliment integer.
  //Sensor is 12 bit by default, so number is first 11 bit - 12th bit
  //We put the two byes together, then use bitwise and to mask bits we dont want.to put the two together.  
  //To put bytes together, we shift high byte to the left by 8 bits, then add on low byte
  //short data type is a 16 bit value integer:
  short iResult = (bHigh<<8)|bLow;
  //Turns out this does the twos complement automatically, but if you wanted to do manually it would look like this:
 // short iResult2=(((bHigh<<8)|bLow) & 2047) - (((bHigh<<8)|bLow) & 2048);
  
  //we divide this by scale factor to get output in Pa.
  //For 25Pa sensor the scale factor is 1200.
  //For 125Pa sensor the scale factor is 240.
  //For 500Pa sensor the scale factor is 60Pa.
  // float flPressure = (130.0/138.0)*(iResult/60.0);
  return iResult;
}

float read_SDP1000(int pressVoltPin)  {
  int byteVolt = analogRead(pressVoltPin);
  // Serial.println(byteVolt);
  float flPressure = (130.0/105.0)*(500.0 * (((byteVolt/1024.0)*(3.75) - 0.25) / 3.75) + 10);
  // Serial.println(flPressure);
  return flPressure;
}

