// =========================================================================================
// Scan: SELECTION VECTOR + SKIP OF COVERED ATTRIBUTES (AVX-512 + OpenMP)
// -----------------------------------------------------------------------------------------
// Parallel version of SIMD/scan_selvec_skip_simd.c, following Parallel/scan_selvec_*.
//
// Partitions are visited one after the other. Inside a partition, the blocks of 16 rows
// are split among the threads. Each thread keeps a PRIVATE selection vector with the ids
// of its own blocks, filters and aggregates it with SIMD; totals are combined with a
// reduction. Predicates on attributes the index already guarantees are not evaluated.
// The last rows that do not fill a whole block go to one thread.
//
// Row ids are 32-bit signed indices for the gather: rows must be < 2^31.
// =========================================================================================
#include <omp.h>
#include "scan_common.h"
#include "simd_common.h"

double scan_selvec_skip_simd_par(float **columns, size_t column_count,
                                 const Predicate *predicates, size_t predicate_count,
                                 const Partition *partitions, const bool *skip,
                                 size_t partition_count, size_t *qualifying_count_out)
{
    const __m512i lane_index = _mm512_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7,
                                                 8, 9, 10, 11, 12, 13, 14, 15);
    double sum_of_first_column = 0.0;
    size_t qualifying_count = 0;

    for (size_t partition_id = 0; partition_id < partition_count; partition_id++) {

        // -----------------------------------------------------------------------------------------
        // Gets the row range, the skip flags and the number of full 16-row blocks
        // -----------------------------------------------------------------------------------------
        size_t first_row = partitions[partition_id].low;
        size_t end_row = partitions[partition_id].high;
        size_t full_blocks = (end_row - first_row) / LANES;   /// Blocks handled with SIMD (in parallel)
        size_t tail_start = first_row + full_blocks * LANES;  /// First row handled scalarly
        const bool *column_is_covered = &skip[partition_id * column_count];

        double partition_sum = 0.0;
        size_t partition_count_rows = 0;

        #pragma omp parallel reduction(+ : partition_sum, partition_count_rows)
        {
            // -----------------------------------------------------------------------------------------
            // Creates the private selection vector of this thread
            // -----------------------------------------------------------------------------------------
            SelVec selected = sv_create(0);

            // -----------------------------------------------------------------------------------------
            // Fills it with this thread's blocks using the first predicate
            // -----------------------------------------------------------------------------------------
            const Predicate *predicate = &predicates[0];
            const float *column = columns[predicate->column];

            if (column_is_covered[predicate->column]) {
                /// The index guarantees every row passes: store all ids without comparing
                #pragma omp for schedule(static) nowait
                for (size_t block = 0; block < full_blocks; block++) {
                    size_t row = first_row + block * LANES;
                    __m512i row_ids = _mm512_add_epi32(_mm512_set1_epi32((int)row), lane_index);
                    sv_reserve(&selected, LANES);
                    _mm512_storeu_si512(selected.data + selected.size, row_ids);
                    selected.size += LANES;
                }

                #pragma omp single nowait
                for (size_t row = tail_start; row < end_row; row++) {
                    sv_push_back(&selected, (uint32_t)row);
                }
            } else {
                __m512 low = _mm512_set1_ps(predicate->low);
                __m512 high = _mm512_set1_ps(predicate->high);

                #pragma omp for schedule(static) nowait
                for (size_t block = 0; block < full_blocks; block++) {
                    size_t row = first_row + block * LANES;
                    __m512 values = _mm512_loadu_ps(column + row);
                    __mmask16 matches = range_mask(values, low, high);
                    __m512i row_ids = _mm512_add_epi32(_mm512_set1_epi32((int)row), lane_index);
                    /// Private vector grows on demand; a compress store may write up to 16 ids
                    sv_reserve(&selected, LANES);
                    _mm512_mask_compressstoreu_epi32(selected.data + selected.size, matches, row_ids);
                    selected.size += (size_t)__builtin_popcount(matches);
                }

                #pragma omp single nowait
                for (size_t row = tail_start; row < end_row; row++) {
                    float value = column[row];
                    sv_maybe_push_back(&selected, (uint32_t)row, predicate->low <= value && value <= predicate->high);
                }
            }

            // -----------------------------------------------------------------------------------------
            // Filters the private selection vector in place, skipping covered attributes
            // -----------------------------------------------------------------------------------------
            for (size_t predicate_id = 1; predicate_id < predicate_count && selected.size > 0; predicate_id++) {
                predicate = &predicates[predicate_id];

                if (column_is_covered[predicate->column]) {
                    continue;
                }

                column = columns[predicate->column];
                __m512 low = _mm512_set1_ps(predicate->low);
                __m512 high = _mm512_set1_ps(predicate->high);

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
            // Aggregates this thread's qualifying rows (combined by the reduction)
            // -----------------------------------------------------------------------------------------
            __m512d accumulator = _mm512_setzero_pd();
            size_t i = 0;
            for (; i + LANES <= selected.size; i += LANES) {
                __m512i row_ids = _mm512_loadu_si512(selected.data + i);
                __m512 values = _mm512_i32gather_ps(row_ids, columns[0], 4);
                accumulator = add_floats_to_double_acc(accumulator, 0xFFFF, values);
            }
            partition_sum += _mm512_reduce_add_pd(accumulator);
            for (; i < selected.size; i++) {
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
