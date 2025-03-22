#include <iostream>
#include <vector>
#include "helpers.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"
#include "../lib/stb/stb_image_write.h"

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