#pragma once

class CPUResize {
public:
    CPUResize();
    ~CPUResize();

    void resize(const float* input, float* output,
                int input_width, int input_height,
                int output_width, int output_height);
};
