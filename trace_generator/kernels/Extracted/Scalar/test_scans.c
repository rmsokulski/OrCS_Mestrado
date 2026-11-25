/* Sanity test: all four variants must return the same (sum, count).
 * Data is sorted on column 0 and split into partitions so that some partitions
 * are fully covered on column 0 (skip = true), mimicking an index search.
 * Build: gcc -std=c11 -O2 -Wall test_scans.c scan_*.c -o test_scans */
#include <stdio.h>
#include <string.h>
#include "scan_common.h"

double scan_bitmap_reeval(float**, size_t, const Predicate*, size_t, const Partition*, const bool*, size_t, size_t*);
double scan_bitmap_skip  (float**, size_t, const Predicate*, size_t, const Partition*, const bool*, size_t, size_t*);
double scan_selvec_reeval(float**, size_t, const Predicate*, size_t, const Partition*, const bool*, size_t, size_t*);
double scan_selvec_skip  (float**, size_t, const Predicate*, size_t, const Partition*, const bool*, size_t, size_t*);

static int cmp(const void *a, const void *b) {
    float x = *(const float*)a, y = *(const float*)b; return (x > y) - (x < y);
}

int main(void) {
    enum { N = 100000, C = 3, P = 10 };
    float *cols[C];
    for (int c = 0; c < C; c++) {
        cols[c] = malloc(N * sizeof(float));
        for (int i = 0; i < N; i++) cols[c][i] = (float)(rand() % 1000);
    }
    qsort(cols[0], N, sizeof(float), cmp);   /* column 0 "indexed" */

    Predicate preds[C] = { {200, 700, 0}, {100, 500, 1}, {0, 900, 2} };
    Partition parts[P];
    bool skip[P * C];
    memset(skip, 0, sizeof skip);
    for (int p = 0; p < P; p++) {
        parts[p].low = (size_t)p * N / P; parts[p].high = (size_t)(p + 1) * N / P;
        float lo = cols[0][parts[p].low], hi = cols[0][parts[p].high - 1];
        skip[p * C + 0] = preds[0].low <= lo && hi <= preds[0].high;  /* covers() */
    }

    struct { const char *name; scan_fn f; } v[] = {
        {"bitmap  / reeval", scan_bitmap_reeval}, {"bitmap  / skip  ", scan_bitmap_skip},
        {"selvec  / reeval", scan_selvec_reeval}, {"selvec  / skip  ", scan_selvec_skip},
    };
    double ref_sum = 0; size_t ref_n = 0; int ok = 1;
    for (int i = 0; i < 4; i++) {
        size_t n; double s = v[i].f(cols, C, preds, C, parts, skip, P, &n);
        printf("%s  sum=%.0f  count=%zu\n", v[i].name, s, n);
        if (i == 0) { ref_sum = s; ref_n = n; }
        else if (s != ref_sum || n != ref_n) ok = 0;
    }
    puts(ok ? "ALL MATCH" : "MISMATCH");
    for (int c = 0; c < C; c++) free(cols[c]);
    return !ok;
}
