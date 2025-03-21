# SIMD Multimedia Filtering with Compiler Intrinsics (WIP)

This project implements image filtering operations using C/C++, with a focus on efficient pixel manipulation and future integration of SIMD optimizations. It utilizes the **stb_image** and **stb_image_write** libraries from nothings/stb for reading and writing BMP images, while storing pixel data in a structured format for easier processing.

Test image(s) can be found in lib/images

## Features
- Loads BMP images and extracts RGB channels separately.
- Supports basic filtering operations (e.g., brightness, edge detection, grayscale, etc.).
- Outputs filtered images as BMP files.
- Designed for future SIMD optimizations.

## Dependencies
- C++17 or later

## Usage
Compile the program:
```sh
make
```
Run the program:
```sh
./filter [flag] <input.bmp> <output.bmp>
```
(Soon to be) available flags:
- `b` - Apply a blur filter
- `e` - Apply edge detection
- `g` - Convert to grayscale

## Future Improvements
- Support for 4-channel images (RGBA)
- Additional filter options
- SIMD optimizations for faster processing
- GUI integration for convenience
- Sound and other photo formats, and later video filtering
