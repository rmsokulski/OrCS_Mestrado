# Scan versions — input files

`scan_main` runs one of 16 scan versions over 4 input files and writes the result to a 5th file.

```
build/scan_main <version> <data> <predicates> <partitions> <skip> <output>
build/scan_main list        # prints the 16 version names
```

Versions are named `<implementation>_<storage>_<evaluation>`:

| Part | Values |
|---|---|
| implementation | `scalar`, `parallel` (OpenMP), `simd` (AVX-512), `simd_parallel` (AVX-512 + OpenMP) |
| storage | `bitmap` or `selvec` (selection vector) |
| evaluation | `reeval` (tests every predicate) or `skip` (uses the skip file) |

Example: `simd_parallel_selvec_skip`.

## The query

Every version answers the same query:

```sql
SELECT SUM(col0), COUNT(*) FROM data
WHERE  low_1 <= col_1 <= high_1 AND low_2 <= col_2 <= high_2 AND ...
```

It scans only the rows listed in the partitions file.

## Input files

All files are plain text and whitespace-separated. The first line is a header with sizes.

### 1. `data.txt`: the table

```
<rows> <columns>
v00 v01 v02 ...      ← row 0
v10 v11 v12 ...      ← row 1
```

- One row per line, values are floats.
- Rows are numbered from 0 in file order.
- Column 0 is the one that gets summed.

### 2. `predicates.txt`: the filter

```
<count>
<column> <low> <high>
```

- A row passes a predicate when `low <= value <= high` (inclusive).
- A row qualifies when it passes **all** predicates.
- `column` must be less than `<columns>` in `data.txt`.

### 3. `partitions.txt`: which rows to scan

```
<count>
<low> <high>
```

- Each line is the row range `[low, high)`: `low` is included, `high` is not.
- Rows outside every partition are never looked at.
- To scan the whole table, use one partition `0 <rows>`.
- In the original code, an index search produces these ranges.

### 4. `skip.txt`: what the index already guarantees

```
<partitions> <columns>
f f f ...            ← partition 0, one 0/1 flag per column
f f f ...            ← partition 1
```

- The header must equal the partition count and the column count.
- Flag `1` at (partition p, column c) means **every** row in partition p already passes the predicate on column c. The `skip` versions don't test it; the `reeval` versions ignore this file.
- ⚠️ A `1` must be true. If it isn't, the `skip` versions return different results from the `reeval` ones.
- A file of all `0`s is always safe.

### 5. Output file (written by `scan_main`)

```
version scalar_selvec_skip
qualifying_count 9
sum 418.000000
seconds 0.000001007
```

## How the files relate

```
data.txt ──── <columns> ───┬── predicates.txt  (column index < columns)
   │                       └── skip.txt        (one flag per column)
   └── <rows> ──── partitions.txt  (0 <= low <= high <= rows)
                       │
                       └── <count> ── skip.txt (one line per partition)
```

## Example (`example/`)

- `data.txt`: 40 rows × 3 columns; column 0 is sorted, as an index would leave it.
- `predicates.txt`: `col0 ∈ [20,80]`, `col1 ∈ [10,60]`, `col2 ∈ [0,90]`.
- `partitions.txt`: 4 partitions of 10 rows each.
- `skip.txt`: real coverage flags. Partition 2 is fully inside `[20,80]` on column 0, and partitions 0 and 1 are fully inside `[0,90]` on column 2.
- Expected result: `qualifying_count 9`, `sum 418`.

## Running

```
./run.sh                    # build + run all 16 versions on example/
./run.sh simd_bitmap_skip   # one version
SDE=/path/to/sde64 ./run.sh # AVX-512 versions on a CPU without AVX-512
OMP_NUM_THREADS=4 ./run.sh  # thread count for the parallel versions
```

Results go to `build/output_<version>.txt`.

Limits: fewer than 2^31 rows (row ids are 32-bit), and the AVX-512 versions need AVX-512 F, VL and DQ.
