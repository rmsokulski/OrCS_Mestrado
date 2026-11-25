// =========================================================================================
// Scan: BITMAP + SKIP OF COVERED ATTRIBUTES (AVX-512 + OpenMP)
// -----------------------------------------------------------------------------------------
// Parallel version of SIMD/scan_bitmap_skip_simd.c.
//
// Partitions are visited one after the other. Inside a partition, the loops over the
// full 64-bit bitmap words are split among the threads; each iteration writes one whole
// word, so there is no race. Words are handed out in chunks of whole cache lines to
// avoid false sharing between cores. The last, partial word is handled scalarly by a single
// thread. Predicates on attributes the index already guarantees are not evaluated.
// Sum and count are combined with a reduction.
// =========================================================================================
#include <omp.h>
#include "scan_common.h"
#include "simd_common.h"

/// Bitmap words per chunk given to a thread: 64 words = 512 B = 8 cache lines (4096 rows).
/// Chunks span whole cache lines (and the bitmap is cache-line aligned), so two threads
/// never write to the same line and no cross-core invalidations (false sharing) occur.
#define WORDS_PER_CHUNK 64

double scan_bitmap_skip_simd_par(float **columns, size_t column_count,
                                 const Predicate *predicates, size_t predicate_count,
                                 const Partition *partitions, const bool *skip,
                                 size_t partition_count, size_t *qualifying_count_out)
{
    double sum_of_first_column = 0.0;
    size_t qualifying_count = 0;

    for (size_t partition_id = 0; partition_id < partition_count; partition_id++) {

        // -----------------------------------------------------------------------------------------
        // Gets the row range, the skip flags of the partition and allocates one bit per row
        // -----------------------------------------------------------------------------------------
        size_t first_row = partitions[partition_id].low;
        size_t row_count = partitions[partition_id].high - first_row;
        size_t full_words = row_count / 64;          /// Words handled with SIMD (in parallel)
        size_t tail_start = full_words * 64;         /// First offset handled scalarly
        const bool *column_is_covered = &skip[partition_id * column_count];
        BitVector qualifies = bv_create(row_count);

        double partition_sum = 0.0;
        size_t partition_count_rows = 0;

        #pragma omp parallel reduction(+ : partition_sum, partition_count_rows)
        {
            // -----------------------------------------------------------------------------------------
            // Initializes the bitmap with the first predicate
            // -----------------------------------------------------------------------------------------
            const Predicate *predicate = &predicates[0];
            const float *column = columns[predicate->column] + first_row;

            if (column_is_covered[predicate->column]) {
                /// The index guarantees every row passes: set whole words without comparing
                #pragma omp for schedule(static, WORDS_PER_CHUNK)
                for (size_t word = 0; word < full_words; word++) {
                    qualifies.bits[word] = ~(uint64_t)0;
                }

                #pragma omp single
                for (size_t offset = tail_start; offset < row_count; offset++) {
                    bv_set(&qualifies, offset, true);
                }
            } else {
                __m512 low = _mm512_set1_ps(predicate->low);
                __m512 high = _mm512_set1_ps(predicate->high);

                #pragma omp for schedule(static, WORDS_PER_CHUNK)
                for (size_t word = 0; word < full_words; word++) {
                    uint64_t word_bits = 0;
                    for (size_t lane_group = 0; lane_group < 4; lane_group++) {
                        __m512 values = _mm512_loadu_ps(column + word * 64 + lane_group * LANES);
                        word_bits |= (uint64_t)range_mask(values, low, high) << (lane_group * LANES);
                    }
                    qualifies.bits[word] = word_bits;
                }

                #pragma omp single
                for (size_t offset = tail_start; offset < row_count; offset++) {
                    float value = column[offset];
                    bv_set(&qualifies, offset, predicate->low <= value && value <= predicate->high);
                }
            }

            // -----------------------------------------------------------------------------------------
            // Refines the bitmap with the remaining predicates, skipping covered attributes
            // -----------------------------------------------------------------------------------------
            /// All threads see the same skip flags, so they all take the same `continue`
            for (size_t predicate_id = 1; predicate_id < predicate_count; predicate_id++) {
                predicate = &predicates[predicate_id];

                if (column_is_covered[predicate->column]) {
                    continue;
                }

                column = columns[predicate->column] + first_row;
                __m512 low = _mm512_set1_ps(predicate->low);
                __m512 high = _mm512_set1_ps(predicate->high);

                #pragma omp for schedule(static, WORDS_PER_CHUNK)
                for (size_t word = 0; word < full_words; word++) {
                    uint64_t word_bits = qualifies.bits[word];
                    if (word_bits == 0) {
                        continue;   /// No row left in these 64: nothing to test
                    }
                    uint64_t new_bits = 0;
                    for (size_t lane_group = 0; lane_group < 4; lane_group++) {
                        __mmask16 active = (__mmask16)(word_bits >> (lane_group * LANES));
                        if (active == 0) continue;
                        /// Masked load: inactive lanes are not read
                        __m512 values = _mm512_maskz_loadu_ps(active, column + word * 64 + lane_group * LANES);
                        new_bits |= (uint64_t)range_mask_active(active, values, low, high) << (lane_group * LANES);
                    }
                    qualifies.bits[word] = new_bits;
                }

                #pragma omp single
                for (size_t offset = tail_start; offset < row_count; offset++) {
                    if (bv_get(&qualifies, offset)) {
                        float value = column[offset];
                        bv_set(&qualifies, offset, predicate->low <= value && value <= predicate->high);
                    }
                }
            }

            // -----------------------------------------------------------------------------------------
            // Aggregates the qualifying rows (each thread its own words, then reduction)
            // -----------------------------------------------------------------------------------------
            /// Floats are widened to doubles so the sum keeps the scalar precision
            const float *first_column = columns[0] + first_row;
            __m512d accumulator = _mm512_setzero_pd();

            #pragma omp for schedule(static, WORDS_PER_CHUNK) nowait
            for (size_t word = 0; word < full_words; word++) {
                uint64_t word_bits = qualifies.bits[word];
                if (word_bits == 0) continue;
                partition_count_rows += (size_t)__builtin_popcountll(word_bits);
                for (size_t lane_group = 0; lane_group < 4; lane_group++) {
                    __mmask16 active = (__mmask16)(word_bits >> (lane_group * LANES));
                    if (active == 0) continue;
                    __m512 values = _mm512_maskz_loadu_ps(active, first_column + word * 64 + lane_group * LANES);
                    accumulator = add_floats_to_double_acc(accumulator, active, values);
                }
            }
            partition_sum += _mm512_reduce_add_pd(accumulator);

            #pragma omp single nowait
            for (size_t offset = tail_start; offset < row_count; offset++) {
                if (bv_get(&qualifies, offset)) {
                    partition_sum += first_column[offset];
                    partition_count_rows++;
                }
            }
        }

        sum_of_first_column += partition_sum;
        qualifying_count += partition_count_rows;
        bv_free(&qualifies);
    }

    if (qualifying_count_out) *qualifying_count_out = qualifying_count;
    return sum_of_first_column;
}
