# OrCS VIMA: fixes and open issues

Nov 25, 2026 · @Rodrigo

Local copy of the shared doc https://claude.ai/artifact/LbXRvBL57XAzeW4GZRNCGX (exported Nov 25, 2026).

## Summary

VIMA simulations in OrCS were not trustworthy: the scan kernels deadlocked, results never reached DRAM, and several latencies and statistics were wrong. After eleven fixes, plus switching VIMA to the 3D-stacked memory it was proposed for, the four VIMA scans finish in 0.21–1.99 M cycles on a 100k-row input (they ran past 500 M cycles before). That is 3–21× faster than scalar. Bitmap VIMA also beats 4 threads by about 8×, while selvec VIMA is still 1.3–1.4× slower than 4 threads.

Gathers are now modelled with the addresses they really read. Three modelling issues remain open; the most important for the numbers is the missing coherence between VIMA and the CPU caches, so VIMA cycle counts are not final yet.

## Fixes

All fixes are committed on `master`; those marked "Affects results" change VIMA cycle counts. x86 and OpenMP traces gave identical cycle counts after every fix.

| Commit | Area | Problem | Change | Measured effect |
| --- | --- | --- | --- | --- |
| a998a87 | VIMA controller | Deadlock: an instruction took its destination line only at completion, in order; when younger instructions held all 8 lines the oldest could never complete. Shared lines were freed by the first user. The buffer loop skipped the instruction after a finished one. | Take all lines (sources + destination) at entry, enter in order, count line users, walk a snapshot of the buffer. | Region 3/4 of the scan: from never finishing (> 47 M / 72 M cycles) to 541 k / 193 k |
| 2ff41c6 | VIMA controller | Results never written to DRAM (fetch marked the result clean); every destination fetched its old 8 KB first. Writebacks to address 0. | Install the result without a fetch, keep it dirty until eviction; lookups match the vector a line holds or is fetching; busy lines are not evicted. | Scans 12–27% faster; vecsum 12% slower (512 reads became 510 writes) |
| e04c193 | VIMA controller | Lookup latency counter never reset: each instruction paid every lookup since the start, plus 2 cycles per retry. | Per-instruction lookup latency, cleared after use. | None alone (see 665a130) |
| 665a130 | VIMA controller | The lookup latency was never applied: TRANSMIT completed without checking readyAt. | The head completes only after readyAt. | None measurable (6 cycles hide behind fetches) |
| 4b075eb | VIMA statistics | Fetch/writeback counters uninitialised (average fetch latency printed 1). | Initialise them; sum every way. | Stats only: 6,519 cycles per fetch (region 2) |
| 8c466ed | Processor | VIMA instructions could be sent out of program order (first ready, not oldest). | Send only the oldest unsent VIMA instruction (Santos 2022, Sec. 4.8–4.10). | None in our traces (already in order) |
| e05bfb3 | Processor | VIMA entries hashed address 0 in memory disambiguation: spurious dependencies, no real ones. | Skip disambiguation for VIMA. | None in our traces |
| c900f25, 8c487ea | Tracer, processor, kernels | CPU loads of VIMA results did not wait for VIMA. | `_vim_fence()` writes a `$5` marker; OrCS drains the core there; scans fence before every CPU read of a VIMA result. | Scans −1% to +3%; 1.4–1.9 M cycles of drain stalls made explicit |
| e429a43 | Tracer | Broadcasts (`fmovs` etc.) recorded as read + write, so OrCS fetched a vector it overwrites. | Record broadcasts as write-only. | 7–13 fewer 8 KB fetches per scan; cycles ±2% |
| 0b6df45, 2dae364 | Build | Kernel and tracer makefiles missed header/include dependencies, leaving stale binaries. | Add the dependencies. | Build only |
| f5b172d | Config | VIMA ran on the DDR controller (8 KB rows, 4 channels): a whole vector mapped to one channel, and its 128 line fetches were serialized. | skylakeServerVIMA.cfg uses memory\_controller\_3D\_closed.cfg (32 vaults, 256 B rows), the memory VIMA was proposed for. | Fetch \~6,500 → \~350 cycles; bitmap VIMA 12×, selvec VIMA 1.6× faster; scalar 10–14% slower |
| f888afe, 6ef5960 | Tracer, trace reader, VIMA controller | Gathers fetched 8 KB contiguous from the column base: wrong data, wrong addresses. The gather source could also hit, and refetch over, a line other instructions held. | The tracer writes the distinct 64 B lines each gather reads (G block); OrCS passes the list by shared pointer to a fresh VIMA line, which fetches exactly those lines and is never matched by address. | Selvec: 169–189 lines per gather instead of 128; cycles +0.04% to +0.15% |

Affects results (VIMA cycle counts): a998a87, 2ff41c6, e04c193, 665a130, 8c466ed, e05bfb3, 8c487ea, e429a43, f5b172d (also changes the CPU versions), f888afe + 6ef5960 (selvec, after a retrace).

## Results

