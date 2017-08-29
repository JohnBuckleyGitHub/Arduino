#ifndef packetHandler_h
#define packetHandler_h

#include "Arduino.h"
#include "MS5611_kj.h"
#include "BMP280.h"

class dataPacket
{
    public:
        dataPacket();
        void resetPacketLoc();
        void setTimeData(uint32_t timeToSet);
        void setBaroData(uint8_t baroCount, sensorData *baroData);
        void setBaroDataBMP(uint8_t baroCount, BMP280Data *baroData);
        void setChecksum();
        void uint32ToPacketVarLoc(byte *bStream, int location, uint32_t value, int sbLength);
        void uint16ToPacketVarLoc(byte *bStream, int location, uint16_t value, int sbLength);
        void int16ToPacketVarLoc(byte *bStream, int location, int16_t value, int sbLength);
        const unsigned int NPacketBytes;


};

#endif