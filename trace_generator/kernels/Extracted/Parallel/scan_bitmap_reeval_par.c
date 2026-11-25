// =========================================================================================
// Scan: BITMAP + RE-EVALUATION (OpenMP parallel)
// -----------------------------------------------------------------------------------------
// Parallel version of Scalar/scan_bitmap_reeval.c.
//
// Partitions are still visited one after the other; the loops over the rows (offsets)
// of each partition are split among the threads.
//
// Bitmap race avoidance: bv_set does a read-modify-write on a 64-bit word, so two
// threads must never write bits of the same word. The offset loops use a static schedule
// with a chunk size that is a multiple of 64, so each thread owns whole words.
// =========================================================================================
#include <omp.h>
#include "scan_common.h"

/// Rows per chunk: multiple of 64 so that chunks never share a bitmap word
#define BITMAP_CHUNK (64 * 64)

double scan_bitmap_reeval_par(float **columns, size_t column_count,
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
        BitVector qualifies = bv_create(row_count);

        double partition_sum = 0.0;
        size_t partition_count_rows = 0;

        #pragma omp parallel
        {
            // -----------------------------------------------------------------------------------------
            // Initializes the bitmap by evaluating the first predicate on every row
            // -----------------------------------------------------------------------------------------
            const Predicate *first_predicate = &predicates[0];
            const float *first_column = columns[first_predicate->column];

            #pragma omp for schedule(static, BITMAP_CHUNK)
            for (size_t offset = 0; offset < row_count; offset++) {
                float value = first_column[first_row + offset];
                bv_set(&qualifies, offset, first_predicate->low <= value && value <= first_predicate->high);
            }
            /// Implicit barrier: the bitmap is complete before refining

            // -----------------------------------------------------------------------------------------
            // Refines the bitmap with the remaining predicates
            // -----------------------------------------------------------------------------------------
            for (size_t predicate_id = 1; predicate_id < predicate_count; predicate_id++) {
                const Predicate *predicate = &predicates[predicate_id];
                const float *column = columns[predicate->column];

                #pragma omp for schedule(static, BITMAP_CHUNK)
                for (size_t offset = 0; offset < row_count; offset++) {
                    if (bv_get(&qualifies, offset)) {
                        float value = column[first_row + offset];
                        bv_set(&qualifies, offset, predicate->low <= value && value <= predicate->high);
                    }
                }
            }

            // -----------------------------------------------------------------------------------------
            // Aggregates the qualifying rows (sum of column 0 and count) with a reduction
            // -----------------------------------------------------------------------------------------
            #pragma omp for schedule(static, BITMAP_CHUNK) reduction(+ : partition_sum, partition_count_rows)
            for (size_t offset = 0; offset < row_count; offset++) {
                if (bv_get(&qualifies, offset)) {
                    partition_sum += columns[0][first_row + offset];
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
