#ifndef DEFAULTF_H
#define DEFAULTF_H

#include "bmpstruct.h"

void filter_grayscale_basic(IMAGE_DATA &image);
void filter_blur_basic(IMAGE_DATA &image);
void filter_brightness_basic(IMAGE_DATA &image, int brightness);

#endif

