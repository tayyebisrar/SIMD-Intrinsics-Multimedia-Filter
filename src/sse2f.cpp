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
    const __m128i simdzero = _mm_setzero_si128();

    for (int i=0; i<totalloops; i++){
        // integer division by 3 is not possible, so will use grayscale = 0.2989×R+0.5870×G+0.1140×B 
        // load in 16 pixels at a time
        __m128i red8 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.red.data() + i*16));
        __m128i green8 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.green.data() + i*16));
        __m128i blue8 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.blue.data() + i*16));

        // convert to 16 bit integers because otherwise the multiplication would overflow
        __m128i red16l = _mm_unpacklo_epi8(red8, simdzero);
        __m128i green16l = _mm_unpacklo_epi8(green8, simdzero);
        __m128i blue16l = _mm_unpacklo_epi8(blue8, simdzero);
        __m128i red16h = _mm_unpackhi_epi8(red8, simdzero);
        __m128i green16h = _mm_unpackhi_epi8(green8, simdzero);
        __m128i blue16h = _mm_unpackhi_epi8(blue8, simdzero);

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
    // perform a 3x3 box blur filter using SSE2 intrinsics, but skipping the edge columns and rows so we can easily load in the 3x3 box without checking for bounds
    // we can then do the edge parts separately

    IMAGE_DATA temporary_image = image;

    const __m128i div9_magicnumber = _mm_set1_epi16(7282);
    const __m128i simdzero = _mm_setzero_si128();

    int i=image.width+1; // start at image index [1, 1] (from idx [0, 0])
    while (i < (image.width*(image.height-1))-1) // end at image index [width-2, height-2] (from idx [width-1, height-1])
    {
        int col = i % image.width;
        // int row = i / image.width; unused
        
        if (col == 0 || col == image.width - 1) {
            i++;
            continue; // skip the first and last column
        }

        if ((col + 16) > (image.width)) {
            // if the next 16 pixels would cross the row boundary, need to process the rest in scalar code
            // was going to use col + 17 (or col + 16 > image.width - 1) to stop it also doing the final column, but we can do it (wrong for last pixel)
            // and then overwrite it later, potentially increasing performance
            
            for (int j = col; j < image.width - 1; j++) { // can skip the last column here, though. Makes no difference
                blur_pixel(image, temporary_image, i - col + j); // call the blur pixel function to do the blur
            }
            i += (image.width - col); // move to the next row
            continue; // go from the top to ensure the next row is processed correctly
        }

        // Process 16 pixels at a time
        // Find the average of a 3x3 box around each pixel in the 16-pixel block


        __m128i top_left_r = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.red.data() + i - image.width - 1));
        __m128i top_r = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.red.data() + i - image.width));
        __m128i top_right_r = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.red.data() + i - image.width + 1));
        __m128i left_r = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.red.data() + i - 1));
        __m128i center_r = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.red.data() + i));
        __m128i right_r = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.red.data() + i + 1));
        __m128i bottom_left_r = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.red.data() + i + image.width - 1));
        __m128i bottom_r = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.red.data() + i + image.width));
        __m128i bottom_right_r = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.red.data() + i + image.width + 1));

        // Unpack the 8-bit integers to 16-bit integers to prevent overflow (adding 9 pixels easily exceeds 255)

        __m128i top_left_r_16_lo = _mm_unpacklo_epi8(top_left_r, simdzero);
        __m128i top_r_16_lo = _mm_unpacklo_epi8(top_r, simdzero);
        __m128i top_right_r_16_lo = _mm_unpacklo_epi8(top_right_r, simdzero);
        __m128i left_r_16_lo = _mm_unpacklo_epi8(left_r, simdzero);
        __m128i center_r_16_lo = _mm_unpacklo_epi8(center_r, simdzero);
        __m128i right_r_16_lo = _mm_unpacklo_epi8(right_r, simdzero);
        __m128i bottom_left_r_16_lo = _mm_unpacklo_epi8(bottom_left_r, simdzero);
        __m128i bottom_r_16_lo = _mm_unpacklo_epi8(bottom_r, simdzero);
        __m128i bottom_right_r_16_lo = _mm_unpacklo_epi8(bottom_right_r, simdzero);
        __m128i top_left_r_16_hi = _mm_unpackhi_epi8(top_left_r, simdzero);
        __m128i top_r_16_hi = _mm_unpackhi_epi8(top_r, simdzero);
        __m128i top_right_r_16_hi = _mm_unpackhi_epi8(top_right_r, simdzero);
        __m128i left_r_16_hi = _mm_unpackhi_epi8(left_r, simdzero);
        __m128i center_r_16_hi = _mm_unpackhi_epi8(center_r, simdzero);
        __m128i right_r_16_hi = _mm_unpackhi_epi8(right_r, simdzero);
        __m128i bottom_left_r_16_hi = _mm_unpackhi_epi8(bottom_left_r, simdzero);
        __m128i bottom_r_16_hi = _mm_unpackhi_epi8(bottom_r, simdzero);
        __m128i bottom_right_r_16_hi = _mm_unpackhi_epi8(bottom_right_r, simdzero);

        // Now we can sum the 16-bit integers together
        __m128i r_sum_lo = _mm_add_epi16(top_left_r_16_lo, top_r_16_lo);
        r_sum_lo = _mm_add_epi16(r_sum_lo, top_right_r_16_lo);
        r_sum_lo = _mm_add_epi16(r_sum_lo, left_r_16_lo);
        r_sum_lo = _mm_add_epi16(r_sum_lo, center_r_16_lo);
        r_sum_lo = _mm_add_epi16(r_sum_lo, right_r_16_lo);
        r_sum_lo = _mm_add_epi16(r_sum_lo, bottom_left_r_16_lo);
        r_sum_lo = _mm_add_epi16(r_sum_lo, bottom_r_16_lo);
        r_sum_lo = _mm_add_epi16(r_sum_lo, bottom_right_r_16_lo);
        __m128i r_sum_hi = _mm_add_epi16(top_left_r_16_hi, top_r_16_hi);
        r_sum_hi = _mm_add_epi16(r_sum_hi, top_right_r_16_hi);
        r_sum_hi = _mm_add_epi16(r_sum_hi, left_r_16_hi);
        r_sum_hi = _mm_add_epi16(r_sum_hi, center_r_16_hi);
        r_sum_hi = _mm_add_epi16(r_sum_hi, right_r_16_hi);
        r_sum_hi = _mm_add_epi16(r_sum_hi, bottom_left_r_16_hi);
        r_sum_hi = _mm_add_epi16(r_sum_hi, bottom_r_16_hi);
        r_sum_hi = _mm_add_epi16(r_sum_hi, bottom_right_r_16_hi);

        // Divide by 9 by approximating using the value 2^16 / 9 and then shifting 16

        __m128i r_avg_lo = _mm_mulhi_epu16(r_sum_lo, div9_magicnumber); // 7282 is 2^16/9
        __m128i r_avg_hi = _mm_mulhi_epu16(r_sum_hi, div9_magicnumber);

        // Pack the 16-bit integers back to 8-bit integers AND shift down by 16 bits

        __m128i r_avg =_mm_packus_epi16(r_avg_lo, r_avg_hi);

        __m128i top_left_g = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.green.data() + i - image.width - 1));
        __m128i top_g = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.green.data() + i - image.width));
        __m128i top_right_g = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.green.data() + i - image.width + 1));
        __m128i left_g = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.green.data() + i - 1));
        __m128i center_g = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.green.data() + i));
        __m128i right_g = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.green.data() + i + 1));
        __m128i bottom_left_g = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.green.data() + i + image.width - 1));
        __m128i bottom_g = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.green.data() + i + image.width));
        __m128i bottom_right_g = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.green.data() + i + image.width + 1));

        // Unpack the 8-bit integers to 16-bit integers to prevent overflow (adding 9 pixels easily exceeds 255)

        __m128i top_left_g_16_lo = _mm_unpacklo_epi8(top_left_g, simdzero);
        __m128i top_g_16_lo = _mm_unpacklo_epi8(top_g, simdzero);
        __m128i top_right_g_16_lo = _mm_unpacklo_epi8(top_right_g, simdzero);
        __m128i left_g_16_lo = _mm_unpacklo_epi8(left_g, simdzero);
        __m128i center_g_16_lo = _mm_unpacklo_epi8(center_g, simdzero);
        __m128i right_g_16_lo = _mm_unpacklo_epi8(right_g, simdzero);
        __m128i bottom_left_g_16_lo = _mm_unpacklo_epi8(bottom_left_g, simdzero);
        __m128i bottom_g_16_lo = _mm_unpacklo_epi8(bottom_g, simdzero);
        __m128i bottom_right_g_16_lo = _mm_unpacklo_epi8(bottom_right_g, simdzero);
        __m128i top_left_g_16_hi = _mm_unpackhi_epi8(top_left_g, simdzero);
        __m128i top_g_16_hi = _mm_unpackhi_epi8(top_g, simdzero);
        __m128i top_right_g_16_hi = _mm_unpackhi_epi8(top_right_g, simdzero);
        __m128i left_g_16_hi = _mm_unpackhi_epi8(left_g, simdzero);
        __m128i center_g_16_hi = _mm_unpackhi_epi8(center_g, simdzero);
        __m128i right_g_16_hi = _mm_unpackhi_epi8(right_g, simdzero);
        __m128i bottom_left_g_16_hi = _mm_unpackhi_epi8(bottom_left_g, simdzero);
        __m128i bottom_g_16_hi = _mm_unpackhi_epi8(bottom_g, simdzero);
        __m128i bottom_right_g_16_hi = _mm_unpackhi_epi8(bottom_right_g, simdzero);

        // Now we can sum the 16-bit integers together
        __m128i g_sum_lo = _mm_add_epi16(top_left_g_16_lo, top_g_16_lo);
        g_sum_lo = _mm_add_epi16(g_sum_lo, top_right_g_16_lo);
        g_sum_lo = _mm_add_epi16(g_sum_lo, left_g_16_lo);
        g_sum_lo = _mm_add_epi16(g_sum_lo, center_g_16_lo);
        g_sum_lo = _mm_add_epi16(g_sum_lo, right_g_16_lo);
        g_sum_lo = _mm_add_epi16(g_sum_lo, bottom_left_g_16_lo);
        g_sum_lo = _mm_add_epi16(g_sum_lo, bottom_g_16_lo);
        g_sum_lo = _mm_add_epi16(g_sum_lo, bottom_right_g_16_lo);
        __m128i g_sum_hi = _mm_add_epi16(top_left_g_16_hi, top_g_16_hi);
        g_sum_hi = _mm_add_epi16(g_sum_hi, top_right_g_16_hi);
        g_sum_hi = _mm_add_epi16(g_sum_hi, left_g_16_hi);
        g_sum_hi = _mm_add_epi16(g_sum_hi, center_g_16_hi);
        g_sum_hi = _mm_add_epi16(g_sum_hi, right_g_16_hi);
        g_sum_hi = _mm_add_epi16(g_sum_hi, bottom_left_g_16_hi);
        g_sum_hi = _mm_add_epi16(g_sum_hi, bottom_g_16_hi);
        g_sum_hi = _mm_add_epi16(g_sum_hi, bottom_right_g_16_hi);

        // Divide by 9 by approximating using the value 2^16 / 9 and then shifting 16

        __m128i g_avg_lo = _mm_mulhi_epu16(g_sum_lo, div9_magicnumber); // 7282 is 2^16/9
        __m128i g_avg_hi = _mm_mulhi_epu16(g_sum_hi, div9_magicnumber);

        // Pack the 16-bit integers back to 8-bit integers AND shift down by 16 bits

        __m128i g_avg =_mm_packus_epi16(g_avg_lo, g_avg_hi);

        __m128i top_left_b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.blue.data() + i - image.width - 1));
        __m128i top_b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.blue.data() + i - image.width));
        __m128i top_right_b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.blue.data() + i - image.width + 1));
        __m128i left_b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.blue.data() + i - 1));
        __m128i center_b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.blue.data() + i));
        __m128i right_b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.blue.data() + i + 1));
        __m128i bottom_left_b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.blue.data() + i + image.width - 1));
        __m128i bottom_b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.blue.data() + i + image.width));
        __m128i bottom_right_b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.blue.data() + i + image.width + 1));

        // Unpack the 8-bit integers to 16-bit integers to prevent overflow (adding 9 pixels easily exceeds 255)

        __m128i top_left_b_16_lo = _mm_unpacklo_epi8(top_left_b, simdzero);
        __m128i top_b_16_lo = _mm_unpacklo_epi8(top_b, simdzero);
        __m128i top_right_b_16_lo = _mm_unpacklo_epi8(top_right_b, simdzero);
        __m128i left_b_16_lo = _mm_unpacklo_epi8(left_b, simdzero);
        __m128i center_b_16_lo = _mm_unpacklo_epi8(center_b, simdzero);
        __m128i right_b_16_lo = _mm_unpacklo_epi8(right_b, simdzero);
        __m128i bottom_left_b_16_lo = _mm_unpacklo_epi8(bottom_left_b, simdzero);
        __m128i bottom_b_16_lo = _mm_unpacklo_epi8(bottom_b, simdzero);
        __m128i bottom_right_b_16_lo = _mm_unpacklo_epi8(bottom_right_b, simdzero);
        __m128i top_left_b_16_hi = _mm_unpackhi_epi8(top_left_b, simdzero);
        __m128i top_b_16_hi = _mm_unpackhi_epi8(top_b, simdzero);
        __m128i top_right_b_16_hi = _mm_unpackhi_epi8(top_right_b, simdzero);
        __m128i left_b_16_hi = _mm_unpackhi_epi8(left_b, simdzero);
        __m128i center_b_16_hi = _mm_unpackhi_epi8(center_b, simdzero);
        __m128i right_b_16_hi = _mm_unpackhi_epi8(right_b, simdzero);
        __m128i bottom_left_b_16_hi = _mm_unpackhi_epi8(bottom_left_b, simdzero);
        __m128i bottom_b_16_hi = _mm_unpackhi_epi8(bottom_b, simdzero);
        __m128i bottom_right_b_16_hi = _mm_unpackhi_epi8(bottom_right_b, simdzero);

        // Now we can sum the 16-bit integers together
        __m128i b_sum_lo = _mm_add_epi16(top_left_b_16_lo, top_b_16_lo);
        b_sum_lo = _mm_add_epi16(b_sum_lo, top_right_b_16_lo);
        b_sum_lo = _mm_add_epi16(b_sum_lo, left_b_16_lo);
        b_sum_lo = _mm_add_epi16(b_sum_lo, center_b_16_lo);
        b_sum_lo = _mm_add_epi16(b_sum_lo, right_b_16_lo);
        b_sum_lo = _mm_add_epi16(b_sum_lo, bottom_left_b_16_lo);
        b_sum_lo = _mm_add_epi16(b_sum_lo, bottom_b_16_lo);
        b_sum_lo = _mm_add_epi16(b_sum_lo, bottom_right_b_16_lo);
        __m128i b_sum_hi = _mm_add_epi16(top_left_b_16_hi, top_b_16_hi);
        b_sum_hi = _mm_add_epi16(b_sum_hi, top_right_b_16_hi);
        b_sum_hi = _mm_add_epi16(b_sum_hi, left_b_16_hi);
        b_sum_hi = _mm_add_epi16(b_sum_hi, center_b_16_hi);
        b_sum_hi = _mm_add_epi16(b_sum_hi, right_b_16_hi);
        b_sum_hi = _mm_add_epi16(b_sum_hi, bottom_left_b_16_hi);
        b_sum_hi = _mm_add_epi16(b_sum_hi, bottom_b_16_hi);
        b_sum_hi = _mm_add_epi16(b_sum_hi, bottom_right_b_16_hi);

        // Divide by 9 by approximating using the value 2^16 / 9 and then shifting 16

        __m128i b_avg_lo = _mm_mulhi_epu16(b_sum_lo, div9_magicnumber); // 7282 is 2^16/9
        __m128i b_avg_hi = _mm_mulhi_epu16(b_sum_hi, div9_magicnumber);

        // Pack the 16-bit integers back to 8-bit integers AND shift down by 16 bits

        __m128i b_avg = _mm_packus_epi16(b_avg_lo, b_avg_hi);

        // Store the blurred pixel values into the temporary image

        _mm_storeu_si128(reinterpret_cast<__m128i*>(temporary_image.red.data() + i), r_avg);
        _mm_storeu_si128(reinterpret_cast<__m128i*>(temporary_image.green.data() + i), g_avg);
        _mm_storeu_si128(reinterpret_cast<__m128i*>(temporary_image.blue.data() + i), b_avg);

        i += 16; // move to the next 16 pixels
    }

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

    image = temporary_image; // copy the temp image back into the original
}

