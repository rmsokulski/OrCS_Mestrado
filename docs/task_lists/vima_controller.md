# OrCS VIMA controller: bugs and fix steps

Nov 25, 2026 · @Rodrigo

## Context

Found while simulating the VIMA scan kernels (`trace_generator/kernels/Extracted/VIMA/`) on `skylakeServerVIMA.cfg` (8 KB vectors, 8-line VIMA cache, `VIMA_CACHE_LATENCY = 2`). The full scans ran past 500 M cycles without finishing. Tracing one kernel loop at a time (`vima_bitmap_reeval`, 30k-row input) showed two of its five loops never finishing, and `VIMA_DEBUG` traces showed why.

- **Controller (`vima/vima_controller.cpp`)**: instructions enter (PACKAGE_STATE_VIMA), wait for their source vectors (TRANSMIT), and complete in order at the head of `vima_buffer` (`check_completion`).
- **VIMA cache (`vima/vima_vector.cpp`)**: each line holds one 8 KB vector; a fetch or writeback sends 128 line requests to DRAM and waits for all of them.

After the fixes below, the four VIMA scans finish (100k-row input): 2.8–4.1 M cycles, 1.2–1.7× faster than scalar. But each VIMA instruction still costs about 4,000–5,000 cycles; the open items explain most of it.

## 1. Fixed

- [x] **Latency counter never reset.** `current_cache_access_latency` grew with every lookup since the start of the simulation, plus 2 cycles per retry of a lookup that found no free line. Each instruction now pays only its own lookups. Fixed in e04c193.
- [x] **Deadlock: destination line taken at completion.** Sources took lines at entry, the destination only at completion, and completion is in order. With younger instructions holding all 8 lines, the head could never get its destination. Region 4 (aggregation) stalled at 352 of 353 instructions for 70+ M cycles. Fixed in a998a87: an instruction enters only when it can take all its lines at once, instructions enter in order, and lines count their users.

  Same commit:
  - A line shared by two instructions was freed by the first to finish (`taken` was a flag, not a count), so a third instruction could take it while the second still used it.
  - `clock()` moved the buffer start mid-loop when the head finished, skipping the next instruction for that cycle. With in-order entry this let younger instructions pass it and block it.
  - The associative lookup tested `cache[i][i]` instead of `cache[index][i]` (only matters with associativity > 1).
- [x] **Lookup latency never applied.** TRANSMIT completed the head without checking `readyAt`, and `check_completion()` then overwrote it. Fixed in 665a130. No change in cycle counts: 6 cycles hide behind fetches of thousands.

## 2. Open: slowness

- [ ] **8 KB fetches are serialized.** An isolated vector fetch takes about 5,100 cycles, about 40 cycles per 64 B line, as if all 128 lines go to one channel and bank. DRAM traffic is uneven too: in region 2, channels 1 and 3 served 3,100 and 3,800 lines, channels 0 and 2 about 800 each. Check the DRAM address mapping (`BANK_ROW_BUFFER_SIZE = 8192` equals the vector size) and how `requestDRAM` queues a vector's sub-requests. May be intended hardware behaviour; decide before changing.
- [x] **Destinations are fetched before being overwritten.** Fixed in 2ff41c6 (with the next two items): the result is installed without a fetch.

  Original finding: At completion the destination line is set to WAIT (or TRANSMIT if dirty), so it fetches the old 8 KB from DRAM even when the instruction writes every element, and even on a hit. `check_completion()` has the "dirty → TRANSMIT, else WAIT" block twice in the old code; the second one applied on every path.
- [x] **Writebacks to address 0.** Gone after 2ff41c6: a line becomes dirty only once it holds a vector (0 of 50 writebacks in regions 2 and 3).

  Original finding: Lines never filled are written back with `address = 0`: `set_next_address` changes `next_address`, but `writeback()` uses `address`, which is only set by `fetch()`. Each costs 5,000–10,000 cycles of DRAM traffic. Seen as `WRITEBACK of address 0` in the `VIMA_DEBUG` traces.
- [ ] **Reused vectors are fetched again.** With 8 lines, the broadcast `low`/`high` vectors are evicted between blocks and fetched again (e.g. 235167744 fetched three times in region 2). May be a valid effect of the cache size; check after the other fixes.
- [ ] **In-order entry costs some overlap.** Region 2 went from 180,836 to 203,108 cycles (+12%) after the deadlock fix: an instruction now waits until all its lines are free. Revisit only if it matters after the fixes above.

## 3. Open: other findings

