#!/usr/bin/env bash
# =========================================================================================
# Builds scan_main and runs it on the example input.
#
#   ./run.sh                  runs all 20 versions
#   ./run.sh <version>        runs one version (see: ./build/scan_main list)
#
# AVX-512 versions: if this CPU lacks AVX-512, set SDE to the sde64 binary
#   SDE=/path/to/sde64 ./run.sh
# Thread count for the parallel versions: OMP_NUM_THREADS=8 ./run.sh
# =========================================================================================
set -euo pipefail
cd "$(dirname "$0")"

BUILD=build
INPUT=example
CFLAGS="-std=c11 -O2 -Wall -Wextra -fopenmp"
AVX512="-mavx512f -mavx512vl -mavx512dq"

# -----------------------------------------------------------------------------------------
# Compiles each folder with its own flags
# -----------------------------------------------------------------------------------------
# Only the SIMD folders get AVX-512 flags, so the other versions never execute
# AVX-512 instructions (the compiler could otherwise auto-vectorize them with it).
mkdir -p "$BUILD"
for f in Scalar/scan_*.c Parallel/scan_*.c VIMA/scan_*.c VIMA/vima_isa.c; do
    gcc $CFLAGS -c "$f" -o "$BUILD/$(basename "${f%.c}").o"
done
for f in SIMD/scan_*.c SIMD_Parallel/scan_*.c; do
    gcc $CFLAGS $AVX512 -c "$f" -o "$BUILD/$(basename "${f%.c}").o"
done
gcc $CFLAGS scan_main.c "$BUILD"/scan_*.o "$BUILD"/vima_isa.o -o "$BUILD/scan_main"

# -----------------------------------------------------------------------------------------
# Decides how AVX-512 versions are launched
# -----------------------------------------------------------------------------------------
if grep -q avx512f /proc/cpuinfo; then
    AVX512_LAUNCHER=""
elif [ -n "${SDE:-}" ]; then
    AVX512_LAUNCHER="$SDE -skx --"
else
    AVX512_LAUNCHER="SKIP"
fi

# -----------------------------------------------------------------------------------------
# Runs the requested version(s)
# -----------------------------------------------------------------------------------------
if [ $# -ge 1 ]; then
    VERSIONS="$1"
else
    VERSIONS=$("$BUILD/scan_main" list | awk '{print $1}')
fi

for version in $VERSIONS; do
    launcher=""
    if [[ "$version" == simd* ]]; then
        if [ "$AVX512_LAUNCHER" = "SKIP" ]; then
            echo "$version: skipped (no AVX-512; set SDE=/path/to/sde64)"
            continue
        fi
        launcher="$AVX512_LAUNCHER"
    fi
    $launcher "$BUILD/scan_main" "$version" \
        "$INPUT/data.txt" "$INPUT/predicates.txt" "$INPUT/partitions.txt" "$INPUT/skip.txt" \
        "$BUILD/output_$version.txt"
done
