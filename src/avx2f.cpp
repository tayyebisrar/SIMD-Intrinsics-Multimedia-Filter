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

void blur_pixel(IMAGE_DATA &image, IMAGE_DATA &temporary_image, int i) {
    // Blurs a single pixel at index i - checks for bounds and offset errors
    int row = i / image.width;
    int col = i % image.width;

    int sr_sum = 0, sg_sum = 0, sb_sum = 0, c = 0;

    for (int j = -1; j <= 1; j++) {
        for (int k = -1; k <= 1; k++) {
            int nrow = row + j;
            int ncol = col + k;

            if (nrow < 0 || nrow >= image.height || ncol < 0 || ncol >= image.width) {
                continue;
            }

            int ni = nrow * image.width + ncol;
            sr_sum += image.red[ni];
            sg_sum += image.green[ni];
            sb_sum += image.blue[ni];
            c++;
        }
    }

    temporary_image.red[i] = std::min(sr_sum / c, 255);
    temporary_image.green[i] = std::min(sg_sum / c, 255);
    temporary_image.blue[i] = std::min(sb_sum / c, 255);
}

void filter_blur(IMAGE_DATA &image){
    // perform a 3x3 box blur filter using AVX2 intrinsics, but skipping the edge columns and rows so we can easily load in the 3x3 box without checking for bounds
    // we can then do the edge parts separately

    IMAGE_DATA temporary_image = image;
    // __m256i r_sum, g_sum, b_sum; // don't need a count since it's always 9 when in range when doing SIMD part

    int c = 0, sr_sum = 0, sg_sum = 0, sb_sum = 0;
    
    // top row
    for (int i = 0; i < image.width; i++){
        blur_pixel(image, temporary_image, i);
    }
    // bottom row
    for (int i = (image.width*(image.height-1)); i < image.width*image.height; i++){
        blur_pixel(image, temporary_image, i);
    }
    // left column (excluding corners)
    for (int i = image.width; i < (image.width*(image.height-1)); i+=image.width){
        blur_pixel(image, temporary_image, i);
    }
    // right column (excluding corners)
    for (int i=(2*image.width)-1; i < image.height*image.width-1; i+=image.width){
        blur_pixel(image, temporary_image, i);
    }

    int i=image.width+1; // start at image index [1, 1] (from idx [0, 0])
    while (i < (image.width*(image.height-1))-1) // end at image index [width-2, height-2] (from idx [width-1, height-1])
    {
        int col = i % image.width;
        int row = i / image.width;
        
        if (col == 0 || col == image.width - 1) {
            i++;
            continue; // skip the first and last column
        }

        if ((col + 32) > (image.width)) {
            // if the next 32 pixels would cross the row boundary, need to process the rest in scalar code
            // was going to use col + 33 to stop it also doing the final column, but we can do it
            // and then overwrite it later, potentially increasing performance
            
            for (int j = col; j < image.width - 1; j++) { // can skip the last column here, though. Makes no difference
                blur_pixel(image, temporary_image, i + j); // call the blur pixel function to do the blur
                i++;
            }
        }
        
    }

    image = temporary_image; // copy the temp image back into the original
}

