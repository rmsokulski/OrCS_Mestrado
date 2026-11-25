/*
 * Shared types and helpers for the four scan variants extracted from
 * MultidimensionalAdaptiveIndexing (FullScan / FullScanCandidateList).
 *
 *                     | re-evaluates all predicates | skips covered attributes
 *   ------------------+-----------------------------+-------------------------
 *   bitmap            | scan_bitmap_reeval.c        | scan_bitmap_skip.c
 *   selection vector  | scan_selvec_reeval.c        | scan_selvec_skip.c
 *
 * Common interface:
 *   columns[c][row]            columnar float table
 *   predicates[k]              {low, high, column}; row qualifies iff low <= v <= high
 *   partitions[p]              row range [low, high) to scan
 *   skip[p * col_count + c]    true => every row of partition p already satisfies the
 *                              predicate on column c (computed by the index search).
 *                              Ignored by the *_reeval variants.
 *   returns sum of column 0 over qualifying rows; *tuples_out = number of qualifying rows
 */
#ifndef SCAN_COMMON_H
#define SCAN_COMMON_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct { float low, high; size_t column; } Predicate;
typedef struct { size_t low, high; } Partition;

typedef double (*scan_fn)(float **columns, size_t col_count,
                          const Predicate *predicates, size_t predicate_count,
                          const Partition *partitions, const bool *skip,
                          size_t partition_count, size_t *tuples_out);

/* ---------------- bitmap ---------------- */
typedef struct { uint64_t *bits; size_t size; } BitVector;

/// Cache-line size in bytes; the bitmap starts on a cache-line boundary so that
/// chunks of whole cache lines assigned to different threads never share a line
#define CACHE_LINE_BYTES 64

static inline BitVector bv_create(size_t n) {
    size_t words = (n + 63) / 64;
    size_t bytes = (words ? words : 1) * sizeof(uint64_t);
    bytes = (bytes + CACHE_LINE_BYTES - 1) / CACHE_LINE_BYTES * CACHE_LINE_BYTES;   /// aligned_alloc needs a multiple
    BitVector bv = { aligned_alloc(CACHE_LINE_BYTES, bytes), n };
    assert(bv.bits);
    memset(bv.bits, 0, bytes);
    return bv;
}
static inline void bv_free(BitVector *bv) { free(bv->bits); bv->bits = NULL; }
static inline void bv_set(BitVector *bv, size_t i, bool v) {
    uint64_t mask = (uint64_t)1 << (i & 63);
    bv->bits[i >> 6] = (bv->bits[i >> 6] & ~mask) | ((uint64_t)v << (i & 63));
}
static inline bool bv_get(const BitVector *bv, size_t i) {
    return (bv->bits[i >> 6] >> (i & 63)) & 1;
}

/* ---------------- selection vector (candidate list) ---------------- */
typedef struct { uint32_t *data; size_t size, capacity; } SelVec;

static inline SelVec sv_create(size_t capacity) {
    SelVec sv = { NULL, 0, capacity ? capacity : 1024 };
    sv.data = malloc(sv.capacity * sizeof(uint32_t));
    assert(sv.data);
    return sv;
}
static inline void sv_free(SelVec *sv) { free(sv->data); sv->data = NULL; }
static inline void sv_grow_if_full(SelVec *sv) {
    if (sv->size == sv->capacity) {
        sv->capacity *= 2;
        sv->data = realloc(sv->data, sv->capacity * sizeof(uint32_t));
        assert(sv->data);
    }
}
static inline void sv_push_back(SelVec *sv, uint32_t v) {
    sv_grow_if_full(sv);
    sv->data[sv->size++] = v;
}
/* branch-free: always writes, advances only on match */
static inline void sv_maybe_push_back(SelVec *sv, uint32_t v, int match) {
    sv_grow_if_full(sv);
    sv->data[sv->size] = v;
    sv->size += match;
}

#endif
