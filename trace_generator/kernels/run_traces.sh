#!/bin/bash
# Build kernels and generate x86 / VIMA traces with sinuca_tracer.
# Usage: PIN_ROOT=/path/to/pin [THREADS=N] ./run_traces.sh [kernel ...]   (default: all, THREADS=1)
# Traces go to $TRACE_DIR (default: <dir containing OrCS_Mestrado>/traces).
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../.." && pwd)
PIN=${PIN_ROOT:?set PIN_ROOT}/pin
TOOL=${TOOL:-$REPO/trace_generator/extras/pinplay/bin/intel64/sinuca_tracer.so}
THREADS=${THREADS:-1}
export OMP_NUM_THREADS=$THREADS   # traced program must spawn as many threads as trace files
OUT=${TRACE_DIR:-$(dirname "$REPO")/traces}

# WORKAROUND: Pin 3.11 crashes on multi-threaded programs with glibc >= 2.34
# (clone3). noclone3 disables clone3 so glibc falls back to clone.
# Remove once Pin is upgraded.
NOCLONE3=$REPO/trace_generator/noclone3
[ "$NOCLONE3" -nt "$NOCLONE3.c" ] || cc -O2 "$NOCLONE3.c" -o "$NOCLONE3"

if [ $# -gt 0 ]; then KERNELS=("$@"); else
    KERNELS=(); for m in "$HERE"/*/Makefile; do k=$(basename "$(dirname "$m")"); [ "$k" != common ] && KERNELS+=("$k"); done
fi

for k in "${KERNELS[@]}"; do
    make -C "$HERE/$k"
    mkdir -p "$OUT/$k/x86" "$OUT/$k/vima"
    "$NOCLONE3" "$PIN" -t "$TOOL" -trace x86  -orcs_tracing 1 -threads "$THREADS" -output "$OUT/$k/x86/$k.x86"     -- "$HERE/$k/$k.x86"
    "$NOCLONE3" "$PIN" -t "$TOOL" -trace iVIM -orcs_tracing 1 -threads "$THREADS" -output "$OUT/$k/vima/$k.vima2K" -- "$HERE/$k/$k.vima"
done
