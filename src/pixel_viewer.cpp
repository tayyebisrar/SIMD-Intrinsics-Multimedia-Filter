#include <iostream>
#include <fstream>
#include "helpers.h"
#include "bmpstruct.h"

int main(int argc, char* argv[])
{  
    if (argc != 2)
    {
        std::cerr << "Usage: ./pixel_viewer [filename]" << std::endl;
        return 1;
    }

    std::string infile = argv[1];

    // Create vectors to hold each color channel.
    IMAGE_DATA in_image = loadInterleavedImage(infile);
    if (in_image.channels == -1){
        std::cerr << "Error loading file " << infile << std::endl;
        return 3;
    }

    std::cout << "Enter -1 to exit." << std::endl;
    std::cout << "Image size: " << in_image.width << "x" << in_image.height << "\n";
    int x = 0;
    do{
        std::cout << "Pixel:";
        std::cin >> x;
        if (x >= 0 && x < in_image.width * in_image.height) {
            std::cout << "Red: " << (int)in_image.red[x] << " Green: " << (int)in_image.green[x] << " Blue: " << (int)in_image.blue[x] << std::endl;
        }
    } while (x != -1);

    return 0;
}
