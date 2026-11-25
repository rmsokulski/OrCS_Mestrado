# OrCS cache write path: open items and known bugs

Nov 24, 2026 · @Rodrigo

## Summary

The two known write bugs don't explain the 0 LLC writebacks on `ompsync`. The LLC (16 MB) is larger than everything the kernel touches (≈ 12.6 MB), so it never evicts. They do lose writebacks in other kernels, though. 94% of stores are merged into a pending request by `isIn` and never applied. When the pending request is a read, as in `a[i] += …`, the line is installed clean and its writeback never happens.

| Measure (`ompsync`, N = 1, `skylakeServer`) | Value |
| --- | --- |
| Stores committed | 1,048,851 |
| Stores counted as L1D writes | 65,656 (6%) |
| Stores merged by `isIn` (`Already there (WRITE)`) | 983,198 (94%) |
| L1D / L2 / LLC writebacks | 130,624 / 114,782 / 0 |
| LLC installs into an empty slot | 196,807 of 196,807 (no LRU eviction ever) |
| Times the dirty bit was set on a valid LLC line | 245,406 |

The last two rows come from a throwaway instrumented build, not from the repo. `Cache_Eviction` counts every line installed, not real evictions.

## Known bugs

- [x] **A write merged into a pending read is never applied.** `cache_manager_t::isIn` (cache\_manager.cpp:358) attaches a new request to any pending request for the same line and type, and deletes it. When the pending request finishes, `install()` switches on that request's `memory_operation`. A write merged into a READ therefore never calls `data_cache[0].write()`: the line isn't marked dirty, and the write isn't counted.

  Effect: in read-modify-write code the line stays clean, so its writeback (and DRAM write) is lost. Write statistics count only the first store to each line.

  Fixed in f6fbad2 and 4bbb834; see the plan items below.
- [x] **A write-allocate write is not accounted after the line is loaded.** On a write miss, `cache_search` sets `is_load_from_write` (cache\_manager.cpp:1015), and `requestDRAM` turns the DRAM subrequests into READs (memory\_controller.cpp:380). The original request should still apply its write when the line arrives. Needs a test that tells a miss filled from DRAM apart from one filled by L2/LLC.

  On `ompsync` the L1D counts 65,656 writes with 89 hits, about one write miss per line of `c`. So the miss path does mark lines dirty here; the bug may be limited to specific paths.

  Resolved: the write itself was applied correctly. The bug was the accounting below the L1, which counted the line fetch as a write. Fixed in d281292; see the plan item.

**Plan for the known bugs** (agreed 2026-11-24):

- [x] **`isIn` merges requests from different cores.** It compares only the line address and type, and `requests` is shared. A request from core 1 that merges into core 0's miss completes without installing the line in core 1's L1/L2, and takes no entry in core 1's MSHRs. Fix: merge only within the same core. After that, two cores missing the same line both go to DRAM; a merge at the shared LLC's MSHR is a later change.

  Done in 4bbb834. `ompsync` N = 1 is identical. At N = 4 the cross-core merges were instruction fetches: cores 1–3 now fetch through their own L1I (≈ 690k accesses each instead of 77k–144k, and 103 misses each = the code lines). Cycles +1.8%, DRAM requests +169 (two cores missing the same line both go to DRAM). `CACHE_CHECK` passes. Still open: merging at the shared LLC's MSHR.
- [x] **Merged writes: apply and count them at the last moment.** `isIn` only increments `merged_writes` on the pending line request. `finishRequest`, just before it removes the request from `requests`, marks the core's L1 line dirty (or the next level that still holds it) and adds `merged_writes` to L1 `Cache_Write` and a new `Cache_Write_Merged`. Doing it at hit or install time would lose writes merged during the hit latency, because a hit stays in `requests` until `readyAt`.

  Done in f6fbad2, with `cache_t::mark_dirty()`. New statistics (printed when nonzero): `Cache_Write_Merged`, and `Cache_Write_Merged_Missing` for a line that left the L1 before `finishRequest`, which was 0 in every run. A merged write completes one L1 latency after the line, like a write hit (`updateClients(stallTime)`). That latency doesn't change cycles, because OrCS retires a store when it is sent; it would matter if store completion ever gates something, for example fences.
