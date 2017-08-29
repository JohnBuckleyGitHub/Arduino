#ifndef packetHandler_h
#define packetHandler_h

#include "Arduino.h"
#include "MS5611.h"
#include "BMP280.h"
#include <SoftwareSerial.h>

class dataPacket
{
    public:
        dataPacket(int dp_sensor_count, int bmp_sensorCount, int ms_sensorCount);
        void resetPacketLoc();
        void setTimeData(uint32_t timeToSet);
        void setBaroData(uint8_t baroCount, sensorData *baroData);
        // void setBaroDataBMP(uint8_t baroCount, BMP280Data *baroData);
        void setChecksum();
        void sendBTpacket(SoftwareSerial *bt_obj);
        void debugPrint();
        void debugPrint2();
        void uint32ToPacketVarLoc(byte *bStream, int location, uint32_t value, int sbLength);
        void uint16ToPacketVarLoc(byte *bStream, int location, uint16_t value, int sbLength);
        void int16ToPacketVarLoc(byte *bStream, int location, int16_t value, int sbLength);
        long restructBytes();
        int NPacketBytes;
    private:
        uint8_t* packetShape;
        uint8_t* packetLocs;
        byte* bStream;
        uint8_t packetLength;
        int packetLoc;
};

#endif