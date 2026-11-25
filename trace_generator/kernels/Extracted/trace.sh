#!/usr/bin/env bash
# =========================================================================================
# Generates OrCS x86 traces of the scan versions with sinuca_tracer.
#
#   ./trace.sh                  traces all 16 versions
#   ./trace.sh <version> ...    traces the given versions (see: build/scan_main.trace list)
#
# Env:
#   INPUT=dir        input folder with data/predicates/partitions/skip.txt (default: example)
#   THREADS=N        threads for parallel* versions (default: 4); others use 1
#   TRACE_DIR=dir    output root (default: <dir containing OrCS_Mestrado>/traces)
#
# Traces go to $TRACE_DIR/scan/<version>/<version>.*  — only the scan call is traced.
# simd* versions are skipped on hosts without AVX-512 (trace them on another machine).
# =========================================================================================
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../.." && pwd)
GEN=$REPO/trace_generator
PIN=${PIN:-$GEN/pin}
TOOL=${TOOL:-$GEN/extras/pinplay/bin/intel64/sinuca_tracer.so}
INPUT=$(cd "${INPUT:-$HERE/example}" && pwd)
THREADS=${THREADS:-4}
OUT=${TRACE_DIR:-$(dirname "$REPO")/traces}/scan
export OMP_WAIT_POLICY=passive   # waiting threads sleep instead of spinning (faster under Pin)

[ -f "$TOOL" ] || { echo "missing $TOOL: run make in $GEN/extras/pinplay/sinuca_tracer"; exit 1; }

# WORKAROUND: Pin 3.11 crashes on multi-threaded programs with glibc >= 2.34 (clone3).
NOCLONE3=$GEN/noclone3
[ "$NOCLONE3" -nt "$NOCLONE3.c" ] || cc -O2 "$NOCLONE3.c" -o "$NOCLONE3"

make -C "$HERE" --no-print-directory
BIN=$HERE/build/scan_main.trace

if [ $# -gt 0 ]; then VERSIONS=("$@"); else mapfile -t VERSIONS < <("$BIN" list | awk '{print $1}'); fi
grep -q avx512f /proc/cpuinfo && HAS_AVX512=1 || HAS_AVX512=0

for v in "${VERSIONS[@]}"; do
    if [[ $v == simd* && $HAS_AVX512 == 0 ]]; then
        echo "$v: skipped (needs an AVX-512 host)"; continue
    fi
    [[ $v == parallel* || $v == simd_parallel* ]] && t=$THREADS || t=1
    dir=$OUT/$v; mkdir -p "$dir"
    echo "== $v ($t thread(s)) -> $dir"
    OMP_NUM_THREADS=$t "$NOCLONE3" "$PIN" -t "$TOOL" -trace x86 -orcs_tracing 1 -threads "$t" \
        -output "$dir/$v" -- "$BIN" "$v" \
        "$INPUT/data.txt" "$INPUT/predicates.txt" "$INPUT/partitions.txt" "$INPUT/skip.txt" \
        "$dir/result.txt"
done