void filter_brightness(IMAGE_DATA &image, int brightness){
    // Adjust brightness of all pixels by a given integer, leaving each pixel in range [0, 255]
    const int totalpixels = image.width * image.height;
    const int rbrightness = std::max(-255, std::min(255, brightness));
    const __m128i brightness_vector = _mm_set1_epi8(static_cast<uint8_t>(std::abs(rbrightness)));
    if (rbrightness >= 0) {
        for (int i = 0; i < totalpixels; i+=16) {
            __m128i red = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.red.data() + i));
            __m128i green = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.green.data() + i));
            __m128i blue = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.blue.data() + i));

            red = _mm_adds_epu8(red, brightness_vector);
            green = _mm_adds_epu8(green, brightness_vector);
            blue = _mm_adds_epu8(blue, brightness_vector);

            _mm_storeu_si128(reinterpret_cast<__m128i*>(image.red.data() + i), red);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(image.green.data() + i), green);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(image.blue.data() + i), blue);
        }
    } 
    else {
        for (int i = 0; i < totalpixels; i+=16){
            __m128i red = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.red.data() + i));
            __m128i green = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.green.data() + i));
            __m128i blue = _mm_loadu_si128(reinterpret_cast<const __m128i*>(image.blue.data() + i));

            red = _mm_subs_epu8(red, brightness_vector);
            green = _mm_subs_epu8(green, brightness_vector);
            blue = _mm_subs_epu8(blue, brightness_vector);

            _mm_storeu_si128(reinterpret_cast<__m128i*>(image.red.data() + i), red);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(image.green.data() + i), green);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(image.blue.data() + i), blue);
        }
    }

    // Remaining pixels (scalar)
    for (int i = (totalpixels / 16 * 16); i < totalpixels; i++) {
        image.red[i] = static_cast<uint8_t>(std::max(0, std::min(255, image.red[i] + rbrightness))); // equivalent to std::clamp from 0 to 255
        image.green[i] = static_cast<uint8_t>(std::max(0, std::min(255, image.green[i] + rbrightness)));
        image.blue[i] = static_cast<uint8_t>(std::max(0, std::min(255, image.blue[i] + rbrightness)));
    }
}