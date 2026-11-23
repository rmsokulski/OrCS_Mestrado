// Vector sum c = a + b.
//   default:     scalar loop (x86 baseline, no function call overhead)
//   -DUSE_VIMA:  one _vim2K_fadds call per 2048-element chunk (traced as VIMA)
#include <cstdlib>
#include "orcs_tracing.h"

#define N   (1 << 20)
#define VSZ 2048

#ifdef USE_VIMA
// Must stay C++ (mangled _Z12_vim2K_fadds...) and noinline: the tracer matches
// the name after the 4-char "_Z12" prefix. Args: (src1, src2, dst).
__attribute__((noinline)) void _vim2K_fadds(float *a, float *b, float *c) {
    for (int i = 0; i < VSZ; i++) c[i] = a[i] + b[i];
}
#endif

int main() {
    float *a = (float*)aligned_alloc(8192, N * sizeof(float));
    float *b = (float*)aligned_alloc(8192, N * sizeof(float));
    float *c = (float*)aligned_alloc(8192, N * sizeof(float));
    for (int i = 0; i < N; i++) { a[i] = i; b[i] = 2 * i; }

    ORCS_tracing_start();
#ifdef USE_VIMA
    for (int i = 0; i < N; i += VSZ) _vim2K_fadds(&a[i], &b[i], &c[i]);
#else
    for (int i = 0; i < N; i++) c[i] = a[i] + b[i];
#endif
    ORCS_tracing_stop();

    int r = (int)c[N - 1];   // keep the result live
    free(a); free(b); free(c);
    return r;
}
