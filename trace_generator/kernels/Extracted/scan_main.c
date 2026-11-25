// =========================================================================================
// scan_main: runs one of the 16 scan versions over input files
// -----------------------------------------------------------------------------------------
// Usage:
//   scan_main <version> <data_file> <predicates_file> <partitions_file> <skip_file> <output_file>
//   scan_main list
//
// See README.md for the file formats.
// =========================================================================================
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "Scalar/scan_common.h"
#include "orcs_tracing_c.h"

// -----------------------------------------------------------------------------------------
// Prototypes of every version (all share the scan_fn signature)
// -----------------------------------------------------------------------------------------
#define SCAN_ARGS float **, size_t, const Predicate *, size_t, const Partition *, const bool *, size_t, size_t *
double scan_bitmap_reeval(SCAN_ARGS);          double scan_bitmap_skip(SCAN_ARGS);
double scan_selvec_reeval(SCAN_ARGS);          double scan_selvec_skip(SCAN_ARGS);
double scan_bitmap_reeval_par(SCAN_ARGS);      double scan_bitmap_skip_par(SCAN_ARGS);
double scan_selvec_reeval_par(SCAN_ARGS);      double scan_selvec_skip_par(SCAN_ARGS);
double scan_bitmap_reeval_simd(SCAN_ARGS);     double scan_bitmap_skip_simd(SCAN_ARGS);
double scan_selvec_reeval_simd(SCAN_ARGS);     double scan_selvec_skip_simd(SCAN_ARGS);
double scan_bitmap_reeval_simd_par(SCAN_ARGS); double scan_bitmap_skip_simd_par(SCAN_ARGS);
double scan_selvec_reeval_simd_par(SCAN_ARGS); double scan_selvec_skip_simd_par(SCAN_ARGS);
double scan_bitmap_reeval_vima(SCAN_ARGS);     double scan_bitmap_skip_vima(SCAN_ARGS);
double scan_selvec_reeval_vima(SCAN_ARGS);     double scan_selvec_skip_vima(SCAN_ARGS);

typedef struct { const char *name; scan_fn function; bool needs_avx512; } Version;

static const Version versions[] = {
    { "scalar_bitmap_reeval",        scan_bitmap_reeval,          false },
    { "scalar_bitmap_skip",          scan_bitmap_skip,            false },
    { "scalar_selvec_reeval",        scan_selvec_reeval,          false },
    { "scalar_selvec_skip",          scan_selvec_skip,            false },
    { "parallel_bitmap_reeval",      scan_bitmap_reeval_par,      false },
    { "parallel_bitmap_skip",        scan_bitmap_skip_par,        false },
    { "parallel_selvec_reeval",      scan_selvec_reeval_par,      false },
    { "parallel_selvec_skip",        scan_selvec_skip_par,        false },
    { "simd_bitmap_reeval",          scan_bitmap_reeval_simd,     true  },
    { "simd_bitmap_skip",            scan_bitmap_skip_simd,       true  },
    { "simd_selvec_reeval",          scan_selvec_reeval_simd,     true  },
    { "simd_selvec_skip",            scan_selvec_skip_simd,       true  },
    { "simd_parallel_bitmap_reeval", scan_bitmap_reeval_simd_par, true  },
    { "simd_parallel_bitmap_skip",   scan_bitmap_skip_simd_par,   true  },
    { "simd_parallel_selvec_reeval", scan_selvec_reeval_simd_par, true  },
    { "simd_parallel_selvec_skip",   scan_selvec_skip_simd_par,   true  },
    { "vima_bitmap_reeval",          scan_bitmap_reeval_vima,     false },
    { "vima_bitmap_skip",            scan_bitmap_skip_vima,       false },
    { "vima_selvec_reeval",          scan_selvec_reeval_vima,     false },
    { "vima_selvec_skip",            scan_selvec_skip_vima,       false },
};
#define VERSION_COUNT (sizeof(versions) / sizeof(versions[0]))

// -----------------------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------------------
static void fail(const char *message, const char *detail) {
    fprintf(stderr, "error: %s%s%s\n", message, detail ? ": " : "", detail ? detail : "");
    exit(1);
}

static FILE *open_or_fail(const char *path, const char *mode) {
    FILE *file = fopen(path, mode);
    if (!file) fail("cannot open file", path);
    return file;
}

static void print_usage(const char *program) {
    fprintf(stderr, "usage: %s <version> <data> <predicates> <partitions> <skip> <output>\n", program);
    fprintf(stderr, "       %s list\n", program);
}

