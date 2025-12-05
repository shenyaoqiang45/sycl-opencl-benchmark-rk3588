@echo off
REM Build script for Windows using Intel oneAPI DPC++ compiler
REM This script sources the Intel oneAPI environment and builds with Ninja + icx

echo === Building SYCL vs OpenCL Benchmark (Windows with Intel oneAPI) ===

REM Source Intel oneAPI environment
call "C:\Program Files (x86)\Intel\oneAPI\setvars.bat"
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to source Intel oneAPI environment. Please ensure Intel oneAPI is installed.
    exit /b 1
)

REM Create build directory
if not exist build mkdir build

REM Configure with CMake using Ninja and icx compiler
echo Configuring with CMake (Ninja + icx)...
cmake -B build -G "Ninja" -DCMAKE_CXX_COMPILER=icx -DCMAKE_BUILD_TYPE=Release -DBUILD_OPENCL=ON -DBUILD_SYCL=ON
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed.
    exit /b 1
)

REM Build
echo Building...
cmake --build build --config Release
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed.
    exit /b 1
)

echo.
echo Build completed successfully!
echo Executable: build\benchmark.exe
echo.
echo Usage: .\build\benchmark.exe ^<input_width^> ^<input_height^> ^<output_width^> ^<output_height^> ^<iterations^>
echo Example: .\build\benchmark.exe 1920 1080 640 480 100
