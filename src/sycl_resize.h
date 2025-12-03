#ifndef SYCL_RESIZE_H
#define SYCL_RESIZE_H

#ifdef USE_SYCL

#include <sycl/sycl.hpp>
#include <memory>

class SYCLResize {
public:
    SYCLResize();
    ~SYCLResize();

    void resize(const float* input, float* output,
               int input_width, int input_height,
               int output_width, int output_height);

private:
    std::unique_ptr<sycl::queue> m_queue;
};

#endif // USE_SYCL
#endif // SYCL_RESIZE_H
