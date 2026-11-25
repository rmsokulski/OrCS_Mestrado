// =========================================================================================
// Scan: SELECTION VECTOR + RE-EVALUATION (OpenMP parallel)
// -----------------------------------------------------------------------------------------
// Parallel version of Scalar/scan_selvec_reeval.c.
//
// Partitions are still visited one after the other; the rows (offsets) of each partition
// are split among the threads. A single shared selection vector cannot be appended to in
// parallel, so each thread keeps a PRIVATE selection vector holding only the rows of its
// own chunk. Since every predicate and the aggregation work row by row, the private
// vectors never need to be merged: each thread filters and aggregates its own rows and
// the totals are combined with a reduction.
// =========================================================================================
#include <omp.h>
#include "scan_common.h"

double scan_selvec_reeval_par(float **columns, size_t column_count,
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
        // Gets the row range of the partition
        // -----------------------------------------------------------------------------------------
        size_t first_row = partitions[partition_id].low;
        size_t end_row = partitions[partition_id].high;

        double partition_sum = 0.0;
        size_t partition_count_rows = 0;

        #pragma omp parallel reduction(+ : partition_sum, partition_count_rows)
        {
            // -----------------------------------------------------------------------------------------
            // Creates the private selection vector of this thread
            // -----------------------------------------------------------------------------------------
            SelVec selected = sv_create(0);

            // -----------------------------------------------------------------------------------------
            // Fills it with the rows of this thread's chunk that pass the first predicate
            // -----------------------------------------------------------------------------------------
            /// nowait: no thread depends on the rows of another thread
            const Predicate *predicate = &predicates[0];
            const float *column = columns[predicate->column];

            #pragma omp for schedule(static) nowait
            for (size_t row = first_row; row < end_row; row++) {
                float value = column[row];
                sv_maybe_push_back(&selected, (uint32_t)row, predicate->low <= value && value <= predicate->high);
            }

            // -----------------------------------------------------------------------------------------
            // Filters the private selection vector in place with the remaining predicates
            // -----------------------------------------------------------------------------------------
            for (size_t predicate_id = 1; predicate_id < predicate_count && selected.size > 0; predicate_id++) {
                predicate = &predicates[predicate_id];
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
            // Aggregates this thread's qualifying rows (combined by the reduction)
            // -----------------------------------------------------------------------------------------
            for (size_t i = 0; i < selected.size; i++) {
                partition_sum += columns[0][selected.data[i]];
            }
            partition_count_rows += selected.size;

            sv_free(&selected);
        }

        sum_of_first_column += partition_sum;
        qualifying_count += partition_count_rows;
    }

    if (qualifying_count_out) *qualifying_count_out = qualifying_count;
    return sum_of_first_column;
}
