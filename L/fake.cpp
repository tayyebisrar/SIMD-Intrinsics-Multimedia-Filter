#include <iostream>
#include <fstream>
#include <vector>
#include "bmp.h"

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
    std::cout << "Input: " << infile << std::endl;
    std::cout << "Output: " << outfile << std::endl;

    // Open the BMP file in binary mode
    std::ifstream bmpFile(infile, std::ios::binary);
    if (!bmpFile.is_open()) {
        std::cerr << "Couldn't open file" << std::endl;
        return 3;
    }

    // Read the BITMAPFILEHEADER and BITMAPINFOHEADER
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    bmpFile.read(reinterpret_cast<char*>(&bf), sizeof(bf));
    bmpFile.read(reinterpret_cast<char*>(&bi), sizeof(bi));

    // Check if the file is a valid BMP file
    if (bf.bfType != 0x4D42) {
        std::cerr << "Invalid BMP file format" << std::endl;
        return 4;
    }

    // Calculate the padding per row (BMP rows must be padded to a multiple of 4 bytes)
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    size_t rowSize = bi.biWidth * sizeof(RGBTRIPLE) + padding;

    // Create a buffer to hold the pixel data
    std::vector<unsigned char> pixelData(rowSize * bi.biHeight);

    // Read the pixel data into the buffer
    for (int i = 0; i < bi.biHeight; ++i) {
        bmpFile.read(reinterpret_cast<char*>(pixelData.data() + i * rowSize), rowSize);
    }

    bmpFile.close();

    // Now, you have the pixel data in `pixelData`, and you can apply the filter.

    std::cout << "Successfully read the BMP file" << std::endl;

    // For now, let's just write the output BMP file (no filtering yet)
    std::ofstream outFile(outfile, std::ios::binary);
    if (!outFile) {
        std::cerr << "Couldn't open the output file!" << std::endl;
        return 5;
    }

    // Write the headers to the output file
    outFile.write(reinterpret_cast<char*>(&bf), sizeof(bf));
    outFile.write(reinterpret_cast<char*>(&bi), sizeof(bi));

    // Write the pixel data to the output file
    outFile.write(reinterpret_cast<char*>(pixelData.data()), pixelData.size());

    outFile.close();

    std::cout << "Output BMP file written successfully" << std::endl;

    return 0;
}
