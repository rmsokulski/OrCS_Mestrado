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
- [ ] **Destinations are fetched before being overwritten.** At completion the destination line is set to WAIT (or TRANSMIT if dirty), so it fetches the old 8 KB from DRAM even when the instruction writes every element, and even on a hit. `check_completion()` has the "dirty → TRANSMIT, else WAIT" block twice in the old code; the second one applied on every path.
- [ ] **Writebacks to address 0.** Lines never filled are written back with `address = 0`: `set_next_address` changes `next_address`, but `writeback()` uses `address`, which is only set by `fetch()`. Each costs 5,000–10,000 cycles of DRAM traffic. Seen as `WRITEBACK of address 0` in the `VIMA_DEBUG` traces.
- [ ] **Reused vectors are fetched again.** With 8 lines, the broadcast `low`/`high` vectors are evicted between blocks and fetched again (e.g. 235167744 fetched three times in region 2). May be a valid effect of the cache size; check after the other fixes.
- [ ] **In-order entry costs some overlap.** Region 2 went from 180,836 to 203,108 cycles (+12%) after the deadlock fix: an instruction now waits until all its lines are free. Revisit only if it matters after the fixes above.

## 3. Open: other findings

- [ ] **`VIMA_Cache_Avg._Fetch_Latency` reports 1** while fetches take thousands of cycles. Check how `fetch_latency_total` and `fetch_count` are summed in `vima_controller_t::statistics`.
- [ ] **Gathers are traced as one 8 KB read at the base address** (`sinuca_tracer`, `fgtrs`), not the gathered addresses, so OrCS undercounts gather traffic. Affects the `vima_selvec_*` scans.
- [ ] **Unaligned operands not tested.** The scans pass `column + first_row`, so their vectors start mid-block; `vecsum` uses 8 KB-aligned arrays. With `VIMA_UNBALANCED = 0` the controller treats them as aligned. Check whether that hides traffic a real VIMA would have.
- [ ] **The processor sends VIMA instructions out of order.** It sends a VIMA instruction once its dependencies resolve, so in region 2 instruction 46 reached the controller after 69 and 74, which overwrite 46's sources. Entry in order now keeps the controller correct, but check whether the processor's ordering is intended.
