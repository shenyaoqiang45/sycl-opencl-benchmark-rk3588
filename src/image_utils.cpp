#include "image_utils.h"
#include <cmath>
#include <algorithm>

std::vector<float> generate_test_image(int width, int height) {
    std::vector<float> image(width * height * 3);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            
            // Generate a gradient pattern
            float fx = static_cast<float>(x) / width;
            float fy = static_cast<float>(y) / height;
            
            image[idx + 0] = fx;                    // R
            image[idx + 1] = fy;                    // G
            image[idx + 2] = 0.5f * (fx + fy);      // B
        }
    }
    
    return image;
}

bool verify_images(const float* img1, const float* img2, 
                  int width, int height, float tolerance) {
    int total_pixels = width * height * 3;
    int diff_count = 0;
    
    for (int i = 0; i < total_pixels; i++) {
        float diff = std::abs(img1[i] - img2[i]);
        if (diff > tolerance) {
            diff_count++;
        }
    }
    
    float error_rate = static_cast<float>(diff_count) / total_pixels;
    return error_rate < 0.01f; // Allow 1% error rate
}
