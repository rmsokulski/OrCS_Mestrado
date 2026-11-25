// =========================================================================================
// AVX-512 helpers shared by the SIMD scan variants.
// Requires: AVX512F, AVX512VL, AVX512DQ (build with -mavx512f -mavx512vl -mavx512dq).
// =========================================================================================
#ifndef SIMD_COMMON_H
#define SIMD_COMMON_H

#include <immintrin.h>
#include <stdint.h>

/// Floats per AVX-512 register
#define LANES 16

// -----------------------------------------------------------------------------------------
// Range predicate on 16 floats: lane i is set iff low <= value[i] <= high
// -----------------------------------------------------------------------------------------
/// Ordered compares: NaN fails, exactly like the scalar `low <= v && v <= high`
static inline __mmask16 range_mask(__m512 values, __m512 low, __m512 high) {
    __mmask16 above_low = _mm512_cmp_ps_mask(values, low, _CMP_GE_OQ);
    return _mm512_mask_cmp_ps_mask(above_low, values, high, _CMP_LE_OQ);
}

// -----------------------------------------------------------------------------------------
// Same, but only lanes already set in `active` are tested (others stay 0)
// -----------------------------------------------------------------------------------------
static inline __mmask16 range_mask_active(__mmask16 active, __m512 values, __m512 low, __m512 high) {
    __mmask16 above_low = _mm512_mask_cmp_ps_mask(active, values, low, _CMP_GE_OQ);
    return _mm512_mask_cmp_ps_mask(above_low, values, high, _CMP_LE_OQ);
}

// -----------------------------------------------------------------------------------------
// Adds 16 floats into a double accumulator (8 doubles), honoring a 16-bit mask
// -----------------------------------------------------------------------------------------
static inline __m512d add_floats_to_double_acc(__m512d acc, __mmask16 mask, __m512 values) {
    __m512d low_half  = _mm512_cvtps_pd(_mm512_castps512_ps256(values));
    __m512d high_half = _mm512_cvtps_pd(_mm512_extractf32x8_ps(values, 1));
    acc = _mm512_mask_add_pd(acc, (__mmask8)(mask & 0xFF), acc, low_half);
    acc = _mm512_mask_add_pd(acc, (__mmask8)(mask >> 8),   acc, high_half);
    return acc;
}

#endif