Cycle counts (Global\_Cycle) on the 100k-row input. Scalar and 4-thread runs use the 3D memory. "VIMA, DDR" is the result after all controller fixes but before the switch to 3D memory. Before the fixes, every VIMA scan deadlocked (> 530 M cycles). Skip beats reeval in every column.

| Version | Scalar | 4 threads | VIMA, DDR | VIMA, 3D | VIMA vs scalar |
| --- | --- | --- | --- | --- | --- |
| bitmap\_reeval | 5,393,178 | 2,091,903 | 3,162,781 | 259,838 | 20.8× faster |
| bitmap\_skip | 4,221,476 | 1,603,458 | 2,037,321 | 205,466 | 20.5× faster |
| selvec\_reeval | 6,154,438 | 1,491,547 | 3,226,887 | 1,989,306 | 3.1× faster |
| selvec\_skip | 5,354,079 | 1,173,646 | 2,558,584 | 1,629,054 | 3.3× faster |

On the 3D memory an 8 KB VIMA fetch takes about 350 cycles. Each vault serves one 256 B row per vector, with 1 row miss and 3 hits, and the load is even across all 32 vaults. The CPU versions are slower than on DDR because closed 256 B rows make almost every access a row miss. Selvec VIMA gains less: its time is dominated by CPU compaction, fences and gathers.

Selvec VIMA uses traces with the gathered addresses (VIMA, 3D column). Its row ids are sorted and fairly dense, so a gather reads 169–189 lines instead of 128, and the extra lines land in parallel vaults: cycles rose by only 0.04–0.15%. On DDR, all versions returned the same result (count 28,442, sum 1,461,438), and the retraced selvec VIMA scans return it too; the other 3D runs were not re-checked. The AVX-512 versions are not included: this machine cannot trace them.

## Open issues

The first one distorts current VIMA results; the rest are modelling gaps or latent bugs, or affect only DDR configs.

| Issue | Impact | Where to look | Decision needed |
| --- | --- | --- | --- |
| No coherence between the VIMA cache and the CPU caches | CPU writes (e.g. selvec row ids) stay in CPU caches while VIMA reads DRAM; CPU reads of VIMA results go through CPU caches. Santos (2022, Sec. 4.4) assumes a non-cacheable region | Processor/cache manager | Model the non-cacheable region or flushes |
| VIMA writes are not ordered against gathered lines | `store_hash` checks only a gather's base, so a gather could read lines an older VIMA instruction is still writing. Not the case in our scans | `vima_controller.cpp`, `can_enter` | Check every gathered line, or require a fence |
| Vectors are identified by their 8 KB-aligned block | Two unaligned vectors starting in the same block would be merged (false hit). Not seen in current traces (51–89% of vectors unaligned, no shared block) | `vima_controller.cpp`, `search_cache`, `store_hash` | Match on the exact start address |
| `VIMA_UNBALANCED` does nothing (`read*_unbalanced` never assigned) | Unaligned vectors span 2 DRAM rows without extra modelling | `vima_controller.cpp` | Remove or implement |
| DDR configs only: every DRAM request is split into 8 KB row-sized sub-requests, and the channel is chosen by the bits above the row | A vector or long stream goes to one channel and row (\~40 cycles per line). No longer affects VIMA, which now uses 3D memory; still affects CPU simulations on DDR configs | `main_memory/memory_controller.cpp`, `requestDRAM` and `set_masks` | Decide whether the mapping is intended |

Performance notes, not bugs: the 8-line VIMA cache (64 KB) refetches broadcast vectors; Santos (2022) used 256 KB (32 lines, `vima_config/8192_256K.cfg`). Entering in order costs \~12% overlap in one region; the thesis' load-ahead (Sec. 4.9) would recover it.

## Method and setup

- **Kernels:** 16 scan versions from `trace_generator/kernels/Extracted` plus 4 VIMA versions written with the current VIMA ISA (range test with `fslts`/`fadds`/`fsubs`, sum and count with `fcums`, selvec gathers with `fgtrs` and CPU compaction). Traced with `trace.sh` (only the scan call is traced).
- **Configs:** `skylakeServerVIMA.cfg` (1 core, 8 KB vectors, 8-line VIMA cache, 3D-stacked memory: memory\_controller\_3D\_closed.cfg, 32 vaults, 256 B closed rows) for scalar and VIMA; a 4-core copy of it for the OpenMP versions. The older `sandy_*` configs no longer run on the current simulator.
- **Inputs:** a generated 100k-row, 3-column input with 4 partitions, not multiples of 2048 rows (results above); a 30k-row input with one aligned and one unaligned partition (debugging).
- **How the bugs were found:** tracing one loop of `vima_bitmap_reeval` at a time (5 regions, 30 s simulation cap) isolated the slow loops; `VIMA_DEBUG` traces and temporary probes on the controller state showed the deadlock, the skipped instruction and the missing writebacks. Each fix was checked on the regions, `vecsum` and the full scans, and x86 traces were re-run to confirm no side effects.
- **Task list:** `docs/task_lists/vima_controller.md` in the repo tracks the same items.
