# OrCS multi-thread sync: implementation tasks

Nov 24, 2026 · @Rodrigo

This list puts section 3 of the [main task list](https://claude.ai/artifact/HhNrsaUBU924DHQcXrkbDi) into practice: OrCS stops at barriers and critical sections instead of ignoring the `$` markers. Each step is one commit, made in this order. Barriers are released only when every core has committed all of its instructions before the marker.

- `$` markers are trace lines, not instructions. They never enter the ROB and are not counted.
- The tracer writes no parallel start/end markers, so that part stays with the `GOMP_parallel` end-marker item in the main list.
- Test kernel: `ompsync`, at N = 1, 2 and 4.

## Step 1: the reader reports sync lines

Today `trace_next_dynamic` skips `$` lines (trace\_reader.cpp:407), so the processor never sees them.

- [x] Add a `sync_type` field to `opcode_package_t`, with codes from the tracer enum: 0 = barrier, 1 = wait critical start, 2 = critical start, 3 = critical end, 4 = free. Ordinary instructions use `SYNC_NONE = -1` (`0xFF` if the field is unsigned), set wherever the package is reset (constructor, `package_clean`). A package is a marker when `sync_type != SYNC_NONE`.
- [x] In `trace_next_dynamic`, return the `$N` event instead of running `continue`.
- [x] In `trace_fetch`, hand the event to the processor as a marker that isn't counted as an instruction.
- [ ] For now, `fetch()` drops the marker. Check that `vecsum` and `ompsync` at N = 1 give exactly the same results as before.
- [x] Commit: `feat(trace_reader): report sync markers to the processor`.

Step 1 result: commit 579fa0c. `ompsync` N = 1 output is identical to the previous build (12,584,306 instructions, 14,289,555 cycles). `vecsum` has not been run yet, so that item stays open. `sync_type_t` lives in `utils/enumerations.hpp`, because `trace_reader.hpp` is included before `opcode_package.hpp`.

**Fixes needed to run N = 4** (found during step 1, all with identical N = 1 output):

- [x] c5706d4 `fix(processor)`: the `MAX_NUM_CACHE_LEVELS` check counted cache entries (9 for 4 cores) instead of levels (3), so every multi-core config aborted. `printCache` read out of bounds for the same reason. `skylake_4cores` now runs `ompsync` N = 4 to the end.
- [x] c329791 `build(makefile)`: `-MMD -MP`, so changing a header rebuilds every object that includes it. Before, stale objects read wrong class layouts.
- [x] 3ab1823 `fix(cache)`: `generateIndexArray` uses `processor_id % AMOUNT` instead of `& (AMOUNT - 1)`, so core counts that aren't a power of 2 map correctly.

## Step 2: the sync manager

A barrier involves every core, so one shared object in `orcs_engine` decides when it opens.

- [x] New `sync_manager.cpp/.hpp`. Barrier: `arrive(core)`, an arrived set, a generation counter, and `active_cores`.
- [x] A core waits until its generation ends, so a fast core can't slip through the next barrier.
- [x] One global lock: `try_acquire(core)` and `release(core)`. The trace doesn't say which lock a critical section uses.
- [x] Create it in `orcs_engine` with the number of cores that have a trace.
- [x] `fetch()` stops at a marker until the manager lets the core go. Count the waiting cycles in a new stat, `stall_sync_cycles`.
- [x] Commit: `feat(processor): stall fetch at barriers and critical sections`, with `Affects results: yes`.

Step 2 result: commit fcef596 `feat(sync)`. The manager is in `sync/sync_manager.*` and the fetch logic is in `processor_t::sync_ready()`. The new statistics are `Stalls Sync`, `Sync Barriers` and `Sync Critical Sections`.

- `ompsync` N = 1: identical to the previous build apart from the new statistics lines.
- `ompsync` N = 4: completes with 2 barriers and 1 critical section per core. Sync stall cycles per core: 0 / 27,805 / 1,005 / 28,397.

The traces have no barrier at the end of a parallel region: the first `$0` is at the region's start. Fix the tracer's `GOMP_parallel` end marker before steps 3 and 4. Fixed in 95da0c2: each region now ends with a $0 in every thread, and at N = 4 the cores finish within 114 cycles of each other (3 barriers per core).

## Step 3: finished cores count as arrived

A core that finishes early must not leave the other cores waiting at a barrier it will never reach.

- [x] When `traceIsOver` becomes true, call `sync_manager.retire(core)`. It lowers `active_cores` and checks the current barrier again.
- [x] If the finishing core holds the lock, release it and print a warning.
- [x] Commit: `fix(processor): count finished cores as arrived at barriers`, with `Affects results: yes`.

Step 3 result: commit 6487853 `fix(sync)`, with the module `sync` rather than `processor`. The "last core opens the barrier" check moved to `open_if_complete()`, which ignores empty barriers. When thread 1's trace is cut after the first barrier, the previous build hangs (400 s timeout) and this one completes. The uncut N = 4 run is identical to the previous build.

## Step 4: drain before the sync takes effect

A core arrives, acquires or releases only after its older instructions commit. That way no work crosses the barrier or the critical section out of order.

- [x] Per-core state: `NONE → DRAINING → WAITING → NONE`.
- [x] DRAINING: no fetching, and wait until `robUsed == 0`, the fetch and decode buffers are empty, and the MOB is empty.
- [x] Then call `arrive()` for a barrier or `try_acquire()` for a critical start, and go to WAITING.
- [x] A critical end also drains first, then calls `release()` and goes straight back to NONE.
- [x] Commit: `fix(processor): drain the pipeline before a sync point`, with `Affects results: yes`.

Step 4 result: commit 94635ef `feat(sync)`. Done before step 3. The drain is a check at the start of `sync_ready()` using `pipeline_empty()` (fetch and decode buffers, ROB, and the 4 MOBs), not separate states. New statistic: `Stalls Sync Drain`. In the `-parallel_start 1 -parallel_end 1` run at N = 2, the cores now finish 4 cycles apart instead of 162. At N = 4 the gap stays at 114 cycles, which is the master's longer tail after the join (69 instructions against 21 to 24). At N = 1 the run takes 1,240 more cycles (+0.009%), with unchanged instruction counts.

## Verification

- [x] N = 1: instruction count exactly the same as before; cycles change only a little, from the drains.
- [x] N = 4: every core has 2 barrier arrivals and 1 lock acquisition, and the run ends without a deadlock.
- [x] A debug log of `(cycle, core, event)` shows that no core passes barrier k before every core has arrived at it.
- [x] Only one core is inside the critical section at any cycle.
- [x] Run with one trace cut short by hand: the simulation still finishes (step 3).
- [x] Store before the barrier and load after it: the load issues after the store commits (step 4).

Verification results (`ompsync` traces with end markers; `skylakeServer` for N = 1, `skylake_4cores` for N = 4):

- **N = 1:** instructions unchanged (12,584,306). Cycles 14,289,626 → 14,290,866 (+1,240, all from 1,594 drain cycles).
- **N = 4:** every core has **3** barriers (start, explicit, and the end of the region added in 95da0c2) and 1 critical section, and the run finishes.
- **Debug log:** `SYNC_DEBUG` in `defines.hpp` (off by default) prints `SYNC <cycle> <core> ARRIVE|PASS|LOCK_ACQUIRE|LOCK_RELEASE|FIRST_LOAD|RETIRE`. The checker script `check_sync.py` passes on the full and cut N = 4 runs: for every barrier, all active cores arrive before the first core passes (barrier 1: last arrival and first pass both at cycle 3,761,187).
- **Critical section:** 4 acquisitions, each after the previous core's release (for example core 0 releases at 4,884,222 and core 1 acquires in the same cycle). There is never a second owner.
- **Cut trace:** core 1 retires at cycle 3,759,702, and barriers 1 and 2 open with cores 0, 2 and 3.
- **Store → load:** at every barrier, the last store commit on any core happens before the first load on any core after it. Barrier 1: store commit at 3,761,187, first load at 3,761,204. The last store is the return address that `call` pushes before the barrier routine.
