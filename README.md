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

## Windows (Intel oneAPI / DPC++)

This project can be built natively on Windows using Intel oneAPI DPC++ compiler for SYCL support.

### Prerequisites
- Intel oneAPI Base Toolkit (includes DPC++ compiler and SYCL runtime)
  - Download: https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit-download.html
- CMake 3.18+ 
- Ninja build system (recommended, included with oneAPI)
- OpenCL drivers / SDK for your GPU vendor (for OpenCL build)

### Quick Build Steps

```cmd
.\build.bat
```

The script automatically:
1. Sources the Intel oneAPI environment (`setvars.bat`)
2. Configures CMake with Ninja and `icx` compiler
3. Builds the project in Release mode

The executable will be at: `build\benchmark.exe`

### Run Test

```cmd
.\run.bat
```

Or run manually with custom parameters:
```cmd
.\build\benchmark.exe 1920 1080 640 480 100
```

### Notes
- The build scripts automatically source Intel oneAPI environment and use the `icx` compiler (Intel C++ compiler)
- SYCL support is enabled via Intel DPC++ on Windows
- On RK3588/Linux, SYCL uses AdaptiveCpp/hipSYCL
- If you encounter "SYCL not found" errors, ensure Intel oneAPI is installed at the default location: `C:\Program Files (x86)\Intel\oneAPI\`

