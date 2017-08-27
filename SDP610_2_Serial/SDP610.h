#ifndef SDP610_H
#define SDP610_H

#include "Arduino.h"

class SDP610
{
  public: 
    SDP610 (byte Address, uint32_t Timeout, int Resolution);
    void SetResolution();
    void Measure();
    short Get_Data();
    void printHex(int num, int precision);
  private:
    byte _address;
    unsigned long _timeout;
    int _response;
    byte _resolution;
};

#endif
