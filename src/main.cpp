#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include "helpers.h"
#include "defaultf.h"

#if defined(__AVX2__)
    #define instrset "AVX2"
    #include "avx2f.h"
#elif defined(__SSE2__)
    #define instrset "SSE2"
    #include "sse2f.h"
#else
    #define instrset "none"
#endif

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
    if (!(flag == 'b' || flag == 'l' || flag == 'g')){
        std::cerr << "Invalid filter." << std::endl;
        return 2;
    }
    std::cout << "Flag: " << flag << std::endl;
    std::cout << "Infile: " << infile << std::endl;
    std::cout << "Outfile: " << outfile << std::endl;

    // Create vectors to hold each color channel.
    IMAGE_DATA in_image = loadInterleavedImage(infile);
    if (in_image.channels == -1){
        std::cerr << "Error loading file " << infile << std::endl;
        return 3;
    }
    
    switch (flag) {
        case 'b': // blur
            if (std::string(instrset) == "none"){
                filter_blur_basic(in_image);
            }
            else {
                filter_blur(in_image);
            }
            break;
        case 'l': // brightness
            int brightness;
            std::cout << "Enter brightness adjustment value (positive or negative): ";
            std::cin >> brightness;
            if (std::string(instrset) == "none"){
                filter_brightness_basic(in_image, brightness);
            }
            else {
                filter_brightness(in_image, brightness);
            }
            break;
        case 'g':
            if (std::string(instrset) == "none"){
                filter_grayscale_basic(in_image);
            }
            else {
                filter_grayscale(in_image);
            }
            break;
        default:
            break;
    }
    std::cout << "Successfully wrote to file with return code " << writeInterleavedImage(outfile, in_image) << " using " << std::string(instrset) << " intrinsics " << std::endl;
    return 0;
}
