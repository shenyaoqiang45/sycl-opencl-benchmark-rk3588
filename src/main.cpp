#include <iostream>
#include <vector>
#include <memory>
#include <iomanip>
#include "timer.h"
#include "image_utils.h"

#ifdef USE_OPENCL
#include "opencl_resize.h"
#endif

#ifdef USE_SYCL
#include "sycl_resize.h"
#endif

void print_usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name 
              << " <input_width> <input_height> <output_width> <output_height> <iterations>\n";
    std::cout << "Example: " << prog_name << " 1920 1080 640 480 100\n";
}

void print_results(const std::string& name, double total_time, int iterations) {
    double avg_time = total_time / iterations;
    std::cout << std::fixed << std::setprecision(3);
    std::cout << name << ":\n";
    std::cout << "  Total time: " << total_time << " ms\n";
    std::cout << "  Average time: " << avg_time << " ms\n";
    std::cout << "  FPS: " << (1000.0 / avg_time) << "\n\n";
}

int main(int argc, char** argv) {
    if (argc != 6) {
        print_usage(argv[0]);
        return 1;
    }

    int input_width = std::atoi(argv[1]);
    int input_height = std::atoi(argv[2]);
    int output_width = std::atoi(argv[3]);
    int output_height = std::atoi(argv[4]);
    int iterations = std::atoi(argv[5]);

    if (input_width <= 0 || input_height <= 0 || 
        output_width <= 0 || output_height <= 0 || iterations <= 0) {
        std::cerr << "Error: All parameters must be positive integers\n";
        return 1;
    }

    std::cout << "=== SYCL vs OpenCL Benchmark on RK3588 ===\n";
    std::cout << "Input size: " << input_width << "x" << input_height << "\n";
    std::cout << "Output size: " << output_width << "x" << output_height << "\n";
    std::cout << "Iterations: " << iterations << "\n\n";

    // Generate test image
    std::cout << "Generating test image...\n";
    auto input_image = generate_test_image(input_width, input_height);
    std::vector<float> output_image(output_width * output_height * 3);

    // Warmup iterations
    const int warmup_iterations = 5;

#ifdef USE_OPENCL
    std::cout << "Initializing OpenCL...\n";
    try {
        OpenCLResize opencl_resizer;
        
        // Warmup
        std::cout << "OpenCL warmup...\n";
        for (int i = 0; i < warmup_iterations; i++) {
            opencl_resizer.resize(input_image.data(), output_image.data(),
                                input_width, input_height,
                                output_width, output_height);
        }

        // Benchmark
        std::cout << "Running OpenCL benchmark...\n";
        Timer timer;
        timer.start();
        for (int i = 0; i < iterations; i++) {
            opencl_resizer.resize(input_image.data(), output_image.data(),
                                input_width, input_height,
                                output_width, output_height);
        }
        double opencl_time = timer.stop();
        print_results("OpenCL", opencl_time, iterations);

    } catch (const std::exception& e) {
        std::cerr << "OpenCL Error: " << e.what() << "\n";
    }
#else
    std::cout << "OpenCL support not compiled\n";
#endif

#ifdef USE_SYCL
    std::cout << "Initializing SYCL (AdaptiveCpp)...\n";
    try {
        SYCLResize sycl_resizer;
        
        // Warmup
        std::cout << "SYCL warmup...\n";
        for (int i = 0; i < warmup_iterations; i++) {
            sycl_resizer.resize(input_image.data(), output_image.data(),
                              input_width, input_height,
                              output_width, output_height);
        }

        // Benchmark
        std::cout << "Running SYCL benchmark...\n";
        Timer timer;
        timer.start();
        for (int i = 0; i < iterations; i++) {
            sycl_resizer.resize(input_image.data(), output_image.data(),
                              input_width, input_height,
                              output_width, output_height);
        }
        double sycl_time = timer.stop();
        print_results("SYCL (AdaptiveCpp)", sycl_time, iterations);

    } catch (const std::exception& e) {
        std::cerr << "SYCL Error: " << e.what() << "\n";
    }
#else
    std::cout << "SYCL support not compiled\n";
#endif

    std::cout << "Benchmark completed!\n";
    return 0;
}
