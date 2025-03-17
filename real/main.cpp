#include <iostream>
#include <fstream>
#include <vector>
#include "bmp.h"

void parseBitmapSoA(std::ifstream &bmpFile, const BITMAPINFOHEADER &bi, std::vector<BYTE> &red, std::vector<BYTE> &green, std::vector<BYTE> &blue);
void displayImageInfo(const BITMAPFILEHEADER &bf, const BITMAPINFOHEADER &bi);
void writeBitmapBack(std::ofstream &outFile, const BITMAPFILEHEADER &bf, const BITMAPINFOHEADER &bi, std::vector<BYTE> &red, std::vector<BYTE> &green, std::vector<BYTE> &blue);

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: ./filter [flag] infile outfile" << std::endl;
        return 1;
    }

    std::string infile = argv[2];
    std::string outfile = argv[3];
    char flag = argv[1][0];
    if (!(flag == 'b' || flag == 'e' || flag == 'g' || flag == 'r'))
    {
        std::cerr << "Invalid filter." << std::endl;
        return 2;
    }
    std::cout << "Flag: " << flag << std::endl;
    std::cout << "Infile: " << infile << std::endl;
    std::cout << "Outfile: " << outfile << std::endl;

    std::ifstream bmpFile(infile, std::ios::binary);
    if (!bmpFile.is_open()) {
        std::cerr << "Couldn't open file" << std::endl;
        return 3;
    }
    BITMAPFILEHEADER bf;
    bmpFile.read(reinterpret_cast<char*>(&bf), sizeof(bf));
    BITMAPINFOHEADER bi;
    bmpFile.read(reinterpret_cast<char*>(&bi), sizeof(bi));
    

    // Ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bi.biCompression != 0 || bi.biBitCount != 24 ) {
        std::cerr << "Invalid BMP file format" << std::endl;
        return 4;
    }
    
    // TODO: Figure out why this doesn't work
    // Adjust the file pointer to the start of the pixel data
    bmpFile.seekg(bf.bfOffBits, std::ios::beg);

    // Create vectors to hold each color channel.
    IMAGE_DATA in_image;
    parseBitmapSoA(bmpFile, bi, in_image.red, in_image.green, in_image.blue);
    bmpFile.close();
    
    std::cout << "First pixel values:\nRed: " << static_cast<int>(in_image.red[0]) << "\nGreen: " << static_cast<int>(in_image.green[0]) << "\nBlue: " << static_cast<int>(in_image.blue[0]) << std::endl;
    
    std::ofstream out(outfile, std::ios::binary);
    if (!(out.is_open())){
        std::cerr << "Couldn't open file" << std::endl;
        return 5;
    }

    writeBitmapBack(out, bf, bi, in_image.red, in_image.green, in_image.blue);
    out.close();

    return 0;
}

void parseBitmapSoA(std::ifstream &bmpFile, const BITMAPINFOHEADER &bi, std::vector<BYTE> &red, std::vector<BYTE> &green, std::vector<BYTE> &blue){
    int width = bi.biWidth;
    int height = std::abs(bi.biHeight);
    // Calculate the padding added at the end of each row in the BMP file.
    int padding = (4 - (width * sizeof(RGBTRIPLE)) % 4) % 4;

    // Pre-allocate vectors to hold each channel.
    red.resize(width * height);
    green.resize(width * height);
    blue.resize(width * height);

    // Iterate over each scanline (row)
    for (int i = 0; i < height; i++) {
        // Process each pixel in the row
        int rowOffset = i * width;
        for (int j = 0; j < width; j++) {
            RGBTRIPLE pixel;
            bmpFile.read(reinterpret_cast<char*>(&pixel), sizeof(RGBTRIPLE));
            int index = rowOffset + j;
            red[index] = pixel.rgbtRed;
            green[index] = pixel.rgbtGreen;
            blue[index] = pixel.rgbtBlue;
        }
        // Skip the padding bytes at the end of the row
        bmpFile.seekg(padding, std::ios::cur);
    }
}

void writeBitmapBack(std::ofstream &outFile, const BITMAPFILEHEADER &bf, const BITMAPINFOHEADER &bi, std::vector<BYTE> &red, std::vector<BYTE> &green, std::vector<BYTE> &blue){
    outFile.write(reinterpret_cast<const char*>(&bf), sizeof(bf));
    outFile.write(reinterpret_cast<const char*>(&bi), sizeof(bi));
    int width = bi.biWidth;
    int height = std::abs(bi.biHeight);
    int padding = (4 - (width * sizeof(RGBTRIPLE)) % 4) % 4;
    for (int i = 0; i < height; i++) {
        int rowOffset = i * width;
        for (int j = 0; j < width; j++) {
            RGBTRIPLE pixel;
            pixel.rgbtRed = red[rowOffset + j];
            pixel.rgbtGreen = green[rowOffset + j];
            pixel.rgbtBlue = blue[rowOffset + j];
            outFile.write(reinterpret_cast<const char*>(&pixel), sizeof(RGBTRIPLE));
        }
        // Write padding bytes
        for (int k = 0; k < padding; k++) {
            outFile.put(0x00);
        }
    }
}

void displayImageInfo(const BITMAPFILEHEADER &bf, const BITMAPINFOHEADER &bi){
    std::cout << "bfType: " << bf.bfType << std::endl;
    std::cout << "bfSize: " << bf.bfSize << std::endl;
    std::cout << "bfReserved1: " << bf.bfReserved1 << std::endl;
    std::cout << "bfReserved2: " << bf.bfReserved2 << std::endl;
    std::cout << "bfOffBits: " << bf.bfOffBits << std::endl;
    std::cout << "biSize: " << bi.biSize << std::endl;
    std::cout << "biWidth: " << bi.biWidth << std::endl;
    std::cout << "biHeight: " << bi.biHeight << std::endl;
    std::cout << "biPlanes: " << bi.biPlanes << std::endl;
    std::cout << "biBitCount: " << bi.biBitCount << std::endl;
    std::cout << "biCompression: " << bi.biCompression << std::endl;
    std::cout << "biSizeImage: " << bi.biSizeImage << std::endl;
    std::cout << "biXPelsPerMeter: " << bi.biXPelsPerMeter << std::endl;
    std::cout << "biYPelsPerMeter: " << bi.biYPelsPerMeter << std::endl;
    std::cout << "biClrUsed: " << bi.biClrUsed << std::endl;
    std::cout << "biClrImportant: " << bi.biClrImportant << std::endl;
}



