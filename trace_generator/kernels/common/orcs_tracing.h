// Tracing region markers recognized by sinuca_tracer
// (symbols _Z18ORCS_tracing_startv / _Z17ORCS_tracing_stopv).
// Run the tracer with -orcs_tracing 1 so only these markers control tracing.
#pragma once

__attribute__((noinline)) void ORCS_tracing_start() { asm volatile(""); }
__attribute__((noinline)) void ORCS_tracing_stop()  { asm volatile(""); }
