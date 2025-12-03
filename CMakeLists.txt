cmake_minimum_required(VERSION 3.16)
project(SYCL_OpenCL_Benchmark LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_OPENCL "Build OpenCL implementation" ON)
option(BUILD_SYCL "Build SYCL (AdaptiveCpp) implementation" ON)

# Find OpenCL
if(BUILD_OPENCL)
    find_package(OpenCL REQUIRED)
    if(OpenCL_FOUND)
        message(STATUS "OpenCL found: ${OpenCL_INCLUDE_DIRS}")
    endif()
endif()

# Find AdaptiveCpp (formerly hipSYCL)
if(BUILD_SYCL)
    find_package(AdaptiveCpp CONFIG)
    if(NOT AdaptiveCpp_FOUND)
        find_package(hipSYCL CONFIG)
        if(hipSYCL_FOUND)
            set(AdaptiveCpp_FOUND TRUE)
        endif()
    endif()
    
    if(AdaptiveCpp_FOUND)
        message(STATUS "AdaptiveCpp/hipSYCL found")
    else()
        message(WARNING "AdaptiveCpp not found, SYCL build disabled")
        set(BUILD_SYCL OFF)
    endif()
endif()

# Source files
set(COMMON_SOURCES
    src/main.cpp
    src/image_utils.cpp
    src/cpu_resize.cpp
)

set(OPENCL_SOURCES
    src/opencl_resize.cpp
)

set(SYCL_SOURCES
    src/sycl_resize.cpp
)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Find OpenMP
find_package(OpenMP)

# Create executable
add_executable(benchmark ${COMMON_SOURCES})

if(OpenMP_CXX_FOUND)
    target_link_libraries(benchmark PRIVATE OpenMP::OpenMP_CXX)
endif()

# Add OpenCL support
if(BUILD_OPENCL)
    target_sources(benchmark PRIVATE ${OPENCL_SOURCES})
    target_link_libraries(benchmark PRIVATE OpenCL::OpenCL)
    target_compile_definitions(benchmark PRIVATE USE_OPENCL)
    
    # Copy OpenCL kernels to build directory
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/kernels 
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Add SYCL support
if(BUILD_SYCL)
    target_sources(benchmark PRIVATE ${SYCL_SOURCES})
    target_compile_definitions(benchmark PRIVATE USE_SYCL)
    
    if(AdaptiveCpp_FOUND)
        add_sycl_to_target(TARGET benchmark SOURCES ${SYCL_SOURCES})
    endif()
endif()

# Link pthread and other required libraries
find_package(Threads REQUIRED)
target_link_libraries(benchmark PRIVATE Threads::Threads)

# ARM Mali optimizations for RK3588
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    target_compile_options(benchmark PRIVATE -march=armv8-a -mtune=cortex-a76)
    message(STATUS "Enabled ARM optimizations for RK3588")
endif()

# Print configuration
message(STATUS "Build configuration:")
message(STATUS "  OpenCL support: ${BUILD_OPENCL}")
message(STATUS "  SYCL support: ${BUILD_SYCL}")
