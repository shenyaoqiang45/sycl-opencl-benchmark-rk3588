#include "cpu_resize.h"
#include <cmath>
#include <algorithm>

CPUResize::CPUResize() {}

CPUResize::~CPUResize() {}

void CPUResize::resize(const float* input, float* output,
                      int input_width, int input_height,
                      int output_width, int output_height) {
    
    float x_ratio = (float)(input_width - 1) / output_width;
    float y_ratio = (float)(input_height - 1) / output_height;

    #pragma omp parallel for collapse(2)
    for (int y = 0; y < output_height; y++) {
        for (int x = 0; x < output_width; x++) {
            int x_l = (int)(x_ratio * x);
            int y_l = (int)(y_ratio * y);
            int x_h = std::min(x_l + 1, input_width - 1);
            int y_h = std::min(y_l + 1, input_height - 1);

            float x_weight = (x_ratio * x) - x_l;
            float y_weight = (y_ratio * y) - y_l;

            float a = x_weight * y_weight;
            float b = (1.0f - x_weight) * y_weight;
            float c = x_weight * (1.0f - y_weight);
            float d = (1.0f - x_weight) * (1.0f - y_weight);

            for (int c_idx = 0; c_idx < 3; c_idx++) {
                float val = a * input[(y_h * input_width + x_h) * 3 + c_idx] +
                            b * input[(y_h * input_width + x_l) * 3 + c_idx] +
                            c * input[(y_l * input_width + x_h) * 3 + c_idx] +
                            d * input[(y_l * input_width + x_l) * 3 + c_idx];
                
                output[(y * output_width + x) * 3 + c_idx] = val;
            }
        }
    }
}