/*
void filter_blur(IMAGE_DATA &image){
    // perform a 3x3 box blur filter using AVX2 intrinsics, but skipping the edge columns and rows so we can easily load in the 3x3 box without checking for bounds
    // we can then do the edge parts later in separate loops following the basic method of box_blur I implemented before

    IMAGE_DATA temporary_image = image; // create a temp image - we can't adjust the pixels in place since surrounding pixels still need the original values
    
    __m256i r_sum, g_sum, b_sum; // don't need a count since it's always 9 when in range when doing SIMD part

    

    int i=image.width + 1; // start at image index [1, 1] (from idx [0, 0])
    while (i < (image.width * (image.height - 1))) {
        int col = i % image.width; 
        int row = i / image.width;

        std::cout << "Image size: " << image.width << "x" << image.height << "\n";
        std::cout << "Pixel " << i << "[" << row << ", " << col << "]\n";

        // skip the first and last column
        if (col == 0 || col == image.width - 1) {
            i++;
            continue;
        }

        // we need to make sure the SIMD code doesn't go across 2 rows, so we need to check if the column is close to the end of the row
        if (col + 31 >= image.width - 1) {
            for (int j = col; j < image.width - 1; j++) { // can skip the final column
                int index  = (i / image.width) * image.width + j;
                int r_sum = 0, g_sum = 0, b_sum = 0, c = 0; // reset the sums and count for each pixel
                for (int k = -1; k <= 0; k++){
                    for (int j = -1; j <= 1; j++){
                        int x = index + j * image.width + k; // current pixel + row offset + column offset
                        if (x >= 0 && x < image.width * image.height){
                            if ((x/image.width) != (index / image.width)){
                                continue; // if the pixel is not in the right row when it should be (a column offset error), skip it
                            }
                            r_sum += image.red[x];
                            g_sum += image.green[x];
                            b_sum += image.blue[x];
                            c++;
                        }
                    }
                }
                temporary_image.red[index] = ((r_sum / c));
                temporary_image.green[index] = ((g_sum / c));
                temporary_image.blue[index] = ((b_sum / c));
                i++;
            }
            continue;
        }
        

        // Process the 32-pixel block (which fits) using SIMD

        __m256i top_left = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.red.data() + i - image.width - 1));
        __m256i top = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.red.data() + i - image.width));
        __m256i top_right = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.red.data() + i - image.width + 1));
        __m256i left = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.red.data() + i - 1));
        __m256i center = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.red.data() + i));
        __m256i right = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.red.data() + i + 1));
        __m256i bottom_left = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.red.data() + i + image.width - 1));
        __m256i bottom = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.red.data() + i + image.width));
        __m256i bottom_right = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.red.data() + i + image.width + 1));

        // Now we need to unpack the 8 bit integers to 16, so we can do the adding without overflowing
        __m256i top_left_16 = _mm256_unpacklo_epi8(top_left, _mm256_setzero_si256());
        __m256i top_16 = _mm256_unpacklo_epi8(top, _mm256_setzero_si256());
        __m256i top_right_16 = _mm256_unpacklo_epi8(top_right, _mm256_setzero_si256());
        __m256i left_16 = _mm256_unpacklo_epi8(left, _mm256_setzero_si256());
        __m256i center_16 = _mm256_unpacklo_epi8(center, _mm256_setzero_si256());
        __m256i right_16 = _mm256_unpacklo_epi8(right, _mm256_setzero_si256());
        __m256i bottom_left_16 = _mm256_unpacklo_epi8(bottom_left, _mm256_setzero_si256());
        __m256i bottom_16 = _mm256_unpacklo_epi8(bottom, _mm256_setzero_si256());
        __m256i bottom_right_16 = _mm256_unpacklo_epi8(bottom_right, _mm256_setzero_si256());

        // Next we can add them together
        r_sum = _mm256_add_epi16(
            _mm256_add_epi16(_mm256_add_epi16(top_left_16, top_16), _mm256_add_epi16(top_right_16, left_16)),
            _mm256_add_epi16(_mm256_add_epi16(center_16, right_16), _mm256_add_epi16(bottom_left_16, bottom_16)));
        r_sum = _mm256_add_epi16(r_sum, bottom_right_16);

        // Now we need to divide by 9 - we can't use direct division (again) so will use multiplication + bit shifting instead (or just taking the right bits)
        __m256i r_avg = _mm256_mulhi_epu16(r_sum, _mm256_set1_epi16(7282)); // 7282 is 2^16/9 and then takes the right bits to get 1/9
        
        // pack the 16 bit integers back to 8 bit integers
        r_avg = _mm256_packus_epi16(r_avg, r_avg);

        _mm256_storeu_si256(reinterpret_cast<__m256i*>(temporary_image.red.data() + i), r_avg); // store the result back to the temporary image
    
        // Repeat the same process for the green channel
        top_left = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.green.data() + i - image.width - 1));
        top = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.green.data() + i - image.width));
        top_right = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.green.data() + i - image.width + 1));
        left = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.green.data() + i - 1));
        center = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.green.data() + i));
        right = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.green.data() + i + 1));
        bottom_left = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.green.data() + i + image.width - 1));
        bottom = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.green.data() + i + image.width));
        bottom_right = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.green.data() + i + image.width + 1));

        // Unpack, sum, divide, and pack for the green channel
        top_left_16 = _mm256_unpacklo_epi8(top_left, _mm256_setzero_si256());
        top_16 = _mm256_unpacklo_epi8(top, _mm256_setzero_si256());
        top_right_16 = _mm256_unpacklo_epi8(top_right, _mm256_setzero_si256());
        left_16 = _mm256_unpacklo_epi8(left, _mm256_setzero_si256());
        center_16 = _mm256_unpacklo_epi8(center, _mm256_setzero_si256());
        right_16 = _mm256_unpacklo_epi8(right, _mm256_setzero_si256());
        bottom_left_16 = _mm256_unpacklo_epi8(bottom_left, _mm256_setzero_si256());
        bottom_16 = _mm256_unpacklo_epi8(bottom, _mm256_setzero_si256());
        bottom_right_16 = _mm256_unpacklo_epi8(bottom_right, _mm256_setzero_si256());

        g_sum = _mm256_add_epi16(
            _mm256_add_epi16(_mm256_add_epi16(top_left_16, top_16), _mm256_add_epi16(top_right_16, left_16)),
            _mm256_add_epi16(_mm256_add_epi16(center_16, right_16), _mm256_add_epi16(bottom_left_16, bottom_16)));
        g_sum = _mm256_add_epi16(g_sum, bottom_right_16);

        __m256i g_avg = _mm256_mulhi_epu16(g_sum, _mm256_set1_epi16(7282));
        g_avg = _mm256_packus_epi16(g_avg, g_avg);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(temporary_image.green.data() + i), g_avg);

        // Repeat the same process for the blue channel
        top_left = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.blue.data() + i - image.width - 1));
        top = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.blue.data() + i - image.width));
        top_right = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.blue.data() + i - image.width + 1));
        left = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.blue.data() + i - 1));
        center = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.blue.data() + i));
        right = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.blue.data() + i + 1));
        bottom_left = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.blue.data() + i + image.width - 1));
        bottom = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.blue.data() + i + image.width));
        bottom_right = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(image.blue.data() + i + image.width + 1));

        // Unpack, sum, divide, and pack for the blue channel
        top_left_16 = _mm256_unpacklo_epi8(top_left, _mm256_setzero_si256());
        top_16 = _mm256_unpacklo_epi8(top, _mm256_setzero_si256());
        top_right_16 = _mm256_unpacklo_epi8(top_right, _mm256_setzero_si256());
        left_16 = _mm256_unpacklo_epi8(left, _mm256_setzero_si256());
        center_16 = _mm256_unpacklo_epi8(center, _mm256_setzero_si256());
        right_16 = _mm256_unpacklo_epi8(right, _mm256_setzero_si256());
        bottom_left_16 = _mm256_unpacklo_epi8(bottom_left, _mm256_setzero_si256());
        bottom_16 = _mm256_unpacklo_epi8(bottom, _mm256_setzero_si256());
        bottom_right_16 = _mm256_unpacklo_epi8(bottom_right, _mm256_setzero_si256());

        b_sum = _mm256_add_epi16(
            _mm256_add_epi16(_mm256_add_epi16(top_left_16, top_16), _mm256_add_epi16(top_right_16, left_16)),
            _mm256_add_epi16(_mm256_add_epi16(center_16, right_16), _mm256_add_epi16(bottom_left_16, bottom_16)));
        b_sum = _mm256_add_epi16(b_sum, bottom_right_16);

        __m256i b_avg = _mm256_mulhi_epu16(b_sum, _mm256_set1_epi16(7282));
        b_avg = _mm256_packus_epi16(b_avg, b_avg);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(temporary_image.blue.data() + i), b_avg);

        // Increment the pixels
        i += 32; // move to the next block of 32 pixels
    }

    // Now, all 'central' pixels have been processed as quickly as possible in SIMD.
    // We need to handle the edges with scalar code because they wont fit correctly otherwise

    int c = 0, sr_sum = 0, sg_sum = 0, sb_sum = 0;
    // top row
    for (int i = 0; i < image.width; i++){
        c = 0, sr_sum = 0, sg_sum = 0, sb_sum = 0; // reset the sums and count for each pixel
        for (int j = 0; j <= 1; j++){
            for (int k = -1; k <= 1; k++){
                int x = i + j * image.width + k; // current pixel + row offset + column offset
                if (x >= 0 && x < image.width * image.height){
                    if ((x/image.width) != (i / image.width)){
                        continue; // if the pixel is not in the right row when it should be (a column offset error), skip it
                    }
                    sr_sum += image.red[x];
                    sg_sum += image.green[x];
                    sb_sum += image.blue[x];
                    c++;
                }
            }
        }

        temporary_image.red[i] = ((sr_sum / c) % 255);
        temporary_image.green[i] = ((sg_sum / c) % 255);
        temporary_image.blue[i] = ((sb_sum / c) % 255);
    }
    // bottom row
    for (int i = image.width*(image.height-1); i < image.width*image.height; i++){
        c = 0, sr_sum = 0, sg_sum = 0, sb_sum = 0; // reset the sums and count for each pixel
        for (int j = -1; j <= 0; j++){
            for (int k = -1; k <= 1; k++){
                int x = i + j * image.width + k; // current pixel + row offset + column offset
                if (x >= 0 && x < image.width * image.height){
                    if ((x/image.width) != (i / image.width)){
                        continue; // if the pixel is not in the right row when it should be (a column offset error), skip it
                    }
                    sr_sum += image.red[x];
                    sg_sum += image.green[x];
                    sb_sum += image.blue[x];
                    c++;
                }
            }
        }

        temporary_image.red[i] = ((sr_sum / c) % 255);
        temporary_image.green[i] = ((sg_sum / c) % 255);
        temporary_image.blue[i] = ((sb_sum / c) % 255);
    }
    // left column
    for (int i=image.width; i < image.height*image.width; i+=image.width){
        c = 0, sr_sum = 0, sg_sum = 0, sb_sum = 0; // reset the sums and count for each pixel
        for (int j = -1; j <= 1; j++){
            for (int k = 0; k <= 1; k++){
                int x = i + j * image.width + k; // current pixel + row offset + column offset
                if (x >= 0 && x < image.width * image.height){
                    sr_sum += image.red[x];
                    sg_sum += image.green[x];
                    sb_sum += image.blue[x];
                    c++;
                }
            }
        }

        temporary_image.red[i] = ((sr_sum / c) % 255);
        temporary_image.green[i] = ((sg_sum / c) % 255);
        temporary_image.blue[i] = ((sb_sum / c) % 255);
    }
    // right column
    for (int i=(2*image.width)-1; i < image.height*image.width-1; i+=image.width){
        c = 0, sr_sum = 0, sg_sum = 0, sb_sum = 0; // reset the sums and count for each pixel
        for (int j = -1; j <= 1; j++){
            for (int k = -1; k <= 0; k++){
                int x = i + j * image.width + k; // current pixel + row offset + column offset
                if (x >= 0 && x < image.width * image.height){
                    sr_sum += image.red[x];
                    sg_sum += image.green[x];
                    sb_sum += image.blue[x];
                    c++;
                }
            }
        }

        temporary_image.red[i] = ((sr_sum / c) % 255);
        temporary_image.green[i] = ((sg_sum / c) % 255);
        temporary_image.blue[i] = ((sb_sum / c) % 255);
    }

    // now we need to copy everything back
    image = temporary_image; // copy the temp image back into the original
}   */