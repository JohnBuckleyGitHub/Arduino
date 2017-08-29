#include <stdio.h>

typedef unsigned char uchar;

float bytesToFloat(uchar b0, uchar b1, uchar b2, uchar b3)
{
    float output;

    *((uchar*)(&output) + 3) = b0;
    *((uchar*)(&output) + 2) = b1;
    *((uchar*)(&output) + 1) = b2;
    *((uchar*)(&output) + 0) = b3;

    return output;
}


int main()
{
    char *b;
    float x = 7.1234567;
    * (float *) b=x;
    printf(b);
    
    float y = bytesToFloat(b[3], b[2], b[1], b[0]);
    printf("%.8f", y);
    return 0;
}