- [x] **Write-allocate below L1 is a fetch.** `cache_search` at levels > 0 counts a WRITE request as a READ (read hit/miss and read latency), and `install()` stops calling `add_cache_write()` on L2/LLC. The write is counted only at L1, and below L1 only writebacks are writes. `is_load_from_write` stays, for the DRAM subrequests.

  Done in d281292 with `stat_operation()`: statistics only, and the request stays a WRITE. On `ompsync` N = 1, L2/LLC `Cache_Read` = old reads + writes (262,224), `Cache_Write` is 0 below the L1, and the read latency averages include the fetches (L2 119 → 122, LLC 85 → 88). Cycles unchanged.
- [x] **`MAX_PARALLEL_REQUESTS_CORE` / `PARALLEL_LIM_ACTIVE` are read but never enforced**, and `searchData` always returns `true`, so the processor's "limit reached" branch never runs. Today the only per-core limit is the private L1/L2 MSHRs (20 and 40 entries). Decide whether to enforce the setting or remove it.

  Decision: not enforced, because the MSHRs already set the limit. Removed in 6a1db5a (`refactor(processor)`): both settings (members, reads, printed values, and their lines in 22 config files) and the never-printed `times_reach_parallel_requests_*` counters. Output identical.

## Open items

- [x] **On any LLC eviction, invalidate the line's copies in every core.** A dirty LLC eviction clears only the requesting core's L1/L2 copies (`line_ptr_caches[request->processor_id][*]`, cache.cpp:288). Other cores keep copies of a line that has left the LLC, and their pointers still aim at the reused slot. Fix: loop over every core, and reset the victim's `line_ptr_caches` before reuse.

  Fixed in ca05ec8 by `cache_t::evict()`, which handles every core and level. New optional setting `INCLUSION = "INCLUSIVE" | "NON_INCLUSIVE"` (LLC only, default inclusive), new statistic `N_Cache_Back_Invalidation`, and a `CACHE_CHECK` scan at the end of the run. 48 MB `ompsync` at N = 1 and N = 4: 0 bad pointers and 0 lines missing from the LLC. With a 2 MB LLC at N = 4, inclusive shows 230,440 LLC back-invalidations and non-inclusive none. Open: the non-inclusive run is 6% slower there (6.21M vs 5.87M cycles), cause not checked yet.
- [x] **A clean eviction clears nothing.** At any level, an evicted line that isn't dirty skips `writeBack`, so its upper-level copies stay valid and their pointers go stale. This happens even on a single core.

  Fixed in ca05ec8: `evict()` runs for clean victims too, and `unlink_line()` clears every pointer to and from the victim.
- [x] **`copyLevels` keeps only the L1's dirty bit.** When a dirty L2 line is evicted while an L1 copy exists, `copyLevels(line, 0, 2)` copies the L1's dirty bit to the LLC and ignores the L2's own (cache.cpp:252). Never seen on `ompsync` (0 cases), but it would drop a dirty L2 line whose L1 copy is clean.

  Fixed in ca05ec8: `copyLevels` was removed, and a dirty upper copy is merged into the victim before its dirty data moves down. Related, 25e363e: `returnLine` installs upper copies clean. L1 writebacks on `ompsync` N = 1 went from 130,624 to 65,572, close to the 65,536 written lines, with identical cycles.
- [x] **Rename or fix `Cache_Eviction`.** It counts every line installed, so it equals the miss count, not the real evictions.

  Fixed in eaa1015. `Cache_Eviction` now counts valid lines replaced by LRU, and the new `Cache_Install` keeps the old meaning; the unprinted `change_line` was removed. On `ompsync` N = 1, installs minus evictions equal each cache's line count: L1D 262,224 − 261,712 = 512, L2 262,371 − 245,987 = 16,384, and the LLC has 0 evictions.
