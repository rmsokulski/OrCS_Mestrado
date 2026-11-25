// Bodies of the VIMA instructions (see vima_common.h). noinline + used: each call must stay
// a real call to the named symbol, so the tracer can replace it by one VIMA instruction.
#include "vima_common.h"

#define VIMA_OP __attribute__((noinline, used))

VIMA_OP void vima_fmovs(const float *value, float *dst) { for (size_t i = 0; i < VIMA_LANES; i++) dst[i] = *value; }
VIMA_OP void vima_fcpys(const float *src, float *dst)   { for (size_t i = 0; i < VIMA_LANES; i++) dst[i] = src[i]; }
VIMA_OP void vima_fadds(const float *a, const float *b, float *dst) { for (size_t i = 0; i < VIMA_LANES; i++) dst[i] = a[i] + b[i]; }
VIMA_OP void vima_fsubs(const float *a, const float *b, float *dst) { for (size_t i = 0; i < VIMA_LANES; i++) dst[i] = a[i] - b[i]; }
VIMA_OP void vima_fmuls(const float *a, const float *b, float *dst) { for (size_t i = 0; i < VIMA_LANES; i++) dst[i] = a[i] * b[i]; }
VIMA_OP void vima_fslts(const float *a, const float *b, float *dst) { for (size_t i = 0; i < VIMA_LANES; i++) dst[i] = a[i] < b[i] ? 1.0f : 0.0f; }
VIMA_OP void vima_fgtrs(const float *base, const uint32_t *ids, float *dst) { for (size_t i = 0; i < VIMA_LANES; i++) dst[i] = base[ids[i]]; }

VIMA_OP void vima_fence(void) { __asm__ volatile ("" ::: "memory"); }

VIMA_OP void vima_fcums(const float *src, float *dst) {
    float sum = 0.0f;
    for (size_t i = 0; i < VIMA_LANES; i++) sum += src[i];
    dst[0] = sum;
}
