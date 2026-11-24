// Tracing region markers recognized by sinuca_tracer
// (symbols _Z18ORCS_tracing_startv / _Z17ORCS_tracing_stopv).
// Run the tracer with -orcs_tracing 1 so only these markers control tracing.
//
// The markers flip ONE switch shared by all threads, and calls do not nest
// (start, start, stop = off). Several start/stop pairs in sequence are fine.
// Call them from serial code, around the parallel region, never inside it:
// a stop from the first thread to finish would stop tracing for all threads.
//
//   ORCS_tracing_start();
//   #pragma omp parallel
//   { ... }
//   ORCS_tracing_stop();
#pragma once

__attribute__((noinline)) void ORCS_tracing_start() { asm volatile(""); }
__attribute__((noinline)) void ORCS_tracing_stop()  { asm volatile(""); }
