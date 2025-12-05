@echo off
REM Run the benchmark with test parameters

REM Source Intel oneAPI environment for runtime DLLs
call "C:\Program Files (x86)\Intel\oneAPI\setvars.bat" > nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo WARNING: Failed to source Intel oneAPI environment. Runtime may fail if DLLs are not in PATH.
)

set EXE=build\benchmark.exe

if not exist %EXE% (
    echo ERROR: Executable not found at %EXE%
    echo Please run build.bat first.
    exit /b 1
)

REM Default test parameters
set INPUT_W=1920
set INPUT_H=1080
set OUTPUT_W=640
set OUTPUT_H=480
set ITERS=100

echo Running benchmark: %EXE% %INPUT_W% %INPUT_H% %OUTPUT_W% %OUTPUT_H% %ITERS%
echo.

%EXE% %INPUT_W% %INPUT_H% %OUTPUT_W% %OUTPUT_H% %ITERS%

if %ERRORLEVEL% neq 0 (
    echo.
    echo ERROR: Benchmark failed with error code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

echo.
echo Test completed successfully!
