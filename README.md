# SIMD Multimedia Filtering with Compiler Intrinsics (WIP)

This project implements image filtering operations using C/C++, with a focus on efficient pixel manipulation and future integration of SIMD optimizations. It utilizes the **stb_image** and **stb_image_write** libraries from nothings/stb for reading and writing BMP images, while storing pixel data in a structured format for easier processing.

Test image(s) can be found in lib/images

**NOTE** - If you open the files with intrinsics for an architecture your computer _doesn't_ support, it may highlight it as an error/squiggle
in some code editors/IDEs. If you prefer not to see these errors/squiggles, you can delete the files that don't
match your architecture.
In either case, the code should still compile, as this is simply visual.

## Features
- Loads BMP images and extracts RGB channels separately.
- Supports basic filtering operations (e.g., brightness, edge detection, grayscale, etc.).
- Outputs filtered images as BMP files.

## Benchmarks
For 100,000 iterations of grayscaled charizard.bmp:
(found in lib/benchmarks.txt)

| Intrinsics        | Average Elapsed Time (seconds)| Speedup (vs Default)  |
|-------------------|-------------------------------|-----------------------|
| Default           | 81.82                         | 1.00x                 |
| SSE2              | 21.63                         | 3.78x                 |
| AVX2              | 11.67                         | 7.02x                 |

## Dependencies
- C++17 or later
- x86 processor with SSE/AVX2, or ARM processor with NEON

## Build & Usage
### Using Makefile
To compile (default):
```sh
make
```
### Manual Compilation
Similar to Makefile,
On x86, run:
```sh
g++ -Iinclude -o filter src/main.cpp src/helpers.cpp src/defaultf.cpp src/sse2f.cpp -w
```
On ARM, run:
```sh
g++ -Iinclude -o filter src/main.cpp src/helpers.cpp src/defaultf.cpp -w -mfpu=neon
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
- Add CMake configuration