// OpenMP synchronization test: vector sum c = a + b split across threads,
// then one explicit barrier and one critical section (global checksum).
// Tests the tracer's sync markers: each thread's trace should contain the
// barrier ($0) and one critical start/end pair ($2/$3).
//   default:     scalar loop (x86 baseline)
//   -DUSE_VIMA:  one _vim2K_fadds call per 2048-element chunk (traced as VIMA)
#include <cstdio>
#include <cstdlib>
#include <omp.h>
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
    double checksum = 0;

    ORCS_tracing_start();
    #pragma omp parallel
    {
        int nt = omp_get_num_threads(), t = omp_get_thread_num();
        // Static split in VSZ-element chunks: thread t gets a contiguous slice.
        int chunks = N / VSZ;
        int lo = (int)((long)chunks * t / nt) * VSZ;
        int hi = (int)((long)chunks * (t + 1) / nt) * VSZ;
#ifdef USE_VIMA
        for (int i = lo; i < hi; i += VSZ) _vim2K_fadds(&a[i], &b[i], &c[i]);
#else
        for (int i = lo; i < hi; i++) c[i] = a[i] + b[i];
#endif
        #pragma omp barrier
        // After the barrier all of c is written: sum a slice that belongs
        // to the next thread, so the barrier is actually needed.
        int u = (t + 1) % nt;
        int ulo = (int)((long)chunks * u / nt) * VSZ;
        int uhi = (int)((long)chunks * (u + 1) / nt) * VSZ;
        double part = 0;
        for (int i = ulo; i < uhi; i++) part += c[i];
        #pragma omp critical
        checksum += part;
    }
    ORCS_tracing_stop();

    double expect = 3.0 * ((double)N * (N - 1) / 2);
    printf("checksum %.0f (expected %.0f)\n", checksum, expect);
    free(a); free(b); free(c);
    return checksum == expect ? 0 : 1;
}
