#include <immintrin.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "avx2f.h"

void filter_grayscale(IMAGE_DATA &image){
    int totalpixels = image.width * image.height;
    int totalloops = totalpixels / 32;

    const __m256i red_multiplier = _mm256_set1_epi16(77);
    const __m256i green_multiplier = _mm256_set1_epi16(77);
    const __m256i blue_multiplier = _mm256_set1_epi16(77);

    for (int i = 0; i < totalloops; i++) {
        // Load 32 pixels at a time
        __m256i red16 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.red.data() + i * 32));
        __m256i green16 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.green.data() + i * 32));
        __m256i blue16 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.blue.data() + i * 32));

        // Unpack 8-bit integers to 16-bit integers
        __m256i red16l = _mm256_unpacklo_epi8(red16, _mm256_setzero_si256());
        __m256i green16l = _mm256_unpacklo_epi8(green16, _mm256_setzero_si256());
        __m256i blue16l = _mm256_unpacklo_epi8(blue16, _mm256_setzero_si256());
        __m256i red16h = _mm256_unpackhi_epi8(red16, _mm256_setzero_si256());
        __m256i green16h = _mm256_unpackhi_epi8(green16, _mm256_setzero_si256());
        __m256i blue16h = _mm256_unpackhi_epi8(blue16, _mm256_setzero_si256());

        // Compute grayscale values
        __m256i gray16l = _mm256_add_epi16(
            _mm256_add_epi16(_mm256_mullo_epi16(blue16l, blue_multiplier), _mm256_mullo_epi16(green16l, green_multiplier)),
            _mm256_mullo_epi16(red16l, red_multiplier));
        __m256i gray16h = _mm256_add_epi16(
            _mm256_add_epi16(_mm256_mullo_epi16(blue16h, blue_multiplier), _mm256_mullo_epi16(green16h, green_multiplier)),
            _mm256_mullo_epi16(red16h, red_multiplier));

        // Divide by 256 (shift right by 8 bits)
        gray16l = _mm256_srli_epi16(gray16l, 8);
        gray16h = _mm256_srli_epi16(gray16h, 8);

        // Pack 16-bit integers back to 8-bit integers
        __m256i gray8 = _mm256_packus_epi16(gray16l, gray16h);

        // Store the grayscale values back to the image
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(image.red.data() + i * 32), gray8);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(image.green.data() + i * 32), gray8);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(image.blue.data() + i * 32), gray8);
    }

    for (int i=totalloops*32; i<totalpixels; i++){
        BYTE gray = static_cast<BYTE>(std::round(image.red[i]*0.2989 + image.green[i]*0.5870 + image.blue[i]*0.1140));
        image.red[i] = gray;
        image.green[i] = gray;
        image.blue[i] = gray;
    }
}