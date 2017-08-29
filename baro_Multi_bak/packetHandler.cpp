#include "packetHandler.h"
#include <FastCRC.h>
#include <MS5611_kj.h>

FastCRC16 CRC16;

const uint8_t clock_byte_length = 4;
const uint8_t temp_byte_length = 2;
const uint8_t press_byte_length = 3;
const uint8_t checkSumLength = 2;
const uint8_t bmp_sensorCount = 4;
const uint8_t ms_sensorCount = 4; 
const uint8_t packetLength = 1 + bmp_sensorCount * 2 + ms_sensorCount * 2;  // packetLength dos not include CRC
uint8_t packetShape[packetLength];
uint8_t packetLocs[packetLength];
// unsigned int NPacketBytes = clock_byte_length + (temp_byte_length + press_byte_length) * (bmp_sensorCount + ms_sensorCount) + checkSumLength; //Number of bytes in packet
const unsigned int NPacketBytes = 70;
byte bStream[NPacketBytes];
int packetLoc = 0;

dataPacket::dataPacket()
{
    packetShape[0] = clock_byte_length;
    packetLocs[0] = 0;
    for (int i = 1; i < packetLength; i += 2) {
        packetLocs[i] = packetShape[i - 1] + packetLocs[i - 1];
        packetShape[i] = temp_byte_length;
        packetLocs[i + 1] = packetShape[i] + packetLocs[i];
        packetShape[i + 1] = press_byte_length;
    }
}

//void dataPacket::createBtPacket(uint32_t timeToSet) {
//    printAverages();
//    resetPacketLoc();
//    setTimeData(timeToSet);
//    for (int i = 0; i < bmp_sensorCount; i++) {
//        LongToPacketVarLoc(bStream, packetLocs[packetLoc], BMP_Data[i].Temp, packetShape[packetLoc]);
//        packetLoc++;
//        LongToPacketVarLoc(bStream, packetLocs[packetLoc], BMP_Data[i].Press, packetShape[packetLoc]);
//        packetLoc++;
//    }
//    for (int j = bmp_sensorCount; j < (bmp_sensorCount + ms_sensorCount); j++) {
//        int i = j - bmp_sensorCount;
//        LongToPacketVarLoc(bStream, packetLocs[packetLoc], long(100 * ms_Data[i].Temp), packetShape[packetLoc]);
//        packetLoc++;
//        long ms_pressAvgLong = long(16 * ms_pressSum[i] / float(phases));
//        LongToPacketVarLoc(bStream, packetLocs[packetLoc], ms_pressAvgLong, packetShape[packetLoc]);
//        packetLoc++;
//        ms_pressSum[i] = 0;
//    }
//}

void dataPacket::resetPacketLoc()
{
    packetLoc = 0;
}


void dataPacket::setTimeData(uint32_t timeToSet)
// This is for a single time
{
    uint32ToPacketVarLoc(bStream, packetLocs[packetLoc], timeToSet, packetShape[packetLoc]);
    packetLoc++;
}

void dataPacket::setBaroData(uint8_t baroCount,sensorData *baroData)
{
    for (int i = 0; i < baroCount; i++) {
        uint16ToPacketVarLoc(bStream, packetLocs[packetLoc], baroData[i].Temp, packetShape[packetLoc]);
        packetLoc++;
        uint32ToPacketVarLoc(bStream, packetLocs[packetLoc], baroData[i].Press, packetShape[packetLoc]);
        packetLoc++;
    }
}

void dataPacket::setBaroDataBMP(uint8_t baroCount,BMP280Data *baroData)
{
    for (int i = 0; i < baroCount; i++) {
        uint16ToPacketVarLoc(bStream, packetLocs[packetLoc], baroData[i].Temp, packetShape[packetLoc]);
        packetLoc++;
        uint32ToPacketVarLoc(bStream, packetLocs[packetLoc], baroData[i].Press, packetShape[packetLoc]);
        packetLoc++;
    }
}

void dataPacket::setChecksum()
{
    uint16_t checksum = CRC16.ccitt(bStream, NPacketBytes - 2);
    byte b[2];
    b[0] = (checksum >> 8) & 0xFF;
    b[1] = checksum & 0xFF;
    uint16ToPacketVarLoc(b, packetLoc, checksum, 2);
    //bStream[NPacketBytes - 2] = b[0];
    //bStream[NPacketBytes - 1] = b[1];
}

void dataPacket::uint32ToPacketVarLoc(byte *bStream, int location, uint32_t value, int sbLength) {
    byte b[4];
    *(uint32_t *)b = value;
    for (int i = 0; i<sbLength; i++)
    {
        bStream[i + location] = b[i];
    }
}

void dataPacket::uint16ToPacketVarLoc(byte *bStream, int location, uint16_t value, int sbLength) {
    byte b[2];
    *(uint16_t *)b = value;
    for (int i = 0; i<sbLength; i++)
    {
        bStream[i + location] = b[i];
    }
}

void dataPacket::int16ToPacketVarLoc(byte *bStream, int location, int16_t value, int sbLength) {
    byte b[2];
    *(uint16_t *)b = value;
    for (int i = 0; i<sbLength; i++)
    {
        bStream[i + location] = b[i];
    }
}