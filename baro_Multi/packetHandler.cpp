#include "packetHandler.h"
#include <FastCRC.h>
#include <MS5611.h>

FastCRC16 CRC16;

const uint16_t tempByteShift = 2 << 7;
const uint16_t pressByteShift = 2 << 6;
const uint8_t clock_byte_length = 4;
const uint8_t temp_byte_length = 2;
const uint8_t press_byte_length = 3;
const uint8_t checkSumLength = 2;
const uint8_t bmp_sensorCount = 4;
const uint8_t ms_sensorCount = 4; 
const uint8_t packetLength = 1 + bmp_sensorCount * 2 + ms_sensorCount * 2;  // packetLength does not include CRC
uint8_t packetShape[packetLength];
uint8_t packetLocs[packetLength];
const unsigned int NPacketBytes = clock_byte_length + (temp_byte_length + press_byte_length) * (bmp_sensorCount + ms_sensorCount) + checkSumLength; //Number of bytes in packet
byte bStream[NPacketBytes];
int packetLoc = 0;


// NPacketBytes = 46
// [0, 4, 6, 9, 11, 14, 16, 19, 21, 24, 26, 29, 31, 34, 36, 39, 41, ]

dataPacket::dataPacket()
{
    PacketByteCount = NPacketBytes;
    packetShape[0] = clock_byte_length;
    packetLocs[0] = 0;
    for (int i = 1; i < packetLength; i += 2) {
        packetLocs[i] = packetShape[i - 1] + packetLocs[i - 1];
        packetShape[i] = temp_byte_length;
        packetLocs[i + 1] = packetShape[i] + packetLocs[i];
        packetShape[i + 1] = press_byte_length;
    }
}

void dataPacket::debugPrint()
{
    Serial.print("[");
    for (int i = 0; i < packetLength; i++) {
        Serial.print(packetLocs[i]);
        Serial.print(", ");
    }
    Serial.println("]");
    delay(500);
}

void dataPacket::debugPrint2()
{
    Serial.print("Packet [");
    for (int i = 0; i < NPacketBytes; i++) {
        Serial.print(int(bStream[i]));
        Serial.print(", ");
    }
    Serial.println("]");
    delay(1000);
}

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
        int16_t Temp = int16_t(baroData[i].Temp * tempByteShift);
        uint32_t Press = uint32_t(baroData[i].Press * pressByteShift);
        uint16ToPacketVarLoc(bStream, packetLocs[packetLoc], Temp, packetShape[packetLoc]);
        packetLoc++;
        uint32ToPacketVarLoc(bStream, packetLocs[packetLoc], Press, packetShape[packetLoc]);
        packetLoc++;
    }
}

//void dataPacket::setBaroDataBMP(uint8_t baroCount,BMP280Data *baroData)
//{
//    for (int i = 0; i < baroCount; i++) {
//        //uint16ToPacketVarLoc(bStream, packetLocs[packetLoc], baroData[i].Temp, packetShape[packetLoc]);
//        uint16ToPacketVarLoc(bStream, packetLocs[packetLoc], i, packetShape[packetLoc]);
//        packetLoc++;
//        //uint32ToPacketVarLoc(bStream, packetLocs[packetLoc], baroData[i].Press, packetShape[packetLoc]);
//        uint32ToPacketVarLoc(bStream, packetLocs[packetLoc], i*4, packetShape[packetLoc]);
//        packetLoc++;
//    }
//}

void dataPacket::setChecksum()
{
    uint16_t checksum = CRC16.ccitt(bStream, NPacketBytes - 2);
    byte b[2];
    b[0] = (checksum >> 8) & 0xFF;
    b[1] = checksum & 0xFF;
    bStream[NPacketBytes - 2] = checksum & 0xFF;
    bStream[NPacketBytes - 1] = (checksum >> 8) & 0xFF;
    /*uint16ToPacketVarLoc(bStream, packetLocs[packetLoc], b, 2);*/
}

void dataPacket::sendBTpacket(SoftwareSerial *bt_obj)
{
    bt_obj->write(bStream, NPacketBytes);
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

long dataPacket::restructBytes()
{
    long output;
    typedef unsigned char uchar;
    *((uchar*)(&output) + 3) = bStream[3];
    *((uchar*)(&output) + 2) = bStream[2];
    *((uchar*)(&output) + 1) = bStream[1]; 
    *((uchar*)(&output) + 0) = bStream[0];

    return output;
}