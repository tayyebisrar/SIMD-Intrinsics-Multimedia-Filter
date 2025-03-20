#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include "bmp.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"

IMAGE_DATA loadInterleavedImage(std::string infile);
int writeInterleavedImage(std::string outfile, IMAGE_DATA out_image);

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

    std::cout << "Wrote to file with return code " << writeInterleavedImage(outfile, in_image) << std::endl;
    return 0;
}

IMAGE_DATA loadInterleavedImage(std::string infile){
    IMAGE_DATA in_image;
    in_image.channels = -1;
    unsigned char *data = stbi_load(infile.c_str(), &in_image.width, &in_image.height, &in_image.channels, 3);
    if (data == NULL)
    {
        std::cerr << "Couldn't open file" << std::endl;
        return in_image;
    }

    printf("Width: %d\n", in_image.width);
    printf("Height: %d\n", in_image.height);
    printf("Channels: %d\n", in_image.channels);

    // load interleaved rgb data into separate std::vector color channels in the IMAGE_DATA struct
    size_t totalPixels = in_image.width * in_image.height;
    in_image.red.resize(totalPixels);
    in_image.green.resize(totalPixels);
    in_image.blue.resize(totalPixels);
    for (size_t i = 0; i < totalPixels; i++)
    {
        in_image.red[i] = data[i * 3];
        in_image.green[i] = data[i * 3 + 1];
        in_image.blue[i] = data[i * 3 + 2];
    }
    stbi_image_free(data);
    return in_image;
}

int writeInterleavedImage(std::string outfile, IMAGE_DATA out_image){
    std::vector<unsigned char> data;
    for (int i = 0; i < out_image.width * out_image.height; i++)
    {
        data.push_back(out_image.red[i]);
        data.push_back(out_image.green[i]);
        data.push_back(out_image.blue[i]);
    }
    return stbi_write_bmp(outfile.c_str(), out_image.width, out_image.height, 3, data.data());
}