- [ ] **Explain why non-inclusive mode is 6% slower.** `ompsync` at N = 4 with a scratch 2 MB LLC: 6,209,484 cycles non-inclusive against 5,866,764 inclusive. Non-inclusive keeps 100,388 valid lines at the end against 67,640, and sends dirty L2 lines straight to DRAM (16,520 L2 writebacks, 0 from the LLC). Compare L1/L2/LLC hits and DRAM requests between the two runs before trusting non-inclusive results.
- [x] **Merge misses to the same line at the shared LLC's MSHR.** Since 4bbb834, two cores that miss the same line both go to DRAM (+169 DRAM requests on `ompsync` N = 4). In design: a cross-core version of `isIn`, checked before `requestDRAM`.

  Done in 9a8d7ca. `merge_at_llc()` runs before the LLC lookup reserves an MSHR entry and before the MSHR-full check. A merged miss counts as an LLC access and miss, plus the new `Cache_Merged_Miss`, takes no LLC entry, and makes no DRAM access. `release_llc_waiters()` fills the waiter's L2/L1 from the LLC once the owner's line is installed, and the waiter completes one LLC latency later. `ompsync` N = 1 is identical. At N = 4, DRAM requests go from 197,323 to 197,154 (the value before 4bbb834), with 211 merged misses and −1.8% cycles. `CACHE_CHECK` also reports 0 MSHR entries left at the end. The 48 MB kernel at N = 4 is timing-sensitive: its DRAM requests move by ±3% between unrelated commits, because phase 2 reads the neighbor core's slice, which may or may not still be in the LLC. Only 151 of the previous build's DRAM requests there were duplicates.
- [ ] **Decide on coherence.** A store on one core doesn't invalidate other cores' copies. It needs the eviction fixes above first.

  Next session. Start with a minimal write-invalidate: on a write, remove the other cores' private copies, using the inclusive LLC line's per-core rows as the directory and `evict()`-style removal. Decide the invalidation latency first. Forwarding from a modified copy and full MESI come later.

Already fixed: 7459aa4 (the LLC writeback used the incoming address instead of the victim's) and d1f856a (shared caches printed once, MPKI with a shared LLC).

## Verification

- [x] **A kernel that forces LLC evictions:** an array larger than the LLC (for example 32 MB against 16 MB), written once. It should show LLC writebacks of about (footprint − LLC) / 64 B lines, and DRAM write requests at the victims' addresses.

  `ompsync` at 48 MB (`N = 1 << 22`, scratch build): LLC writebacks 246,192 at N = 1 and 230,327 at N = 4, after ca05ec8.
- [x] **A read-modify-write kernel** (`a[i] += 1` over an array larger than the LLC). Before the `isIn` fix: no LLC writebacks, and L1D writes ≈ stores / 16. After: every line dirty, and writebacks ≈ lines evicted.

  Ran it (32 MB, scratch kernel). The expectation above was wrong: writebacks were already right before the fix (LLC 262,147), because about 3 of each line's 16 stores arrive after the fill and hit the L1, which marks the line dirty. After f6fbad2, cycles and writebacks are identical, and L1 `Cache_Write` goes from 1,572,865 to 8,388,609 (= committed stores, with 6,815,744 merged).
- [ ] **Write-allocate:** count, per fill source (L2, LLC, DRAM), the write misses that end with the L1 line dirty. Every one should.
- [x] **Store accounting:** L1D writes plus merged writes = committed stores, with merged writes applied when their request completes.

  RMW 32 MB: `Write_Hit` 1,572,864 + `Write_Miss` 1 + `Write_Merged` 6,815,744 = 8,388,609 = `Total_Store_Requests`.
- [x] **Multi-core inclusion:** after the eviction fixes, every valid L1/L2 line in any core is also in the LLC. Add a check under a debug flag that scans the caches at the end of the run.

  `CACHE_CHECK` (defines.hpp, ca05ec8): 0 bad pointers and 0 upper-level lines missing from the LLC, on 48 MB `ompsync` N = 1 and N = 4, on a 2 MB LLC at N = 4, and after 4bbb834.
- [x] **No regression:** `ompsync` N = 1 and N = 4 results unchanged where no eviction or merge is involved.

  After each commit, `ompsync` N = 1 and N = 4 were compared with the previous build. Changes appeared only where expected: writeback counters (25e363e), `Write_Merged` (f6fbad2), and N = 4 cross-core merges (4bbb834).
