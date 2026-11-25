// =========================================================================================
// Scan: SELECTION VECTOR + RE-EVALUATION (AVX-512)
// -----------------------------------------------------------------------------------------
// SIMD version of Scalar/scan_selvec_reeval.c.
//
// - First predicate: 16 contiguous rows are compared at once and the ids of the matching
//   rows are appended with a COMPRESS STORE (vpcompressd).
// - Remaining predicates: 16 row ids are read from the selection vector, their values are
//   fetched with a GATHER, compared, and the surviving ids are compress-stored back in
//   place (the write position never passes the read position).
// - Aggregation: values of column 0 are gathered and summed in double precision.
// Elements that do not fill a whole register are handled scalarly.
//
// Row ids are 32-bit signed indices for the gather: rows must be < 2^31.
// =========================================================================================
#include "scan_common.h"
#include "simd_common.h"

double scan_selvec_reeval_simd(float **columns, size_t column_count,
                               const Predicate *predicates, size_t predicate_count,
                               const Partition *partitions, const bool *skip,
                               size_t partition_count, size_t *qualifying_count_out)
{
    (void)column_count;
    (void)skip;   // This variant never skips: all predicates are always re-evaluated

    const __m512i lane_index = _mm512_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7,
                                                 8, 9, 10, 11, 12, 13, 14, 15);
    double sum_of_first_column = 0.0;
    size_t qualifying_count = 0;

    for (size_t partition_id = 0; partition_id < partition_count; partition_id++) {

        // -----------------------------------------------------------------------------------------
        // Gets the row range of the partition and creates the selection vector
        // -----------------------------------------------------------------------------------------
        /// Capacity for every row (+1 for the branch-free write), so it never needs to grow
        size_t first_row = partitions[partition_id].low;
        size_t end_row = partitions[partition_id].high;
        SelVec selected = sv_create(end_row - first_row + 1);

        // -----------------------------------------------------------------------------------------
        // Fills the selection vector with the rows that pass the first predicate
        // -----------------------------------------------------------------------------------------
        const Predicate *predicate = &predicates[0];
        const float *column = columns[predicate->column];
        __m512 low = _mm512_set1_ps(predicate->low);
        __m512 high = _mm512_set1_ps(predicate->high);

        size_t row = first_row;
        for (; row + LANES <= end_row; row += LANES) {
            __m512 values = _mm512_loadu_ps(column + row);
            __mmask16 matches = range_mask(values, low, high);
            __m512i row_ids = _mm512_add_epi32(_mm512_set1_epi32((int)row), lane_index);
            /// Writes only the matching ids, packed contiguously
            _mm512_mask_compressstoreu_epi32(selected.data + selected.size, matches, row_ids);
            selected.size += (size_t)__builtin_popcount(matches);
        }
        for (; row < end_row; row++) {
            float value = column[row];
            sv_maybe_push_back(&selected, (uint32_t)row, predicate->low <= value && value <= predicate->high);
        }

        // -----------------------------------------------------------------------------------------
        // Filters the selection vector in place with the remaining predicates
        // -----------------------------------------------------------------------------------------
        for (size_t predicate_id = 1; predicate_id < predicate_count && selected.size > 0; predicate_id++) {
            predicate = &predicates[predicate_id];
            column = columns[predicate->column];
            low = _mm512_set1_ps(predicate->low);
            high = _mm512_set1_ps(predicate->high);

            size_t write_position = 0;
            size_t read_position = 0;
            for (; read_position + LANES <= selected.size; read_position += LANES) {
                __m512i row_ids = _mm512_loadu_si512(selected.data + read_position);
                __m512 values = _mm512_i32gather_ps(row_ids, column, 4);
                __mmask16 matches = range_mask(values, low, high);
                _mm512_mask_compressstoreu_epi32(selected.data + write_position, matches, row_ids);
                write_position += (size_t)__builtin_popcount(matches);
            }
            for (; read_position < selected.size; read_position++) {
                uint32_t row_id = selected.data[read_position];
                float value = column[row_id];
                selected.data[write_position] = row_id;
                write_position += predicate->low <= value && value <= predicate->high;
            }
            selected.size = write_position;
        }

        // -----------------------------------------------------------------------------------------
        // Aggregates the qualifying rows (sum of column 0 and count)
        // -----------------------------------------------------------------------------------------
        __m512d accumulator = _mm512_setzero_pd();
        size_t i = 0;
        for (; i + LANES <= selected.size; i += LANES) {
            __m512i row_ids = _mm512_loadu_si512(selected.data + i);
            __m512 values = _mm512_i32gather_ps(row_ids, columns[0], 4);
            accumulator = add_floats_to_double_acc(accumulator, 0xFFFF, values);
        }
        sum_of_first_column += _mm512_reduce_add_pd(accumulator);
        for (; i < selected.size; i++) {
            sum_of_first_column += columns[0][selected.data[i]];
        }
        qualifying_count += selected.size;

        sv_free(&selected);
    }

    if (qualifying_count_out) *qualifying_count_out = qualifying_count;
    return sum_of_first_column;
}