- [x] **`VIMA_Cache_Avg._Fetch_Latency` reports 1** while fetches take thousands of cycles. The fetch and writeback counters were never initialised (~1e14 / ~1e14), and the totals read only way 0 of each set. Fixed in 4b075eb; region 2 now reports 6,519 cycles per fetch.
- [x] **VIMA results are never written back to DRAM.** Fixed in 2ff41c6: results stay dirty and are written back on eviction; lookups match the vector a line holds or is about to hold, and busy lines are not evicted. VIMA scans (100k rows): 2.09–3.24 M cycles, 12–27% faster; vecsum 12% slower (same request count, writes instead of reads).

  Original finding: Found with the fixed stats: `VIMA_Cache_Writebacks` is 0. At completion the destination is marked dirty and set to WAIT (it fetches the old 8 KB), and `fetch()` sets `dirty = false` when it finishes (`vima_vector.cpp:66`), so the result is marked clean. Fix together with "destinations are fetched before being overwritten" (section 2).
- [ ] **Gathers fetch the wrong lines.** The tracer records `fgtrs` as (base, indices, destination), and the controller fetches the source as 8 KB contiguous from `base`, forced to miss. A real 2048-element gather touches up to 2048 lines spread over the column: OrCS undercounts gather traffic up to 16× and fetches the wrong addresses. `vima_vector_t::gather` (random addresses) is never set, and random addresses would not be better. Decide: record the gathered addresses in the trace (tracer + trace format + controller), or another model. Affects the `vima_selvec_*` scans.
- [ ] **Unaligned operands: small effect today, two latent issues.** In the large-input scan traces 51–89% of vectors are unaligned (e.g. 135 of 186 in `vima_bitmap_reeval`). Measured over all 8 VIMA scan traces: no two different vectors share an 8 KB-aligned block, so no false hits yet. Latent issues:
  - The VIMA cache identifies a vector by its 8 KB-aligned block (tag and index drop the low 13 bits), so two unaligned vectors starting in the same block would be treated as one (false hit). `store_hash` aliases the same way (only over-serializes).
  - `read1_unbalanced`/`read2_unbalanced` are never assigned, so `VIMA_UNBALANCED = 1` does nothing.
  In DRAM, an unaligned vector spans 2 rows and 2 channels; one line request crosses the row boundary and becomes 2 sub-requests.
- [x] **VIMA instructions have no memory dependencies.** Fixed following Santos (2022): VIMA executes and commits strictly in order (Sec. 4.8–4.10), and the programmer orders CPU accesses to VIMA data (Sec. 4.4).
  - 8c466ed: VIMA instructions are sent to the controller in program order (the traces were already in order; no cycle change).
  - e05bfb3: VIMA entries skip memory disambiguation (no slot-0 collision in our traces; no cycle change).
  - c900f25, 8c487ea: `_vim_fence()` writes `$5` (`SYNC_VIMA_FENCE`); OrCS drains the core there. The scans fence before every CPU read of a VIMA result. VIMA scans: −1% to +3% cycles, 1.4–1.9 M cycles of drain stalls (the ROB already made the CPU wait).

  Original finding: The VIMA MOB line never sets `memory_address` (it stays 0 after `package_clean`), and `make_memory_dependences` only adds READ and WRITE operations to the hashes. So:
  - VIMA instructions have no RAW/WAR/WAW order among themselves in the processor; they are sent whenever ready. In region 2, instruction 69 overwrote the sources of the older instruction 46 before 46 ran (WAR). The controller's `store_hash` only orders by arrival.
  - CPU loads of a VIMA result (e.g. `fcums` → `result[0]` in the scans) do not wait for the VIMA write, so dependent CPU work starts early and the VIMA round-trip is not timed.
  - Every VIMA instruction checks hash bucket 0 (address 0) and can pick up spurious dependencies on unrelated loads and stores there.
  Decide: track VIMA operands (3 × 8 KB ranges) in disambiguation, or send VIMA instructions to the controller in program order and make CPU loads wait on pending VIMA writes.

## 4. Found while investigating section 3

- [x] **`fmovs` (broadcast) fetches its destination.** Fixed in e429a43 (tracer): broadcasts are recorded as write-only. VIMA scans (100k rows): 7–13 fewer 8 KB fetches each, cycles within ±2%. Also 2dae364: the tracer now rebuilds when its `#include`d sources change.

  Original finding: The tracer records 1-operand instructions as read and written at the destination (`hmc_write_memory_1param`), so OrCS sees an in-place operation and fetches 8 KB before overwriting it (region 1: 128 requests for one `fmovs`). The destination of a broadcast should be write-only.
- [ ] **No coherence between the VIMA cache and the CPU caches.** In selvec, the CPU writes the row ids into its caches and VIMA reads them from DRAM; CPU accesses to VIMA results also go through the CPU caches. Santos (2022) assumes VIMA data lives in a non-cacheable region (Sec. 4.4); OrCS does not model it.

- [ ] **Every DRAM request becomes row-sized sub-requests.** `requestDRAM` splits any request into `BANK_ROW_BUFFER_SIZE` (8 KB) sub-requests, and the channel is selected by the bits just above the row (`channel_bits_shift = log2(BANK_ROW_BUFFER_SIZE)`). A vector fetch sends 128 line requests that all map to one channel and one row. Likely cause of "8 KB fetches are serialized" (section 2). Check what each sub-request costs before changing it.