// =========================================================================================
// Main
// =========================================================================================
int main(int argc, char **argv)
{
    // -----------------------------------------------------------------------------------------
    // Lists the versions if requested
    // -----------------------------------------------------------------------------------------
    if (argc == 2 && strcmp(argv[1], "list") == 0) {
        for (size_t i = 0; i < VERSION_COUNT; i++) {
            printf("%s%s\n", versions[i].name, versions[i].needs_avx512 ? "  (AVX-512)" : "");
        }
        return 0;
    }
    if (argc != 7) {
        print_usage(argv[0]);
        return 1;
    }

    // -----------------------------------------------------------------------------------------
    // Chooses the version by name
    // -----------------------------------------------------------------------------------------
    const Version *version = NULL;
    for (size_t i = 0; i < VERSION_COUNT; i++) {
        if (strcmp(argv[1], versions[i].name) == 0) version = &versions[i];
    }
    if (!version) fail("unknown version (run `scan_main list`)", argv[1]);

    if (version->needs_avx512 &&
        !(__builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512vl") &&
          __builtin_cpu_supports("avx512dq"))) {
        fail("this CPU lacks AVX-512 F/VL/DQ (run it under Intel SDE: sde64 -skx --)", version->name);
    }

    // -----------------------------------------------------------------------------------------
    // Reads the database: "<rows> <columns>", then one row per line
    // -----------------------------------------------------------------------------------------
    /// Stored column by column (columns[c][row]), as the scans expect
    FILE *file = open_or_fail(argv[2], "r");
    size_t row_count, column_count;
    if (fscanf(file, "%zu %zu", &row_count, &column_count) != 2 || column_count == 0)
        fail("bad header in data file", argv[2]);
    if (row_count >= (1u << 31)) fail("too many rows (row ids must be < 2^31)", argv[2]);

    float **columns = malloc(column_count * sizeof(float *));
    for (size_t c = 0; c < column_count; c++) {
        columns[c] = malloc((row_count ? row_count : 1) * sizeof(float));
    }
    for (size_t row = 0; row < row_count; row++) {
        for (size_t c = 0; c < column_count; c++) {
            if (fscanf(file, "%f", &columns[c][row]) != 1) fail("missing value in data file", argv[2]);
        }
    }
    fclose(file);

    // -----------------------------------------------------------------------------------------
    // Reads the predicates: "<count>", then "<column> <low> <high>" per line
    // -----------------------------------------------------------------------------------------
    file = open_or_fail(argv[3], "r");
    size_t predicate_count;
    if (fscanf(file, "%zu", &predicate_count) != 1 || predicate_count == 0)
        fail("bad header in predicates file", argv[3]);

    Predicate *predicates = malloc(predicate_count * sizeof(Predicate));
    for (size_t k = 0; k < predicate_count; k++) {
        if (fscanf(file, "%zu %f %f", &predicates[k].column, &predicates[k].low, &predicates[k].high) != 3)
            fail("bad predicate line", argv[3]);
        if (predicates[k].column >= column_count) fail("predicate column out of range", argv[3]);
    }
    fclose(file);

    // -----------------------------------------------------------------------------------------
    // Reads the partitions: "<count>", then "<low> <high>" per line (rows [low, high))
    // -----------------------------------------------------------------------------------------
    file = open_or_fail(argv[4], "r");
    size_t partition_count;
    if (fscanf(file, "%zu", &partition_count) != 1) fail("bad header in partitions file", argv[4]);

    Partition *partitions = malloc((partition_count ? partition_count : 1) * sizeof(Partition));
    for (size_t p = 0; p < partition_count; p++) {
        if (fscanf(file, "%zu %zu", &partitions[p].low, &partitions[p].high) != 2)
            fail("bad partition line", argv[4]);
        if (partitions[p].low > partitions[p].high || partitions[p].high > row_count)
            fail("partition out of range", argv[4]);
    }
    fclose(file);

    // -----------------------------------------------------------------------------------------
    // Reads the skip matrix: "<partitions> <columns>", then one line of 0/1 per partition
    // -----------------------------------------------------------------------------------------
    file = open_or_fail(argv[5], "r");
    size_t skip_rows, skip_columns;
    if (fscanf(file, "%zu %zu", &skip_rows, &skip_columns) != 2 ||
        skip_rows != partition_count || skip_columns != column_count)
        fail("skip header must be \"<partition_count> <column_count>\"", argv[5]);

    size_t skip_size = partition_count * column_count;
    bool *skip = malloc((skip_size > 0 ? skip_size : 1) * sizeof(bool));
    for (size_t i = 0; i < skip_size; i++) {
        int flag;
        if (fscanf(file, "%d", &flag) != 1 || (flag != 0 && flag != 1)) fail("skip values must be 0 or 1", argv[5]);
        skip[i] = flag;
    }
    fclose(file);

    // -----------------------------------------------------------------------------------------
    // Runs the chosen version and measures its time
    // -----------------------------------------------------------------------------------------
    struct timespec start, end;
    size_t qualifying_count = 0;

    clock_gettime(CLOCK_MONOTONIC, &start);
    ORCS_tracing_start();   // traced region: only the scan (serial code, around any parallel region)
    double sum = version->function(columns, column_count, predicates, predicate_count,
                                   partitions, skip, partition_count, &qualifying_count);
    ORCS_tracing_stop();
    clock_gettime(CLOCK_MONOTONIC, &end);
    double seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // -----------------------------------------------------------------------------------------
    // Writes the result to the output file and to the terminal
    // -----------------------------------------------------------------------------------------
    file = open_or_fail(argv[6], "w");
    fprintf(file, "version %s\nqualifying_count %zu\nsum %.6f\nseconds %.9f\n",
            version->name, qualifying_count, sum, seconds);
    fclose(file);

    printf("%-28s qualifying_count=%zu  sum=%.6f  time=%.6fs\n", version->name, qualifying_count, sum, seconds);

    // -----------------------------------------------------------------------------------------
    // Frees everything
    // -----------------------------------------------------------------------------------------
    for (size_t c = 0; c < column_count; c++) free(columns[c]);
    free(columns); free(predicates); free(partitions); free(skip);
    return 0;
}
