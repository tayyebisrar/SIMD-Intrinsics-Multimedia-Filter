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