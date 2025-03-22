#include <iostream>
#include <vector>
#include "defaultf.h"

void filter_grayscale_basic(IMAGE_DATA &image){
    // method for grayscale
    for (int i = 0; i < image.width * image.height; i++){
        BYTE gray = (image.red[i] + image.green[i] + image.blue[i]) / 3;
        image.red[i] = gray;
        image.green[i] = gray;
        image.blue[i] = gray;
    }
}