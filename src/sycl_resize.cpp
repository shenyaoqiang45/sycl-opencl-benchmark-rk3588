#ifdef USE_SYCL

#include "sycl_resize.h"
#include <iostream>
#include <stdexcept>

SYCLResize::SYCLResize() {
    try {
        // Prefer a GPU device if available. Otherwise fall back to default device.
        bool device_selected = false;

        auto platforms = sycl::platform::get_platforms();
        for (const auto &plat : platforms) {
            auto devices = plat.get_devices();
            for (const auto &dev : devices) {
                auto dev_type = dev.get_info<sycl::info::device::device_type>();
                if (dev_type == sycl::info::device_type::gpu) {
                    m_queue = std::make_unique<sycl::queue>(dev);
                    std::cout << "Selected SYCL GPU device: "
                              << dev.get_info<sycl::info::device::name>() << "\n";
                    std::cout << "  Platform: " << plat.get_info<sycl::info::platform::name>()
                              << " (" << plat.get_info<sycl::info::platform::vendor>() << ")\n";
                    device_selected = true;
                    break;
                }
            }
            if (device_selected) break;
        }

        if (!device_selected) {
            // No GPU found; use default selector
            std::cout << "GPU not available, using default device\n";
            m_queue = std::make_unique<sycl::queue>();
            auto dev = m_queue->get_device();
            std::cout << "Using SYCL device: " << dev.get_info<sycl::info::device::name>() << "\n";
            std::cout << "  Platform: " << m_queue->get_context().get_platform().get_info<sycl::info::platform::name>()
                      << " (" << m_queue->get_context().get_platform().get_info<sycl::info::platform::vendor>() << ")\n";
        }
    } catch (const sycl::exception& e) {
        throw std::runtime_error(std::string("SYCL initialization failed: ") + e.what());
    }
}

SYCLResize::~SYCLResize() {
    if (m_queue) {
        m_queue->wait();
    }
}

void SYCLResize::resize(const float* input, float* output,
                       int input_width, int input_height,
                       int output_width, int output_height) {
    if (!m_queue) {
        throw std::runtime_error("SYCL queue not initialized");
    }

    size_t input_size = input_width * input_height * 3;
    size_t output_size = output_width * output_height * 3;

    try {
        // Create buffers
        sycl::buffer<float, 1> input_buffer(input, sycl::range<1>(input_size));
        sycl::buffer<float, 1> output_buffer(output, sycl::range<1>(output_size));

        // Submit kernel
        m_queue->submit([&](sycl::handler& h) {
            auto input_acc = input_buffer.get_access<sycl::access::mode::read>(h);
            auto output_acc = output_buffer.get_access<sycl::access::mode::write>(h);

            h.parallel_for(sycl::range<2>(output_height, output_width),
                          [=](sycl::id<2> idx) {
                int out_y = idx[0];
                int out_x = idx[1];

                // Calculate source coordinates
                float src_x = (out_x + 0.5f) * input_width / output_width - 0.5f;
                float src_y = (out_y + 0.5f) * input_height / output_height - 0.5f;

                // Clamp to valid range
                src_x = sycl::fmax(0.0f, sycl::fmin(src_x, input_width - 1.0f));
                src_y = sycl::fmax(0.0f, sycl::fmin(src_y, input_height - 1.0f));

                int x0 = static_cast<int>(src_x);
                int y0 = static_cast<int>(src_y);
                int x1 = sycl::min(x0 + 1, input_width - 1);
                int y1 = sycl::min(y0 + 1, input_height - 1);

                float wx = src_x - x0;
                float wy = src_y - y0;

                // Bilinear interpolation for each channel
                for (int c = 0; c < 3; c++) {
                    float p00 = input_acc[(y0 * input_width + x0) * 3 + c];
                    float p10 = input_acc[(y0 * input_width + x1) * 3 + c];
                    float p01 = input_acc[(y1 * input_width + x0) * 3 + c];
                    float p11 = input_acc[(y1 * input_width + x1) * 3 + c];

                    float p0 = p00 * (1.0f - wx) + p10 * wx;
                    float p1 = p01 * (1.0f - wx) + p11 * wx;
                    float p = p0 * (1.0f - wy) + p1 * wy;

                    output_acc[(out_y * output_width + out_x) * 3 + c] = p;
                }
            });
        });

        // Wait for completion
        m_queue->wait_and_throw();

    } catch (const sycl::exception& e) {
        throw std::runtime_error(std::string("SYCL resize failed: ") + e.what());
    }
}

#endif // USE_SYCL
