#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include "helpers.h"

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
    if (!(flag == 'b' || flag == 'e' || flag == 'g')){
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
    
    int x = 0;
    do{
        printf("Pixel: ");
        scanf("%d", &x);
        if (x >= 0 && x < in_image.width * in_image.height){
            printf("Red: %d\n", in_image.red[x]);
            printf("Green: %d\n", in_image.green[x]);
            printf("Blue: %d\n", in_image.blue[x]);
        }
    }while (x >= 0 && x < in_image.width * in_image.height);

    switch (flag) {
        case 'b':
            // filter_blur_basic(in_image);
            break;
        case 'e':
            // filter_edge_basic(in_image);
            break;
        case 'g':
            filter_grayscale_basic(in_image);
            break;
        default:
            break;
    }
    std::cout << "Successfully wrote to file with return code " << writeInterleavedImage(outfile, in_image) << std::endl;
    return 0;
}
