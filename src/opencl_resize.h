#ifndef OPENCL_RESIZE_H
#define OPENCL_RESIZE_H

#ifdef USE_OPENCL

#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>
#include <string>
#include <vector>

class OpenCLResize {
public:
    OpenCLResize();
    ~OpenCLResize();

    void resize(const float* input, float* output,
               int input_width, int input_height,
               int output_width, int output_height);

private:
    void init_opencl();
    void cleanup();
    std::string load_kernel_source(const char* filename);

    cl_platform_id m_platform;
    cl_device_id m_device;
    cl_context m_context;
    cl_command_queue m_queue;
    cl_program m_program;
    cl_kernel m_kernel;
    
    bool m_initialized;
};

#endif // USE_OPENCL
#endif // OPENCL_RESIZE_H
