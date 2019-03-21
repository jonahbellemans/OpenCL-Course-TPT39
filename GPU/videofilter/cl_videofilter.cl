__kernel void apply_3x3_convolution(__global const float *input,
                                __global const float *conv_kernel,
                                __global float *restrict result)
{
  int i = get_global_id(0);
  int j = get_global_id(1);

  for (int k = 0; k < 3, ++k) {
    for (int l = 0; l < 3, ++l) {
      result[i * 360 + j] += input[(i + k) + j + l] * conv_kernel[k * 3 + l];
    }
  }
}
