// =========================================================================================
// Scan: BITMAP + RE-EVALUATION
// -----------------------------------------------------------------------------------------
// Based on FullScan::scan_partition (include/indexes/full_scan.hpp).
//
// For each partition, one bit per row records whether the row still qualifies.
// Every predicate is evaluated on every partition, even when the index already knows
// the partition satisfies it (the skip matrix is accepted but ignored).
// =========================================================================================
#include "scan_common.h"

double scan_bitmap_reeval(float **columns, size_t column_count,
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

        // -----------------------------------------------------------------------------------------
        // Initializes the bitmap by evaluating the first predicate on every row
        // -----------------------------------------------------------------------------------------
        const Predicate *predicate = &predicates[0];
        const float *column = columns[predicate->column];

        for (size_t offset = 0; offset < row_count; offset++) {
            float value = column[first_row + offset];
            bv_set(&qualifies, offset, predicate->low <= value && value <= predicate->high);
        }

        // -----------------------------------------------------------------------------------------
        // Refines the bitmap with the remaining predicates
        // -----------------------------------------------------------------------------------------
        /// Only rows that still qualify are tested; a failing test clears their bit
        for (size_t predicate_id = 1; predicate_id < predicate_count; predicate_id++) {
            predicate = &predicates[predicate_id];
            column = columns[predicate->column];

            for (size_t offset = 0; offset < row_count; offset++) {
                if (bv_get(&qualifies, offset)) {
                    float value = column[first_row + offset];
                    bv_set(&qualifies, offset, predicate->low <= value && value <= predicate->high);
                }
            }
        }

        // -----------------------------------------------------------------------------------------
        // Aggregates the qualifying rows (sum of column 0 and count)
        // -----------------------------------------------------------------------------------------
        for (size_t offset = 0; offset < row_count; offset++) {
            if (bv_get(&qualifies, offset)) {
                sum_of_first_column += columns[0][first_row + offset];
                qualifying_count++;
            }
        }

        bv_free(&qualifies);
    }

    if (qualifying_count_out) *qualifying_count_out = qualifying_count;
    return sum_of_first_column;
}
