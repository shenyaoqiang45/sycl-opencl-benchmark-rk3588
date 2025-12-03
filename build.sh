#!/bin/bash

# Build script for RK3588
# This script should be run on the RK3588 device

set -e

echo "=== Building SYCL vs OpenCL Benchmark ==="

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_OPENCL=ON \
    -DBUILD_SYCL=ON

# Build
echo "Building..."
make -j$(nproc)

echo "Build completed successfully!"
echo "Executable: build/benchmark"
echo ""
echo "Usage: ./benchmark <input_width> <input_height> <output_width> <output_height> <iterations>"
echo "Example: ./benchmark 1920 1080 640 480 100"
