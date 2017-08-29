#ifndef sensorStruct_h
#define sensorStruct_h

struct sensorData {
    long Pcount;
    double Temp;
    double Press;
    uint32_t Raw_P;
    uint32_t Raw_T;
};

#endif