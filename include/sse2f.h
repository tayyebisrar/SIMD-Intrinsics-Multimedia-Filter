#ifndef SSE2F_H
#define SSE2F_H

#include "bmpstruct.h"

void filter_grayscale(IMAGE_DATA &image);
void filter_blur(IMAGE_DATA &image);
void filter_brightness(IMAGE_DATA &image, int brightness);

#endif