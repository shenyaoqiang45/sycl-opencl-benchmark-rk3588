#ifdef USE_OPENCL

#include "opencl_resize.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>

#define CHECK_CL_ERROR(err, msg) \
    if (err != CL_SUCCESS) { \
        throw std::runtime_error(std::string(msg) + " (Error code: " + std::to_string(err) + ")"); \
    }

OpenCLResize::OpenCLResize() 
    : m_platform(nullptr), m_device(nullptr), m_context(nullptr),
      m_queue(nullptr), m_program(nullptr), m_kernel(nullptr),
      m_initialized(false) {
    init_opencl();
}

OpenCLResize::~OpenCLResize() {
    cleanup();
}

std::string OpenCLResize::load_kernel_source(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error(std::string("Failed to open kernel file: ") + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void OpenCLResize::init_opencl() {
    cl_int err;

    // Get platform
    cl_uint num_platforms;
    err = clGetPlatformIDs(1, &m_platform, &num_platforms);
    CHECK_CL_ERROR(err, "Failed to get OpenCL platform");

    if (num_platforms == 0) {
        throw std::runtime_error("No OpenCL platforms found");
    }

    // Get device (prefer GPU)
    err = clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_GPU, 1, &m_device, nullptr);
    if (err != CL_SUCCESS) {
        std::cout << "GPU not found, trying CPU...\n";
        err = clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_ALL, 1, &m_device, nullptr);
        CHECK_CL_ERROR(err, "Failed to get OpenCL device");
    }

    // Print device info
    char device_name[128];
    clGetDeviceInfo(m_device, CL_DEVICE_NAME, sizeof(device_name), device_name, nullptr);
    std::cout << "Using OpenCL device: " << device_name << "\n";

    // Create context
    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    CHECK_CL_ERROR(err, "Failed to create OpenCL context");

    // Create command queue
    m_queue = clCreateCommandQueue(m_context, m_device, 0, &err);
    CHECK_CL_ERROR(err, "Failed to create command queue");

    // Load and compile kernel
    std::string kernel_source = load_kernel_source("kernels/resize.cl");
    const char* source_str = kernel_source.c_str();
    size_t source_size = kernel_source.length();

    m_program = clCreateProgramWithSource(m_context, 1, &source_str, &source_size, &err);
    CHECK_CL_ERROR(err, "Failed to create program");

    err = clBuildProgram(m_program, 1, &m_device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
        std::vector<char> log(log_size);
        clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG, log_size, log.data(), nullptr);
        std::cerr << "Build log:\n" << log.data() << "\n";
        throw std::runtime_error("Failed to build program");
    }

    m_kernel = clCreateKernel(m_program, "resize_bilinear", &err);
    CHECK_CL_ERROR(err, "Failed to create kernel");

    m_initialized = true;
}

void OpenCLResize::cleanup() {
    if (m_kernel) clReleaseKernel(m_kernel);
    if (m_program) clReleaseProgram(m_program);
    if (m_queue) clReleaseCommandQueue(m_queue);
    if (m_context) clReleaseContext(m_context);
}

void OpenCLResize::resize(const float* input, float* output,
                         int input_width, int input_height,
                         int output_width, int output_height) {
    if (!m_initialized) {
        throw std::runtime_error("OpenCL not initialized");
    }

    cl_int err;
    
    size_t input_size = input_width * input_height * 3 * sizeof(float);
    size_t output_size = output_width * output_height * 3 * sizeof(float);

    // Create buffers
    cl_mem input_buffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                        input_size, (void*)input, &err);
    CHECK_CL_ERROR(err, "Failed to create input buffer");

    cl_mem output_buffer = clCreateBuffer(m_context, CL_MEM_WRITE_ONLY,
                                         output_size, nullptr, &err);
    CHECK_CL_ERROR(err, "Failed to create output buffer");

    // Set kernel arguments
    err = clSetKernelArg(m_kernel, 0, sizeof(cl_mem), &input_buffer);
    err |= clSetKernelArg(m_kernel, 1, sizeof(cl_mem), &output_buffer);
    err |= clSetKernelArg(m_kernel, 2, sizeof(int), &input_width);
    err |= clSetKernelArg(m_kernel, 3, sizeof(int), &input_height);
    err |= clSetKernelArg(m_kernel, 4, sizeof(int), &output_width);
    err |= clSetKernelArg(m_kernel, 5, sizeof(int), &output_height);
    CHECK_CL_ERROR(err, "Failed to set kernel arguments");

    // Execute kernel
    size_t global_work_size[2] = {(size_t)output_width, (size_t)output_height};
    size_t local_work_size[2] = {16, 16};
    
    err = clEnqueueNDRangeKernel(m_queue, m_kernel, 2, nullptr,
                                global_work_size, local_work_size,
                                0, nullptr, nullptr);
    CHECK_CL_ERROR(err, "Failed to execute kernel");

    // Read results
    err = clEnqueueReadBuffer(m_queue, output_buffer, CL_TRUE, 0,
                             output_size, output, 0, nullptr, nullptr);
    CHECK_CL_ERROR(err, "Failed to read output buffer");

    // Cleanup buffers
    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer);
}

#endif // USE_OPENCL
