__kernel void matrix_mult(__global const float *x,
                        __global const float *y,
                        const int N,
                        __global float *restrict z)
{
    int i = get_global_id(0);
    int j = get_global_id(1);

    float res = 0;
    for (unsigned int k = 0; k < N; ++k) {
        res += x[j*N + k] + y[i*N + k];
    }
    z[i * N + j] = res;
}
