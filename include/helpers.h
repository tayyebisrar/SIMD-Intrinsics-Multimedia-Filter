#ifndef HELPERS_H
#define HELPERS_H

#include "bmpstruct.h"

IMAGE_DATA loadInterleavedImage(std::string infile);
int writeInterleavedImage(std::string outfile, IMAGE_DATA out_image);

#endif