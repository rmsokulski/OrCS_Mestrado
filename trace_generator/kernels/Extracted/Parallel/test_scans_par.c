/* Sanity test: the four parallel variants must return the same (sum, count) as the
 * scalar reference (Scalar/scan_selvec_skip.c).
 * Build: gcc -std=c11 -O2 -fopenmp -Wall test_scans_par.c scan_*_par.c ../Scalar/scan_selvec_skip.c -o test_scans_par */
#include <stdio.h>
#include <string.h>
#include <omp.h>
#include "scan_common.h"

double scan_selvec_skip      (float**, size_t, const Predicate*, size_t, const Partition*, const bool*, size_t, size_t*);
double scan_bitmap_reeval_par(float**, size_t, const Predicate*, size_t, const Partition*, const bool*, size_t, size_t*);
double scan_bitmap_skip_par  (float**, size_t, const Predicate*, size_t, const Partition*, const bool*, size_t, size_t*);
double scan_selvec_reeval_par(float**, size_t, const Predicate*, size_t, const Partition*, const bool*, size_t, size_t*);
double scan_selvec_skip_par  (float**, size_t, const Predicate*, size_t, const Partition*, const bool*, size_t, size_t*);

static int cmp(const void *a, const void *b) {
    float x = *(const float*)a, y = *(const float*)b; return (x > y) - (x < y);
}

int main(void) {
    enum { N = 1000003, C = 3, P = 7 };   /* odd sizes: partitions not word-aligned */
    float *cols[C];
    for (int c = 0; c < C; c++) {
        cols[c] = malloc(N * sizeof(float));
        for (int i = 0; i < N; i++) cols[c][i] = (float)(rand() % 1000);
    }
    qsort(cols[0], N, sizeof(float), cmp);

    Predicate preds[C] = { {200, 700, 0}, {100, 500, 1}, {0, 900, 2} };
    Partition parts[P];
    bool skip[P * C];
    memset(skip, 0, sizeof skip);
    for (int p = 0; p < P; p++) {
        parts[p].low = (size_t)p * N / P; parts[p].high = (size_t)(p + 1) * N / P;
        float lo = cols[0][parts[p].low], hi = cols[0][parts[p].high - 1];
        skip[p * C + 0] = preds[0].low <= lo && hi <= preds[0].high;
    }

    size_t ref_n; double ref_sum = scan_selvec_skip(cols, C, preds, C, parts, skip, P, &ref_n);
    printf("threads=%d  reference (scalar)  sum=%.0f  count=%zu\n", omp_get_max_threads(), ref_sum, ref_n);

    struct { const char *name; scan_fn f; } v[] = {
        {"bitmap / reeval / par", scan_bitmap_reeval_par}, {"bitmap / skip   / par", scan_bitmap_skip_par},
        {"selvec / reeval / par", scan_selvec_reeval_par}, {"selvec / skip   / par", scan_selvec_skip_par},
    };
    int ok = 1;
    for (int i = 0; i < 4; i++) {
        size_t n; double s = v[i].f(cols, C, preds, C, parts, skip, P, &n);
        printf("%s  sum=%.0f  count=%zu\n", v[i].name, s, n);
        if (s != ref_sum || n != ref_n) ok = 0;
    }
    puts(ok ? "ALL MATCH" : "MISMATCH");
    for (int c = 0; c < C; c++) free(cols[c]);
    return !ok;
}
