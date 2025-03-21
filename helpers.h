#ifndef HELPERS_H
#define HELPERS_H
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

IMAGE_DATA loadInterleavedImage(std::string infile);
int writeInterleavedImage(std::string outfile, IMAGE_DATA out_image);

void filter_grayscale_basic(IMAGE_DATA &image);

#endif