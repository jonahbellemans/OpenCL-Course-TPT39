__kernel void apply_3x3_convolution(__global const float *input,
                                __global const float *conv_kernel,
                                __global float *restrict result)
{
  int i = get_global_id(0);
  int j = get_global_id(1);

  int cols = 640;
  int rows = 360;

  result[i * cols + j] = 0;

  for (int k = 0; k < 3; ++k) {
    for (int l = 0; l < 3; ++l) {
      result[i * cols + j] += input[(i * (cols + 2) + j) + (k * (cols + 2)) + l] * conv_kernel[k * 3 + l];
    }
  }
}
