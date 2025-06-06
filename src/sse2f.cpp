#include <emmintrin.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "sse2f.h"

void filter_grayscale(IMAGE_DATA &image){
    int totalpixels = image.width * image.height;
    int totalloops = totalpixels / 16;

    const __m128i red_multiplier = _mm_set1_epi16(77); // roughly 0.2989 * 256
    const __m128i green_multiplier = _mm_set1_epi16(150); // roughly 0.5870 * 256
    const __m128i blue_multiplier = _mm_set1_epi16(29); // roughly 0.1140 * 256

    for (int i=0; i<totalloops; i++){
        // integer division by 3 is not possible, so will use grayscale = 0.2989×R+0.5870×G+0.1140×B 
        // load in 16 pixels at a time
        __m128i red8 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.red.data() + i*16));
        __m128i green8 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.green.data() + i*16));
        __m128i blue8 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.blue.data() + i*16));

        // convert to 16 bit integers because otherwise the multiplication would overflow
        __m128i red16l = _mm_unpacklo_epi8(red8, _mm_setzero_si128());
        __m128i green16l = _mm_unpacklo_epi8(green8, _mm_setzero_si128());
        __m128i blue16l = _mm_unpacklo_epi8(blue8, _mm_setzero_si128());
        __m128i red16h = _mm_unpackhi_epi8(red8, _mm_setzero_si128());
        __m128i green16h = _mm_unpackhi_epi8(green8, _mm_setzero_si128());
        __m128i blue16h = _mm_unpackhi_epi8(blue8, _mm_setzero_si128());

        // so for example red16l looks like r0, 0, r1, 0, r2, 0, r3, 0, r4, 0, r5, 0, r6, 0, r7, 0
        // and red16h looks like r8, 0, r9, 0, r10, 0, r11, 0, r12, 0, r13, 0, r14, 0, r15, 0

        // set gray values to rgb multiplied by the coefficients
        __m128i gray16l = _mm_add_epi16(_mm_add_epi16(_mm_mullo_epi16(blue16l, blue_multiplier),_mm_mullo_epi16(green16l, green_multiplier)), _mm_mullo_epi16(red16l, red_multiplier));
        __m128i gray16h = _mm_add_epi16(_mm_add_epi16(_mm_mullo_epi16(blue16h, blue_multiplier),_mm_mullo_epi16(green16h, green_multiplier)), _mm_mullo_epi16(red16h, red_multiplier));

            // Divide by 256 (shift right by 8 bits)
        gray16l = _mm_srli_epi16(gray16l, 8);
        gray16h = _mm_srli_epi16(gray16h, 8);

        // pack the 16 bit integers back to 8 bit integers
        __m128i gray8 = _mm_packus_epi16(gray16l, gray16h);

        // store the values back to the image
        _mm_storeu_si128(reinterpret_cast<__m128i*>(image.red.data() + i * 16), gray8);
        _mm_storeu_si128(reinterpret_cast<__m128i*>(image.green.data() + i * 16), gray8);
        _mm_storeu_si128(reinterpret_cast<__m128i*>(image.blue.data() + i * 16), gray8);

    }
    for (int i=totalloops*16; i<totalpixels; i++){
        BYTE gray = static_cast<BYTE>(std::round(image.red[i]*0.2989 + image.green[i]*0.5870 + image.blue[i]*0.1140));
        image.red[i] = gray;
        image.green[i] = gray;
        image.blue[i] = gray;
    }
}

void filter_blur(IMAGE_DATA &image){
    // this is just here to stop the main function from crashing because there wasn't a blur function defined - will add later
    const int box_radius = 8;
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
                    r_sum += image.red[x];
                    g_sum += image.green[x];
                    b_sum += image.blue[x];
                    count++;
                }
            }
        }

        image.red[i] = ((r_sum / count) % 255);
        image.green[i] = ((g_sum / count) % 255);
        image.blue[i] = ((b_sum / count) % 255);
    }
}

void filter_brightness_basic(IMAGE_DATA &image, int brightness){
    const int rbrightness = std::max(-255, std::min(255, brightness)); // clamp brightness to [-255, 255]
    for (int i = 0; i < image.width * image.height; i++){
        image.red[i] = static_cast<uint8_t>(std::max(0, std::min(255, image.red[i] + rbrightness))); // equivalent to std::clamp from 0 to 255
        image.green[i] = static_cast<uint8_t>(std::max(0, std::min(255, image.green[i] + rbrightness)));
        image.blue[i] = static_cast<uint8_t>(std::max(0, std::min(255, image.blue[i] + rbrightness)));
    }
}
