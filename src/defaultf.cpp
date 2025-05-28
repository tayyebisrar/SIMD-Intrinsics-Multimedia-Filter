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

void filter_blur_basic(IMAGE_DATA &image){
    IMAGE_DATA temporary_image = image; // create a temp image - we can't adjust the pixels in place since surrounding pixels still need the original values
    const int box_radius = 1;
    // simple 3x3 box blur filter
    int r_sum, g_sum, b_sum, count;
    for (int i = 0; i < image.width * image.height; i++){
        r_sum = 0;
        g_sum = 0;
        b_sum = 0;
        count = 0;

        // check for 3x3 box around the pixel. In a 2d array, this is done by subtracting and adding 1 to the current pixel and checking if it's in the bounds.
        // However, since this is now a 1d array, we check the index of the pixel, and the pixel subtract and add 1, and then also the pixel subtract and add the width of the image, along with the pairs pixels next to them..
        // If it is in bounds, add the pixel to the sum and increment the count. If not, skip it. 
        for (int j = -box_radius; j <= box_radius; j++){
            for (int k = -box_radius; k <= box_radius; k++){
                int x = i + j * image.width + k; // current pixel + row offset + column offset
                if (x >= 0 && x < image.width * image.height){
                    if ((x/image.width) != (i / image.width)){
                        continue; // if the pixel is not in the right row when it should be (a column offset error), skip it
                    }
                    r_sum += image.red[x];
                    g_sum += image.green[x];
                    b_sum += image.blue[x];
                    count++;
                }
            }
        }

        temporary_image.red[i] = (r_sum / count);
        temporary_image.green[i] = (g_sum / count);
        temporary_image.blue[i] = (b_sum / count);
    }
    image = temporary_image; // copy the temp image back into the original
}

