/*http://www.electroschematics.com/9798/reading-temperatures-i2c-arduino/ */


#include "Wire.h"
#include "stdlib.h"
#define SDP_ADD_1 0x40  //decimal address of sensor 1
static int tStart=0;  //Miliseconds since logging started.

void setup() {
  Serial.begin(9600);
  Wire.begin();     // create a wire object
  tStart=millis();  //Setup start time.  Not necessary as this runs at startup, but will change that later.
}
 
void loop() {
  float fP1 = read_SDP610(SDP_ADD_1);  //the "1" is the number 1, not letter l.. 
  int tNow=millis()-tStart;
  //Following concatenates an output so we get <milliseconds|pressure>
  //Converting float to string is a pain in the arse.
  //Can write directly to serial stream, but I'm trying to write to serial in a single call.
  char outstr[15];
  dtostrf(fP1,5,2,outstr);
  String str=outstr;
  str.trim();
  String strResult = "<" + String(tNow) + "|" + str + ">";
  Serial.print(strResult);
  delay(10);  
}
 
float read_SDP610(int address) {
  //start the communication with IC with the address xx
  Wire.beginTransmission(address); 
  //Send request to trigger measurement:
  Wire.write(0xF);
  //end transmission
  Wire.endTransmission();
  //request to start reading:
  //request 2 bytes from address xx
  //SDP returns two bytes, most significant first.
  Wire.requestFrom(address, 2);
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
  float flPressure = iResult/1200.0;
  return flPressure;
}

