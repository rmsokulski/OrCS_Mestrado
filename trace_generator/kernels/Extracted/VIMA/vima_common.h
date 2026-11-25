// =========================================================================================
// VIMA instructions used by the scans, with the current VIMA ISA only
// -----------------------------------------------------------------------------------------
// sinuca_tracer (-trace iVIM) replaces each call to these functions by one VIMA instruction
// and records its pointer arguments as 8 KB vector operands. It recognizes them by their
// C++ symbol names (_Z12_vim2K_<op>...), so the C functions get those names with asm labels.
// OrCS models only timing and memory traffic; the bodies (vima_isa.c) compute the result.
//
// Semantics chosen for the existing instructions (all vectors are VIMA_LANES floats):
//   fmovs(value, dst)      dst[i] = *value              (1 operand: dst)
//   fcpys(src, dst)        dst[i] = src[i]              (2 operands)
//   fadds/fsubs/fmuls      dst[i] = a[i] op b[i]        (3 operands)
//   fslts(a, b, dst)       dst[i] = a[i] < b[i] ? 1 : 0 (3 operands)
//   fcums(src, dst)        dst[0] = sum of src[i]       (2 operands; reduction)
//   fgtrs(base, ids, dst)  dst[i] = base[ids[i]]        (3 operands; gather)
// The tracer records fgtrs's base as one 8 KB read at `base`, not the gathered addresses.
// =========================================================================================
#ifndef VIMA_COMMON_H
#define VIMA_COMMON_H

#include <stdint.h>
#include "scan_common.h"

#define VIMA_LANES       2048                              /// Elements per _vim2K_ vector
#define VIMA_VECTOR_SIZE (VIMA_LANES * sizeof(float))      /// 8 KB

void vima_fmovs(const float *value, float *dst)                 __asm__("_Z12_vim2K_fmovsPKfPf");
void vima_fcpys(const float *src, float *dst)                   __asm__("_Z12_vim2K_fcpysPKfPf");
void vima_fadds(const float *a, const float *b, float *dst)     __asm__("_Z12_vim2K_faddsPKfS0_Pf");
void vima_fsubs(const float *a, const float *b, float *dst)     __asm__("_Z12_vim2K_fsubsPKfS0_Pf");
void vima_fmuls(const float *a, const float *b, float *dst)     __asm__("_Z12_vim2K_fmulsPKfS0_Pf");
void vima_fslts(const float *a, const float *b, float *dst)     __asm__("_Z12_vim2K_fsltsPKfS0_Pf");
void vima_fcums(const float *src, float *dst)                   __asm__("_Z12_vim2K_fcumsPKfPf");
void vima_fgtrs(const float *base, const uint32_t *ids, float *dst) __asm__("_Z12_vim2K_fgtrsPKfPKjPf");

// -----------------------------------------------------------------------------------------
// Scratch vectors, 8 KB aligned
// -----------------------------------------------------------------------------------------
typedef struct {
    float *ones;            /// All 1.0
    float *low, *high;      /// Current predicate bounds, broadcast
    float *below, *above;   /// v < low, high < v
    float *outside;         /// below + above
    float *passes;          /// 1.0 where low <= v <= high
    float *values;          /// Gathered values / column 0 times the mask
    float *result;          /// fcums output (element 0)
} VimaScratch;

static inline float *vima_vector_alloc(size_t vectors) {
    float *v = aligned_alloc(VIMA_VECTOR_SIZE, (vectors ? vectors : 1) * VIMA_VECTOR_SIZE);
    assert(v);
    return v;
}

static inline VimaScratch vima_scratch_create(void) {
    VimaScratch s;
    float **all[] = { &s.ones, &s.low, &s.high, &s.below, &s.above, &s.outside, &s.passes, &s.values, &s.result };
    for (size_t i = 0; i < sizeof(all) / sizeof(all[0]); i++) *all[i] = vima_vector_alloc(1);
    const float one = 1.0f;
    vima_fmovs(&one, s.ones);
    return s;
}

static inline void vima_scratch_free(VimaScratch *s) {
    free(s->ones); free(s->low); free(s->high); free(s->below); free(s->above);
    free(s->outside); free(s->passes); free(s->values); free(s->result);
}

/// Broadcasts a predicate's bounds (2 VIMA instructions)
static inline void vima_set_bounds(VimaScratch *s, const Predicate *predicate) {
    vima_fmovs(&predicate->low, s->low);
    vima_fmovs(&predicate->high, s->high);
}

/// passes = 1.0 where low <= values <= high, else 0.0 (4 VIMA instructions).
/// below and above are never both 1 when low <= high, so passes = 1 - below - above.
static inline void vima_range(VimaScratch *s, const float *values, float *passes) {
    vima_fslts(values, s->low, s->below);
    vima_fslts(s->high, values, s->above);
    vima_fadds(s->below, s->above, s->outside);
    vima_fsubs(s->ones, s->outside, passes);
}

/// Sum of a vector, read back by the CPU (1 VIMA instruction + 1 load)
static inline float vima_sum(VimaScratch *s, const float *vector) {
    vima_fcums(vector, s->result);
    return s->result[0];
}

static inline bool range_passes(const Predicate *predicate, float value) {
    return predicate->low <= value && value <= predicate->high;
}

#endif
