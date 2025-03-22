#ifndef BMPSTRUCT_H
#define BMPSTRUCT_H
#include <stdint.h>
#include <vector>

typedef uint8_t  BYTE;
typedef uint32_t DWORD;
typedef int32_t  LONG;
typedef uint16_t WORD;

typedef struct
{
    BYTE  rgbtBlue;
    BYTE  rgbtGreen;
    BYTE  rgbtRed;
} __attribute__((__packed__))
RGBTRIPLE;

struct IMAGE_DATA {
    // holds a red, green and blue vector
    std::vector<BYTE> red;
    std::vector<BYTE> green;
    std::vector<BYTE> blue;
    int width;
    int height;
    int channels;
};

#endif