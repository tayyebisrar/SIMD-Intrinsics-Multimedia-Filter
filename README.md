# SIMD Multimedia Filtering with Compiler Intrinsics

A C++ image processing project that implements high-performance filters using SIMD intrinsics (SSE2 and AVX2). Designed for efficient pixel-level operations, this tool demonstrates major speedups over scalar implementations.

Utilizes the **stb_image** and **stb_image_write** libraries from nothings/stb for reading and writing BMP images.

Test image(s) can be found in lib/images

## Features
- Load and parse 24-bit BMP images using `stb_image`.
- Perform pixel-wise filtering: grayscale, brightness adjustment, and box blur.
- Save processed images in BMP format using `stb_image_write`.

## Benchmarks

### Grayscale: `charizard.bmp` (100,000 iterations)

| Intrinsics        | Average Elapsed Time (seconds)| Speedup (vs Default)  |
|-------------------|-------------------------------|-----------------------|
| Default           | 81.82                         | 1.00x                 |
| SSE2              | 21.63                         | 3.78x                 |
| AVX2              | 11.67                         | 7.02x                 |

### Blur: `lena_color.bmp` (100,000 iterations)

| Intrinsics        | Average Elapsed Time (seconds)| Speedup (vs Default)  |
|-------------------|-------------------------------|-----------------------|
| Default           | 1048.96                       | 1.00x                 |
| SSE2              | 424.941                       | 2.47x                 |
| AVX2              | 325.26                        | 3.21x                 |

### Brightness +128: `lena_color.bmp` (100,000 iterations)

| Intrinsics        | Average Elapsed Time (seconds)| Speedup (vs Default)  |
|-------------------|-------------------------------|-----------------------|
| Default           | 409.320                       | 1.00x                 |
| SSE2              | 30.269                        | 13.52x                |
| AVX2              | 15.542                        | 26.34x                |

- Measurements were taken on a desktop machine with minimal background processes.
- All tests used identical input.
- The "Default" version uses scalar loops (see `/src/default.cpp`)

## Dependencies
- C++17 or later
- x86 processor with SSE2/AVX2

## Build & Usage
### Using Makefile
To compile with SSE2 (default):
```sh
make
```
To compile with AVX2 intrinsics:
```sh
make use_avx2=1
```
### Manual Compilation
Similar to Makefile,
For SSE2, run:
```sh
g++ -Iinclude -o filter src/main.cpp src/helpers.cpp src/defaultf.cpp src/sse2f.cpp -w
```
For AVX2, run:
```sh
g++ -Iinclude -o -mavx2 filter src/main.cpp src/helpers.cpp src/defaultf.cpp src/avx2f.cpp -w
```
### Running
Run the program:
```sh
./filter [flag] <input.bmp> <output.bmp>
```
Example: `./filter g lib/images/charizard.bmp out/charizard_gray.bmp`

Available flags:
- `g` - Convert to grayscale
- `b` - Apply a blur filter
- `l` - Increase/Decrease Brightness

## Future Improvements
- Support for 4-channel images (RGBA)
- GUI integration for convenience
- CMake configuration

## Image Attribution

The sample images used for benchmarking (e.g., `charizard.bmp`, `lena_color.bmp`) are included for demonstrative purposes only and are **not owned by the project author**:

- **Lena_color**: Commonly used in image processing research; its use is historically accepted in academic contexts, though not officially licensed for redistribution.
- **Charizard**: Copyright © Nintendo. Used here strictly for non-commercial benchmarking.

## License
This project is licensed under the [MIT License](LICENSE).