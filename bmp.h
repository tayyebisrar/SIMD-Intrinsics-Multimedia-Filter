// BMP-related data types based on Microsoft's own

#include <stdint.h>
#include <vector>

/**
 * Common Data Types
 *
 * The data types in this section are essentially aliases for C/C++
 * primitive data types.
 *
 * Adapted from http://msdn.microsoft.com/en-us/library/cc230309.aspx.
 * See http://en.wikipedia.org/wiki/Stdint.h for more on stdint.h.
 */
typedef uint8_t  BYTE;
typedef uint32_t DWORD;
typedef int32_t  LONG;
typedef uint16_t WORD;

/**
 * BITMAPFILEHEADER
 *
 * The BITMAPFILEHEADER structure contains information about the type, size,
 * and layout of a file that contains a DIB [device-independent bitmap].
 *
 * Adapted from http://msdn.microsoft.com/en-us/library/dd183374(VS.85).aspx.
 */
typedef struct
{
    WORD   bfType; // file type, must be 0x4d42, or 'BM' 
    DWORD  bfSize; // file size in bytes
    WORD   bfReserved1; // must be 0
    WORD   bfReserved2; // also must be 0
    DWORD  bfOffBits; // offset in bytes from the BITMAPFILEHEADER til the beginning of the BM bits
} __attribute__((__packed__))
BITMAPFILEHEADER;

/**
 * BITMAPINFOHEADER
 *
 * The BITMAPINFOHEADER structure contains information about the
 * dimensions and color format of a DIB [device-independent bitmap].
 *
 * Adapted from http://msdn.microsoft.com/en-us/library/dd183376(VS.85).aspx.
 */
typedef struct
{
    DWORD  biSize; // number of bytes required
    LONG   biWidth; // width in pixels
    LONG   biHeight; // height in pixels
    WORD   biPlanes; // color planes number, must be 1
    WORD   biBitCount; // bits per pixel
    DWORD  biCompression; // compression method
    DWORD  biSizeImage; // number of bytes in image
    LONG   biXPelsPerMeter; // resolution X, pixels per meter
    LONG   biYPelsPerMeter; // resolution Y, pixels per meter
    DWORD  biClrUsed; // number of colors used
    DWORD  biClrImportant; // number of important colors
} __attribute__((__packed__)) // GCC-specific, makes sure there's no padding between attributes of the struct
BITMAPINFOHEADER;

/**
 * RGBTRIPLE
 *
 * This structure describes a color consisting of relative intensities of
 * red, green, and blue.
 *
 * Adapted from http://msdn.microsoft.com/en-us/library/aa922590.aspx.
 */
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