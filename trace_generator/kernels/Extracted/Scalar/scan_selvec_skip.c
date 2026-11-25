// =========================================================================================
// Scan: SELECTION VECTOR + SKIP OF COVERED ATTRIBUTES
// -----------------------------------------------------------------------------------------
// Direct extraction of FullScanCandidateList::scan_partition
// (include/indexes/full_scan_candidate_list.hpp), the scan used by all adaptive indexes.
//
// For each partition, a list of row ids holds the rows that still qualify.
// Predicates on attributes the index already guarantees for the whole partition
// (skip[partition][column] == true) are not evaluated. Filtering is branch-free.
// =========================================================================================
#include "scan_common.h"

double scan_selvec_skip(float **columns, size_t column_count,
                        const Predicate *predicates, size_t predicate_count,
                        const Partition *partitions, const bool *skip,
                        size_t partition_count, size_t *qualifying_count_out)
{
    double sum_of_first_column = 0.0;
    size_t qualifying_count = 0;

    for (size_t partition_id = 0; partition_id < partition_count; partition_id++) {

        // -----------------------------------------------------------------------------------------
        // Gets the row range, the skip flags of the partition and creates an empty selection vector
        // -----------------------------------------------------------------------------------------
        size_t first_row = partitions[partition_id].low;
        size_t end_row = partitions[partition_id].high;
        const bool *column_is_covered = &skip[partition_id * column_count];
        SelVec selected = sv_create(0);

        // -----------------------------------------------------------------------------------------
        // Fills the selection vector with the first predicate
        // -----------------------------------------------------------------------------------------
        const Predicate *predicate = &predicates[0];
        const float *column = columns[predicate->column];

        if (column_is_covered[predicate->column]) {
            /// The index guarantees every row passes: select all rows without comparing
            for (size_t row = first_row; row < end_row; row++) {
                sv_push_back(&selected, (uint32_t)row);
            }
        } else {
            for (size_t row = first_row; row < end_row; row++) {
                float value = column[row];
                sv_maybe_push_back(&selected, (uint32_t)row, predicate->low <= value && value <= predicate->high);
            }
        }

        // -----------------------------------------------------------------------------------------
        // Filters the selection vector in place, skipping covered attributes
        // -----------------------------------------------------------------------------------------
        /// Stops early when no row is left to test
        for (size_t predicate_id = 1; predicate_id < predicate_count && selected.size > 0; predicate_id++) {
            predicate = &predicates[predicate_id];

            if (column_is_covered[predicate->column]) {
                continue;
            }

            column = columns[predicate->column];
            size_t write_position = 0;
            for (size_t read_position = 0; read_position < selected.size; read_position++) {
                uint32_t row = selected.data[read_position];
                float value = column[row];
                selected.data[write_position] = row;
                write_position += predicate->low <= value && value <= predicate->high;
            }
            selected.size = write_position;
        }

        // -----------------------------------------------------------------------------------------
        // Aggregates the qualifying rows (sum of column 0 and count)
        // -----------------------------------------------------------------------------------------
        for (size_t i = 0; i < selected.size; i++) {
            sum_of_first_column += columns[0][selected.data[i]];
        }
        qualifying_count += selected.size;

        sv_free(&selected);
    }

    if (qualifying_count_out) *qualifying_count_out = qualifying_count;
    return sum_of_first_column;
}
