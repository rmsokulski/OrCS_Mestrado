// =========================================================================================
// Scan: BITMAP + RE-EVALUATION (VIMA, current ISA)
// -----------------------------------------------------------------------------------------
// VIMA version of SIMD/scan_bitmap_reeval_simd.c.
//
// The current VIMA ISA has no bit masks, so the "bitmap" is a 0/1 float vector per
// 2048-row block (8 KB instead of 256 B). Predicate by predicate, over whole blocks:
//   passes = range test (4 instructions);  qualifies = qualifies x passes (fmuls)
// The first evaluated predicate writes qualifies directly.
// Every predicate is evaluated on every row (the skip file is ignored).
// Aggregation: count = fcums(qualifies); sum = fcums(column 0 x qualifies).
// There is no per-block early exit: every instruction processes all 2048 rows.
// Rows after the last whole block of a partition are handled scalarly.
// =========================================================================================
#include "vima_common.h"

double scan_bitmap_reeval_vima(float **columns, size_t column_count,
                               const Predicate *predicates, size_t predicate_count,
                               const Partition *partitions, const bool *skip,
                               size_t partition_count, size_t *qualifying_count_out)
{
    (void)column_count;
    (void)skip;   // This variant never skips: all predicates are always re-evaluated
    VimaScratch scratch = vima_scratch_create();
    double sum_of_first_column = 0.0;
    size_t qualifying_count = 0;

    for (size_t partition_id = 0; partition_id < partition_count; partition_id++) {

        size_t first_row = partitions[partition_id].low;
        size_t row_count = partitions[partition_id].high - first_row;
        size_t block_count = row_count / VIMA_LANES;         /// Blocks handled by VIMA
        size_t tail_start = block_count * VIMA_LANES;        /// First offset handled scalarly

        float *qualifies = vima_vector_alloc(block_count);

        // -----------------------------------------------------------------------------------------
        // Builds the 0/1 mask of the whole blocks, one predicate at a time
        // -----------------------------------------------------------------------------------------
        size_t evaluated = 0;
        for (size_t predicate_id = 0; predicate_id < predicate_count; predicate_id++) {
            const Predicate *predicate = &predicates[predicate_id];

            const float *column = columns[predicate->column] + first_row;
            vima_set_bounds(&scratch, predicate);

            for (size_t block = 0; block < block_count; block++) {
                float *block_mask = qualifies + block * VIMA_LANES;
                if (evaluated == 0) {
                    vima_range(&scratch, column + block * VIMA_LANES, block_mask);
                } else {
                    vima_range(&scratch, column + block * VIMA_LANES, scratch.passes);
                    vima_fmuls(block_mask, scratch.passes, block_mask);
                }
            }
            evaluated++;
        }

        // -----------------------------------------------------------------------------------------
        // Aggregates the whole blocks (count and sum of column 0)
        // -----------------------------------------------------------------------------------------
        const float *first_column = columns[0] + first_row;
        for (size_t block = 0; block < block_count; block++) {
            const float *values = first_column + block * VIMA_LANES;
            if (evaluated == 0) {
                /// No predicate evaluated: every row qualifies
                qualifying_count += VIMA_LANES;
                sum_of_first_column += vima_sum(&scratch, values);
                continue;
            }
            float *block_mask = qualifies + block * VIMA_LANES;
            qualifying_count += (size_t)vima_sum(&scratch, block_mask);
            vima_fmuls(values, block_mask, scratch.values);
            sum_of_first_column += vima_sum(&scratch, scratch.values);
        }

        // -----------------------------------------------------------------------------------------
        // Rows after the last whole block, scalarly
        // -----------------------------------------------------------------------------------------
        for (size_t offset = tail_start; offset < row_count; offset++) {
            bool row_qualifies = true;
            for (size_t predicate_id = 0; predicate_id < predicate_count && row_qualifies; predicate_id++) {
                const Predicate *predicate = &predicates[predicate_id];

                row_qualifies = range_passes(predicate, columns[predicate->column][first_row + offset]);
            }
            if (row_qualifies) {
                sum_of_first_column += first_column[offset];
                qualifying_count++;
            }
        }

        free(qualifies);
    }

    vima_scratch_free(&scratch);
    if (qualifying_count_out) *qualifying_count_out = qualifying_count;
    return sum_of_first_column;
}
