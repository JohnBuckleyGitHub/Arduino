#include "Arduino.h"
#include "SDP610.h"
#include "Wire.h"

SDP610::SDP610(byte Address, uint32_t Timeout, int Resolution)
{
  Wire.begin();
  _address = Address;
  _resolution = Resolution;
}

void SDP610::SetResolution()
{
  byte bMSB; //most significant byte
  byte bLSB; //least significant byte
  Wire.begin();
  Wire.beginTransmission(_address);
  Wire.write(0xE5);
  Wire.endTransmission();
  Wire.requestFrom(_address,2);
  bool bFailed=false;
  //Wait for response up to the allotted timeout:
  unsigned long start_time = millis();
  while(Wire.available() == 0)
  {
    if ((millis() - start_time) > _timeout) 
    {
      bFailed=true;
      break;
    }
  }
  if (bFailed){return;}
  
  bMSB = Wire.read();
  bLSB = Wire.read();
  //Change values:
  bMSB = (bMSB & (255-(7<<1))) + ((_resolution-9)<<1);

  //Write new values:
  byte bMessage[3];
  bMessage[0] = 0xE4; //Command to change resolution.
  bMessage[1] = bMSB;
  bMessage[2] = bLSB;
  Wire.beginTransmission(_address);
  Wire.write(bMessage,3);
  Wire.endTransmission(true);
  //Pause to check it works:
  delay(100);
}

void SDP610::Measure()
{
  //Read short (signed 16 bit integer) value from sensor as two separate bytes.
  Wire.beginTransmission(_address);
  //Send command to trigger measurement:
  Wire.write(0xF1);
  Wire.endTransmission();
}

short SDP610::Get_Data()
{
  //Assume that the trigger measurement command has already been sent via Measure routine.
  //Request two bytes from sensor at _address
  byte bLSB; //least significant byte
  byte bMSB; //most significant byte
  Wire.requestFrom(_address,2);
  bool bFailed=false;
  unsigned long start_time = millis();
  //Wait for response up to the allotted timeout:
  while(Wire.available() == 0)
  {
    if ((millis() - start_time) > _timeout) 
    {
      bFailed=true;
      break;
    }
  }
  if (!bFailed)
  {
    bMSB = Wire.read();
    bLSB = Wire.read();
  }
  else
  {
    bMSB=0;
    bLSB=0;
  }
  short _result = (bMSB<<8)|bLSB;
  return _result;
}

void SDP610::printHex(int num, int precision) {
     char tmp[16];
     char format[128];

     sprintf(format, "0x%%.%dX", precision);

     sprintf(tmp, format, num);
     Serial.println(tmp);
}
