// C version of ../common/orcs_tracing.h.
// sinuca_tracer finds the markers by their C++ symbol names, so the C functions
// are given those exact names with asm labels.
// Same rules: run the tracer with -orcs_tracing 1, and call the markers from
// serial code only, never inside an OpenMP parallel region.
#pragma once

void ORCS_tracing_start(void) __asm__("_Z18ORCS_tracing_startv");
void ORCS_tracing_stop(void)  __asm__("_Z17ORCS_tracing_stopv");

__attribute__((noinline, used)) void ORCS_tracing_start(void) { __asm__ volatile(""); }
__attribute__((noinline, used)) void ORCS_tracing_stop(void)  { __asm__ volatile(""); }
