#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include <vector>

// Generate a test image with gradient pattern (RGB format)
std::vector<float> generate_test_image(int width, int height);

// Verify two images are similar (for correctness testing)
bool verify_images(const float* img1, const float* img2, 
                  int width, int height, float tolerance = 0.01f);

#endif // IMAGE_UTILS_H
