# SIMD Multimedia Filtering with Compiler Intrinsics (WIP)

This project implements image filtering operations using C/C++, with a focus on efficient pixel manipulation and future integration of SIMD optimizations. It utilizes the **stb_image** and **stb_image_write** libraries from nothings/stb for reading and writing BMP images, while storing pixel data in a structured format for easier processing.

Test image(s) can be found in lib/images

**NOTE** - If you open the files with intrinsics for an architecture your computer _doesn't_ support, it may highlight it as an error/squiggle
in some code editors/IDEs. If you prefer not to see these errors/squiggles, you can delete the files that don't
match your architecture.
In either case, the code should still compile.

## Features
- Loads BMP images and extracts RGB channels separately.
- Supports basic filtering operations (e.g., brightness, edge detection, grayscale, etc.).
- Outputs filtered images as BMP files.

## Dependencies
- C++17 or later
- x86 processor with SSE/AVX2, or ARM processor with NEON

## Build & Usage
### Using Makefile
To compile (default):
```sh
make
```
To compile (ARM ARMv7-A):
```sh
make use_neon=1
```
### Manual Compilation
Similar to Makefile, run:
```sh
g++ -Iinclude -o filter src/main.cpp src/helpers.cpp src/defaultf.cpp src/sse2f.cpp -w
```
On ARMv7-A, run:
```sh
g++ -Iinclude -o filter src/main.cpp src/helpers.cpp src/defaultf.cpp src/sse2f.cpp -w -mfpu=neon
```
### Running
Run the program:
```sh
./filter [flag] <input.bmp> <output.bmp>
```
Available flags:
- `g` - Convert to grayscale - Available for SSE2 and Default (Scalar)

Work In Progress flags:
- `b` - Apply a blur filter
- `e` - Apply edge detection

## Future Improvements
- Support for 4-channel images (RGBA)
- Additional filter options
- GUI integration for convenience
- Sound and other photo formats, and later video filtering
- Implementation of AVX2 and NEON
