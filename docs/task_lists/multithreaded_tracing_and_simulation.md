# OrCS multi-threaded tracing and simulation: fix and verify steps

Nov 24, 2026 · @Rodrigo

## Context

The tracer can already write multi-threaded traces, but OrCS ignores the synchronization markers in them, so a multi-threaded simulation is not yet meaningful. Findings come from reading the code only; no multi-threaded trace has been generated yet.

- **Tracer (`sinuca_tracer.cpp`)**: `-threads N` opens one `<out>.tidK.dyn.out.gz` and one `<out>.tidK.mem.out.gz` per thread, plus one shared `tid0.stat`. libgomp (`GOMP_*`) calls emit `$SYNC_*` lines for barriers, critical sections and parallel start/end.
- **Wrappers**: `createTrace.py` and `kernels/run_traces.sh` never pass `-threads`, so it stays at 1.
- **OrCS reader (`trace_reader.cpp`)**: core K opens `tidK` files, so `-t <base>` repeated once per core already works. `$` lines are skipped (line \~407), so threads run with no barriers between them.

## 1. Tracer fixes

- [x] Add a `--threads N` flag to `createTrace.py` and a `THREADS` variable to `run_traces.sh`; forward it as `-threads N` to Pin.
- [x] Export `OMP_NUM_THREADS=N` in both wrappers so the run matches the trace file count.
- [x] Replace the `threadid < max_threads` assertion in `ThreadStart` with a clear error message naming the thread ID and `-threads` value. Spare IDs are not needed: with `OMP_NUM_THREADS=N`, libgomp starts exactly N threads (tested with N = 4).
- [ ] (Possible improvement) Let the traced program set the thread count instead of `-threads`: allocate `thread_data` for `PIN_MAX_THREADS`, open `tidK` files in `ThreadStart`, keep a list of copied (`is_spawn`) markers and replay it into each new thread's file, loop over started threads instead of `max_threads`, and keep `-threads` as an optional cap. First check whether Pin reuses thread IDs after a thread exits; if it does, two threads end up in one `tidK` file today.
- [x] Work around Pin 3.11 crashing on multi-threaded programs with glibc >= 2.34: the `noclone3` launcher makes `clone3` return ENOSYS so glibc falls back to `clone`.
- [ ] **Warning:** `noclone3` is a temporary workaround. Upgrade Pin to a release that supports `clone3` (and `.relr.dyn`, which today forces `-static`), then remove the launcher.
- [x] Update the GOMP routine lists for GCC 13 libgomp. Also fixed: the sync hooks read freed memory (`tempStr.c_str()`), so no `$SYNC` markers were ever written.
- [x] Remove the block in `ImageLoad` that hooks `GOMP_parallel_end` to `handleOrCSTracingEvent(true)`: with `-orcs_tracing 1` it turns tracing back on at the end of every parallel region.
- [x] Mark the end of `GOMP_parallel` regions. Not an easy fix: in a static build `GOMP_parallel` ends the region internally, so there is no `GOMP_parallel_end` call to hook. Only the combined `GOMP_parallel_loop_*` calls emit an end marker. Options: an `IPOINT_AFTER` hook on `GOMP_parallel` (Pin doesn't guarantee it fires on every return path), or a hook on the internal `gomp_team_end`. Either way, the marker must be copied to every thread's trace, like the start marker.

  Now blocking the simulation work. In the `ompsync` traces, each thread's markers are `$0` (start of region), work, `$0` (explicit barrier), work, `$2 $3`, then 29 to 101 instructions with no barrier after them. So OrCS has no join at the end of a region: at N = 4, cores finish up to 8k cycles apart (commit fcef596).

  Fixed in 95da0c2. Every thread writes its own `$0` when it enters `gomp_team_barrier_wait_final`, the implicit barrier that both the master and the workers call. The master counts the region end in `gomp_team_end`, right after it leaves that barrier, so `-parallel_end` can't stop tracing before every thread has its marker. On `ompsync` (x86 and VIMA, N = 1/2/4), every thread now ends with `$2 $3 $0`, and the N = 1 instruction counts are unchanged. In OrCS at N = 4, each core has 3 barriers and all four finish within 114 cycles of each other. 26c67ac fixed the tracer build: it had been broken since 579fa0c by a duplicate `SYNC_*` enum.

  For now, the tracer prints a warning at the end of the run when the number of region starts and ends differ.
- [x] Make `createTrace.py` work from any directory (`tracerFolder` is relative to `trace_generator/`).
- [x] Make `count_inst` atomic, or per thread, so `-number_max_inst` is reliable with several threads.
- [ ] (Low priority) Possible race in the "last active thread" check in `control_instrumented_bbl`, only when `-parallel_start` and `-parallel_end` are both set: the early exit after the last traced region may be skipped, so the program runs to the end. Traces are not affected. The tracer now prints a warning when both options are set. Not locked, because a lock would slow every basic block.
- [x] Document that `ORCS_tracing_start/stop` is a single global switch: call it once, in serial code, around the parallel region.
- [x] Decide whether libgomp spin-wait loops should be traced; if not, stop emitting BBLs inside `gomp_barrier_wait*` and `gomp_team_barrier_wait*`.

  Decided: don't trace them. The tracer skips the barrier, mutex, semaphore and ptrlock spin routines, and the wrappers set `OMP_WAIT_POLICY=passive`.
- [x] Check that `synthetic_trace_generation` (VIMA/HMC) writes to the calling thread's files before tracing VIMA kernels with several threads.

  Checked, no change needed: every VIMA/HMC/MIPS write uses `IARG_THREAD_ID`. In a 4-thread OpenMP `vecsum` VIMA test, each thread had 128 `_vim2K_fadds` calls and 384 memory entries (8,192 bytes each), all inside its own quarter of `a`, `b` and `c`.

## 2. Tracer verification

- [x] Write a test kernel in `kernels/` with an OpenMP parallel vector sum, one explicit `#pragma omp barrier` and one `critical` section, wrapped in `ORCS_tracing_start/stop`.

  Done: `kernels/ompsync` (x86 and VIMA builds, self-checking checksum). First 4-thread trace: every thread has 2 barrier markers (one at the start of the region and the explicit one; corrected later: the implicit barrier at the end is missing) and 1 critical start/end pair, in both builds.
- [x] Trace it with N = 2 and N = 4; confirm Pin finishes without the `ThreadStart` assertion.

  Done for N = 1, 2 and 4 (x86 and VIMA builds): Pin finishes, and the checksum is correct under Pin.
- [x] Confirm N files each of `tidK.dyn` and `tidK.mem` exist and that every `tidK.dyn` is non-trivial.

  Exactly N file pairs each time. At N = 4, each x86 `tidK.dyn` has about 525k lines.
- [x] Confirm each thread's dynamic trace has matching `$` barrier markers (same count per thread) and paired critical start/end markers.

  Every thread, at every N: 2 `$0` (one at the start of the region and the explicit barrier; the implicit end barrier is missing, see the GOMP\_parallel end-marker item), 1 `$2` and 1 `$3`.
- [x] Confirm there are no "Found different GOMP call" comments left.

  None in any trace.
- [x] Compare the summed instruction count of the N-thread trace with the 1-thread trace; the difference should be roughly runtime overhead, not doubled work.

  x86: 12,584,306 instructions at N = 1, +2,233 at N = 2 (0.02%), +6,256 at N = 4 (0.05%). VIMA: 6,297,973, +2,236 and +6,265. The work is split evenly; thread 0 has about 4k more instructions at N = 4 (region setup).
- [x] Spot-check that the memory addresses in each `tidK.mem` cover the expected slice of the array for that thread.

  At N = 2 and 4 (x86), thread K writes only its slice of `c` and reads the same slice of `a` and `b`, then reads thread K+1's slice of `c` after the barrier, as the kernel does.

## 3. OrCS simulator fixes

- [x] Parse `$<SYNC_*>` lines in `trace_reader_t` instead of skipping them, and pass the event (barrier, critical start, critical end, parallel start/end) to the processor.

  Done in 579fa0c. The tracer writes no parallel start/end markers, so that part moved to the `GOMP_parallel` end-marker item. Details are in the [implementation tasks](https://claude.ai/artifact/1YAQ8uErkAmMJUnKBJpF1X).
- [x] Add a global synchronization manager in `orcs_engine`: a barrier counter that stalls fetch on each core until all active cores arrive, and a lock for critical sections.

  Done in fcef596 (`sync/sync_manager.*`): one barrier with a generation counter and one global lock.
- [x] Make a finished core count as arrived at every later barrier, so the other cores do not deadlock.

  Done in 6487853. A trace cut after its first barrier no longer hangs the simulation.
- [x] Stall fetch at a sync point only after older instructions commit, so the barrier is not crossed out of order.

  Done in 94635ef: arrive, acquire and release only once the buffers, ROB and MOBs are empty.
- [ ] Add a helper or doc note for running an N-thread trace: an N-core config and `-t <base>` repeated N times.
- [x] Check how caches are shared across cores: which levels are private or shared, and whether shared levels see all cores' requests.

  L1I, L1D and L2 are private to each core, and the LLC is shared (`processor_id % count`). At N = 4 the LLC's 262,820 accesses equal the sum of the four L2s' accesses, so it sees every core's requests. Fixed: 7459aa4 (writeback address) and d1f856a (shared statistics printed once, MPKI with a shared LLC). The remaining problems are the items below.
- [ ] Cache write path: the known write bugs (writes merged by `isIn`, write-allocate) and the eviction/inclusion fixes are tracked in [OrCS cache write path: open items and known bugs](https://claude.ai/artifact/TCBB8NE4k1NP1j4Rqms9mK). Correction: the 0 LLC writebacks on `ompsync` aren't a bug. The 16 MB LLC holds the whole ≈ 12.6 MB footprint, so it never evicts.
- [x] On any LLC eviction, clean or dirty, invalidate the line's copies in every core, not only the requesting core's, and reset the victim's `line_ptr_caches` before reusing it. Today other cores keep stale copies that point at the reused LLC line, and a clean eviction clears nothing.

  Moved to the cache write path doc.
- [ ] Decide on coherence: read-sharing kernels can run without it; kernels that write shared data need at least invalidation of private copies.

  Planned for the next session, as a minimal write-invalidate on top of the inclusive LLC. Cache fixes done so far (ca05ec8, 25e363e, eaa1015, f6fbad2, 4bbb834, d281292, 6a1db5a) and the remaining items are in [OrCS cache write path: open items and known bugs](https://claude.ai/artifact/TCBB8NE4k1NP1j4Rqms9mK).
- [x] Add statistics: cycles stalled at barriers and in critical sections, per core.

  Per core: `Stalls Sync` (all sync waiting), `Stalls Sync Drain` (the part spent draining the pipeline), `Sync Barriers`, `Sync Critical Sections`.

## 4. Simulation verification

- [x] Run the 1-thread trace on a 1-core config and the N-thread trace on an N-core config; both must finish without deadlock.

  `ompsync`: N = 1 on `skylakeServer`, N = 2 on `skylake_2cores`, N = 4 on `skylake_4cores`. All of them finish, including N = 4 with one trace cut short.
- [x] Check that the barrier count seen by each core equals the count of `$` barrier markers in its trace.

  3 `$0` per trace and `Sync Barriers: 3` per core, and 1 critical section each. In the cut trace, core 1 shows 1 barrier, matching its 1 `$0`.
- [ ] Build a test trace with deliberately unbalanced work: the fast core must show stall cycles at the barrier equal to roughly the gap.
- [ ] Confirm total cycles for N threads fall between the 1-thread time / N and the 1-thread time, and track the expected speedup on a compute-bound kernel.
- [x] Confirm the critical section never runs on two cores in the same cycle (add a debug assert).

  Checked from the `SYNC_DEBUG` log, not with an assert: every acquisition follows the previous release. `lock_release` already stops with an error if a core releases a lock it doesn't hold.
- [ ] Compare shared-cache hit rates with 1 and N cores; shared data should show hits across cores.
- [ ] Re-run an existing single-thread x86 and VIMA trace and confirm results are unchanged (no regression).
