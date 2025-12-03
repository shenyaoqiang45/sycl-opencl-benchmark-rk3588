# SYCL vs OpenCL Performance Benchmark on RK3588  test

This project compares the performance of OpenCL and AdaptiveCpp (SYCL) implementations on RK3588 platform.

## Features
- Image resize implementation using bilinear interpolation
- OpenCL implementation using Mali GPU
- SYCL implementation using AdaptiveCpp
- Performance benchmarking with timing utilities

## Building

### Prerequisites
- CMake 3.18+
- OpenCL SDK (Mali driver for RK3588)
- AdaptiveCpp (optional, for SYCL support)
- C++17 compatible compiler

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make -j4
```

### Build Options
- `-DBUILD_OPENCL=ON/OFF` - Enable/disable OpenCL implementation (default: ON)
- `-DBUILD_SYCL=ON/OFF` - Enable/disable SYCL implementation (default: ON)

## Running

```bash
./benchmark <input_width> <input_height> <output_width> <output_height> <iterations>
```

Example:
```bash
./benchmark 1920 1080 640 480 100
```

## RK3588 Specific Notes
- RK3588 uses Mali-G610 GPU
- OpenCL support via ARM Mali driver
- For optimal performance, ensure GPU frequency governor is set to performance mode

## Performance Tips
- Ensure the Mali GPU driver is properly installed
- Set GPU governor to performance mode: `echo performance > /sys/class/devfreq/fb000000.gpu/governor`
- Use sufficient iterations (>100) for stable measurements
- Run warmup iterations before benchmarking
