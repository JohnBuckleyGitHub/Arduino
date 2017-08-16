#include "Wire.h"
#include "stdlib.h"
#define SDP_ADD_1 0x25  //0x40 is mfg default address for SDP-6x

//Tested 20/9/15 and this works well.
//Below the address is changed to 0x60.
//Excel "Viewer.xlsm" has sheet explaining how to get MSB and LDB from desired address.

void setup() {
  delay(1000);
  int iRes=0;
  Serial.begin(115200);
  Serial.print("Starting...");
  Wire.begin();
  Wire.beginTransmission(SDP_ADD_1);
  //Write command to change address:
  byte bMessage1[5];
  bMessage1[0]=0xFA;
  bMessage1[1]=0x2C;
  bMessage1[2]=0x20;
  bMessage1[3]=0x03;  // First byte of new address
  bMessage1[4]=0x07;  // Second byte of new address
  //Send it all:
  Wire.write(bMessage1,5);
  iRes=Wire.endTransmission(true);
  Serial.print("Response =");
  Serial.println(iRes);
  //Give it some time to work - takes 10ms at least:
  delay(200);
  //Send command to reset:
  Wire.beginTransmission(SDP_ADD_1);
  Wire.write(0xFE);
  iRes=Wire.endTransmission(true);
  Serial.print("Response =");
  Serial.println(iRes);
}

void loop() {
  Serial.println("Done");
  delay(250);
}


