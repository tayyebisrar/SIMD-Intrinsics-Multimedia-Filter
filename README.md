# SIMD Multimedia Filtering with Compiler Intrinsics

This project implements image filtering operations using C/C++, with a focus on efficient pixel manipulation and future integration of SIMD optimizations. It utilizes the **stb_image** and **stb_image_write** libraries from nothings/stb for reading and writing BMP images, while storing pixel data in a structured format for easier processing.

Test image(s) can be found in lib/images

**NOTE** - If you open the files with intrinsics for an architecture your computer _doesn't_ support, it may highlight it as an error/squiggle
in some code editors/IDEs. If you prefer not to see these errors/squiggles, you can delete the files that don't
match your architecture.
In either case, the code should still compile, as this is simply visual.

## Features
- Loads BMP images and extracts RGB channels separately.
- Supports basic filtering operations (e.g., brightness, blurring, grayscale).
- Outputs filtered images as BMP files.

## Benchmarks
For 100,000 iterations of grayscaled charizard.bmp:
(found in lib/benchmarks.txt)

| Intrinsics        | Average Elapsed Time (seconds)| Speedup (vs Default)  |
|-------------------|-------------------------------|-----------------------|
| Default           | 81.82                         | 1.00x                 |
| SSE2              | 21.63                         | 3.78x                 |
| AVX2              | 11.67                         | 7.02x                 |

For 100,000 iterations of box-blurred lena_color.bmp:

| Intrinsics        | Average Elapsed Time (seconds)| Speedup (vs Default)  |
|-------------------|-------------------------------|-----------------------|
| Default           | 1048.96                       | 1.00x                 |
| SSE2              | 424.941                       | 2.47x                 |
| AVX2              | 325.26                        | 3.21x                 |

For 100,000 iterations of 128-brightness increase lena_color.bmp:

| Intrinsics        | Average Elapsed Time (seconds)| Speedup (vs Default)  |
|-------------------|-------------------------------|-----------------------|
| Default           | 409.320                       | 1.00x                 |
| SSE2              | 30.269                        | 13.52x                |
| AVX2              | 15.542                        | 26.34x                |

- Measurements were taken on a desktop machine with minimal background processes.
- All tests used identical input.
- The "Default" version uses a scalar loop without intrinsics (viewable algorithms in /src/default.cpp)

## Dependencies
- C++17 or later
- x86 processor with SSE2/AVX2

## Build & Usage
### Using Makefile
To compile (default):
```sh
make
```
To compile with AVX2 intrinsics:
```sh
make use_avx2=1
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
- `g` - Convert to grayscale
- `b` - Apply a blur filter
- `l` - Increase/Decrease Brightness

## Future Improvements
- Support for 4-channel images (RGBA)
- Support for ARM chips (NEON Intrinsics)
- GUI integration for convenience
- Sound and other photo formats, and later video filtering
- CMake configuration