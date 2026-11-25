// =========================================================================================
// Scan: SELECTION VECTOR + SKIP OF COVERED ATTRIBUTES (VIMA, current ISA)
// -----------------------------------------------------------------------------------------
// VIMA version of SIMD/scan_selvec_skip_simd.c.
//
// The current VIMA ISA has no compress instruction, so this is a VIMA + CPU hybrid:
// - First predicate: VIMA computes the 0/1 vector of each 2048-row block (range test);
//   the CPU reads it and appends the matching row ids (the work of a compress).
// - Remaining predicates: VIMA gathers 2048 selected values (fgtrs) and tests them; the
//   CPU compacts the surviving ids in place.
// - Aggregation: VIMA gathers column 0 and sums it (fcums); count = selection size.
// Covered attributes (skip flag 1) issue no VIMA instructions and need no compaction.
// Rows and ids that do not fill a whole 2048-element vector are handled scalarly.
// =========================================================================================
#include "vima_common.h"

double scan_selvec_skip_vima(float **columns, size_t column_count,
                             const Predicate *predicates, size_t predicate_count,
                             const Partition *partitions, const bool *skip,
                             size_t partition_count, size_t *qualifying_count_out)
{
    VimaScratch scratch = vima_scratch_create();
    double sum_of_first_column = 0.0;
    size_t qualifying_count = 0;

    for (size_t partition_id = 0; partition_id < partition_count; partition_id++) {

        /// Capacity for every row (+1 for the branch-free write), so it never needs to grow
        size_t first_row = partitions[partition_id].low;
        size_t end_row = partitions[partition_id].high;
        const bool *column_is_covered = &skip[partition_id * column_count];
        SelVec selected = sv_create(end_row - first_row + 1);

        // -----------------------------------------------------------------------------------------
        // Fills the selection vector with the first predicate
        // -----------------------------------------------------------------------------------------
        const Predicate *predicate = &predicates[0];
        const float *column = columns[predicate->column];
        size_t row = first_row;

        if (column_is_covered[predicate->column]) {
            /// The index guarantees every row passes: store all ids without comparing
            for (; row < end_row; row++) sv_push_back(&selected, (uint32_t)row);
        } else {
            vima_set_bounds(&scratch, predicate);
            for (; row + VIMA_LANES <= end_row; row += VIMA_LANES) {
                vima_range(&scratch, column + row, scratch.passes);
                vima_fence();   /// The CPU reads the VIMA result next
                for (size_t lane = 0; lane < VIMA_LANES; lane++) {   /// CPU compaction
                    sv_maybe_push_back(&selected, (uint32_t)(row + lane), scratch.passes[lane] != 0.0f);
                }
            }
            for (; row < end_row; row++) {
                sv_maybe_push_back(&selected, (uint32_t)row, range_passes(predicate, column[row]));
            }
        }

        // -----------------------------------------------------------------------------------------
        // Filters the selection vector in place
        // -----------------------------------------------------------------------------------------
        for (size_t predicate_id = 1; predicate_id < predicate_count && selected.size > 0; predicate_id++) {
            predicate = &predicates[predicate_id];
            if (column_is_covered[predicate->column]) continue;
            column = columns[predicate->column];
            vima_set_bounds(&scratch, predicate);

            size_t write_position = 0;
            size_t read_position = 0;
            for (; read_position + VIMA_LANES <= selected.size; read_position += VIMA_LANES) {
                vima_fgtrs(column, selected.data + read_position, scratch.values);
                vima_range(&scratch, scratch.values, scratch.passes);
                vima_fence();   /// The CPU reads the VIMA result next
                for (size_t lane = 0; lane < VIMA_LANES; lane++) {   /// CPU compaction
                    selected.data[write_position] = selected.data[read_position + lane];
                    write_position += scratch.passes[lane] != 0.0f;
                }
            }
            for (; read_position < selected.size; read_position++) {
                uint32_t row_id = selected.data[read_position];
                selected.data[write_position] = row_id;
                write_position += range_passes(predicate, column[row_id]);
            }
            selected.size = write_position;
        }

        // -----------------------------------------------------------------------------------------
        // Aggregates the qualifying rows (sum of column 0 and count)
        // -----------------------------------------------------------------------------------------
        size_t i = 0;
        for (; i + VIMA_LANES <= selected.size; i += VIMA_LANES) {
            vima_fgtrs(columns[0], selected.data + i, scratch.values);
            sum_of_first_column += vima_sum(&scratch, scratch.values);
        }
        for (; i < selected.size; i++) {
            sum_of_first_column += columns[0][selected.data[i]];
        }
        qualifying_count += selected.size;

        sv_free(&selected);
    }

    vima_scratch_free(&scratch);
    if (qualifying_count_out) *qualifying_count_out = qualifying_count;
    return sum_of_first_column;
}
