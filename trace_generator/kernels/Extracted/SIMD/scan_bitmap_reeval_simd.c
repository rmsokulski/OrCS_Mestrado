// =========================================================================================
// Scan: BITMAP + RE-EVALUATION (AVX-512)
// -----------------------------------------------------------------------------------------
// SIMD version of Scalar/scan_bitmap_reeval.c.
//
// One AVX-512 compare tests 16 rows and yields a 16-bit mask. Four masks fill one
// 64-bit bitmap word, which is written at once (no per-bit read-modify-write).
// When refining, a word that is already 0 is skipped entirely, and only the lanes still
// set are loaded and compared. Rows that do not fill a whole word are handled scalarly.
// =========================================================================================
#include "scan_common.h"
#include "simd_common.h"

double scan_bitmap_reeval_simd(float **columns, size_t column_count,
                               const Predicate *predicates, size_t predicate_count,
                               const Partition *partitions, const bool *skip,
                               size_t partition_count, size_t *qualifying_count_out)
{
    (void)column_count;
    (void)skip;   // This variant never skips: all predicates are always re-evaluated

    double sum_of_first_column = 0.0;
    size_t qualifying_count = 0;

    for (size_t partition_id = 0; partition_id < partition_count; partition_id++) {

        // -----------------------------------------------------------------------------------------
        // Gets the row range of the partition and allocates one bit per row
        // -----------------------------------------------------------------------------------------
        size_t first_row = partitions[partition_id].low;
        size_t row_count = partitions[partition_id].high - first_row;
        size_t full_words = row_count / 64;          /// Words handled with SIMD
        size_t tail_start = full_words * 64;         /// First offset handled scalarly
        BitVector qualifies = bv_create(row_count);

        // -----------------------------------------------------------------------------------------
        // Initializes the bitmap by evaluating the first predicate on every row
        // -----------------------------------------------------------------------------------------
        const Predicate *predicate = &predicates[0];
        const float *column = columns[predicate->column] + first_row;
        __m512 low = _mm512_set1_ps(predicate->low);
        __m512 high = _mm512_set1_ps(predicate->high);

        for (size_t word = 0; word < full_words; word++) {
            uint64_t word_bits = 0;
            for (size_t lane_group = 0; lane_group < 4; lane_group++) {
                __m512 values = _mm512_loadu_ps(column + word * 64 + lane_group * LANES);
                word_bits |= (uint64_t)range_mask(values, low, high) << (lane_group * LANES);
            }
            qualifies.bits[word] = word_bits;
        }
        for (size_t offset = tail_start; offset < row_count; offset++) {
            float value = column[offset];
            bv_set(&qualifies, offset, predicate->low <= value && value <= predicate->high);
        }

        // -----------------------------------------------------------------------------------------
        // Refines the bitmap with the remaining predicates
        // -----------------------------------------------------------------------------------------
        for (size_t predicate_id = 1; predicate_id < predicate_count; predicate_id++) {
            predicate = &predicates[predicate_id];
            column = columns[predicate->column] + first_row;
            low = _mm512_set1_ps(predicate->low);
            high = _mm512_set1_ps(predicate->high);

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
            for (size_t offset = tail_start; offset < row_count; offset++) {
                if (bv_get(&qualifies, offset)) {
                    float value = column[offset];
                    bv_set(&qualifies, offset, predicate->low <= value && value <= predicate->high);
                }
            }
        }

        // -----------------------------------------------------------------------------------------
        // Aggregates the qualifying rows (sum of column 0 and count)
        // -----------------------------------------------------------------------------------------
        /// Floats are widened to doubles so the sum keeps the scalar precision
        const float *first_column = columns[0] + first_row;
        __m512d accumulator = _mm512_setzero_pd();

        for (size_t word = 0; word < full_words; word++) {
            uint64_t word_bits = qualifies.bits[word];
            if (word_bits == 0) continue;
            qualifying_count += (size_t)__builtin_popcountll(word_bits);
            for (size_t lane_group = 0; lane_group < 4; lane_group++) {
                __mmask16 active = (__mmask16)(word_bits >> (lane_group * LANES));
                if (active == 0) continue;
                __m512 values = _mm512_maskz_loadu_ps(active, first_column + word * 64 + lane_group * LANES);
                accumulator = add_floats_to_double_acc(accumulator, active, values);
            }
        }
        sum_of_first_column += _mm512_reduce_add_pd(accumulator);

        for (size_t offset = tail_start; offset < row_count; offset++) {
            if (bv_get(&qualifies, offset)) {
                sum_of_first_column += first_column[offset];
                qualifying_count++;
            }
        }

        bv_free(&qualifies);
    }

    if (qualifying_count_out) *qualifying_count_out = qualifying_count;
    return sum_of_first_column;
}
