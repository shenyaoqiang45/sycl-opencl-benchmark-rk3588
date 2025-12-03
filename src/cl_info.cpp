#include <CL/cl.h>
#include <iostream>
#include <vector>

int main() {
    cl_uint num_platforms;
    cl_int err = clGetPlatformIDs(0, nullptr, &num_platforms);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to get number of platforms: " << err << std::endl;
        return 1;
    }

    std::cout << "Number of platforms: " << num_platforms << std::endl;

    std::vector<cl_platform_id> platforms(num_platforms);
    err = clGetPlatformIDs(num_platforms, platforms.data(), nullptr);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to get platform IDs: " << err << std::endl;
        return 1;
    }

    for (cl_uint i = 0; i < num_platforms; ++i) {
        char buffer[1024];
        clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(buffer), buffer, nullptr);
        std::cout << "Platform " << i << ": " << buffer << std::endl;

        cl_uint num_devices;
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &num_devices);
        if (err != CL_SUCCESS) {
            std::cerr << "  Failed to get number of devices: " << err << std::endl;
            continue;
        }

        std::vector<cl_device_id> devices(num_devices);
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, num_devices, devices.data(), nullptr);
        if (err != CL_SUCCESS) {
            std::cerr << "  Failed to get device IDs: " << err << std::endl;
            continue;
        }

        for (cl_uint j = 0; j < num_devices; ++j) {
            clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(buffer), buffer, nullptr);
            std::cout << "  Device " << j << ": " << buffer << std::endl;
            
            cl_device_type type;
            clGetDeviceInfo(devices[j], CL_DEVICE_TYPE, sizeof(type), &type, nullptr);
            std::cout << "    Type: " << (type & CL_DEVICE_TYPE_GPU ? "GPU" : "CPU/Other") << std::endl;

            // Try to create context
            cl_context context = clCreateContext(nullptr, 1, &devices[j], nullptr, nullptr, &err);
            if (err != CL_SUCCESS) {
                std::cerr << "    Failed to create context: " << err << std::endl;
            } else {
                std::cout << "    Context created successfully!" << std::endl;
                clReleaseContext(context);
            }
        }
    }

    return 0;
}
