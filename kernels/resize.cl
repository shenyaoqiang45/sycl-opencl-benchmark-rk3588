__kernel void resize_bilinear(
    __global const float* input,
    __global float* output,
    int input_width,
    int input_height,
    int output_width,
    int output_height)
{
    int out_x = get_global_id(0);
    int out_y = get_global_id(1);

    if (out_x >= output_width || out_y >= output_height) {
        return;
    }

    // Calculate source coordinates
    float src_x = (out_x + 0.5f) * input_width / output_width - 0.5f;
    float src_y = (out_y + 0.5f) * input_height / output_height - 0.5f;

    // Clamp to valid range
    src_x = fmax(0.0f, fmin(src_x, (float)(input_width - 1)));
    src_y = fmax(0.0f, fmin(src_y, (float)(input_height - 1)));

    int x0 = (int)src_x;
    int y0 = (int)src_y;
    int x1 = min(x0 + 1, input_width - 1);
    int y1 = min(y0 + 1, input_height - 1);

    float wx = src_x - x0;
    float wy = src_y - y0;

    // Bilinear interpolation for each channel (RGB)
    for (int c = 0; c < 3; c++) {
        float p00 = input[(y0 * input_width + x0) * 3 + c];
        float p10 = input[(y0 * input_width + x1) * 3 + c];
        float p01 = input[(y1 * input_width + x0) * 3 + c];
        float p11 = input[(y1 * input_width + x1) * 3 + c];

        float p0 = p00 * (1.0f - wx) + p10 * wx;
        float p1 = p01 * (1.0f - wx) + p11 * wx;
        float p = p0 * (1.0f - wy) + p1 * wy;

        output[(out_y * output_width + out_x) * 3 + c] = p;
    }
}
