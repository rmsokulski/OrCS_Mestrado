# Graph Report - OrCS_Mestrado  (2026-11-23)

## Corpus Check
- 117 files · ~361,082 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 1859 nodes · 2959 edges · 85 communities (73 shown, 10 thin omitted)
- Extraction: 95% EXTRACTED · 5% INFERRED · 0% AMBIGUOUS · INFERRED: 152 edges (avg confidence: 0.84)
- Token cost: 82,117 input · 0 output

## Community Hubs (Navigation)
- Processor Core Types
- DRAM Controller Header
- DRAM Channel Header
- Cache Manager Header
- Cache Header
- Memory Package
- Simulation Engine
- Utilities
- PinPlay Debugger Shell
- HIVE Controller
- PinPlay Tracepoints
- VIMA Controller Header
- Simulator Includes
- Trace Reader
- Circular Buffer
- Memory Disambiguation
- Memory Order Buffer
- Processor Pipeline Stages
- Branch Predictor
- Debugger Interpreter
- Memory Request Client
- VIMA Vector Unit
- x86 Intrinsics Tracing
- Uop Package
- PinPlay Instrumentation
- Reorder Buffer Line
- Branch Profiler Pintool
- Sinuca Tracer
- Memory Package Impl
- Debugger Help System
- BBL Tracer
- Tracer Shared Headers
- RVV Memory Access Docs
- Prefetcher
- Instruction Set Config
- Set-Associative Template
- Piecewise Predictor
- Instruction Type Counter
- Instruction Count Checks
- Cache Manager Impl
- Benchmark Lists
- ROB and Functional Units
- Cache Impl
- Stride Prefetcher
- Cache Line
- MOB Execute Stage
- Tracer Thread Control
- OMP Tracer Pintool
- Trace Generation Docs
- DRAM Channel Impl
- VIMA Controller Impl
- Uop Package Impl
- x86 Opcode Conversion
- Library Call Tracer
- Two-Bit Predictor
- DRAM Controller Impl
- Stride Table
- Cache Search Flow
- Tracer Thread Data
- Enum Helpers
- BTB Line
- Opcode Package
- Predictor Interface
- Config Loading
- Pintool Thread Counters
- Cache Instantiation
- Cache Set
- Simulator Main
- SPEC2006 Tracer Script
- BTB
- Register Info
- Opcode to Uop
- Unknown Memory Size
- Memory Access Record
- BOOM v3 Parameters
- Statistics Stub A
- Autofix Script
- Tracer Install Script
- Uop Memory Operation
- VIMA Trace Script
- x86 Trace Script
- Word Splitting
- Statistics Stub B

## God Nodes (most connected - your core abstractions)
1. `processor_t` - 228 edges
2. `DR_SHELL` - 120 edges
3. `memory_package_t` - 83 edges
4. `cache_manager_t` - 78 edges
5. `memory_channel_t` - 69 edges
6. `memory_controller_t` - 69 edges
7. `cache_t` - 63 edges
8. `vima_controller_t` - 54 edges
9. `get_enum_memory_operation_char()` - 46 edges
10. `orcs_engine_t` - 44 edges

## Surprising Connections (you probably didn't know these)
- `statistics` --calls--> `get_enum_cache_type_char()`  [INFERRED]
  cache/cache.hpp → utils/enumerations.cpp
- `statistics` --calls--> `get_enum_memory_operation_char()`  [INFERRED]
  cache/cache.hpp → utils/enumerations.cpp
- `print_requests` --calls--> `get_enum_memory_operation_char()`  [INFERRED]
  cache/cache_manager.hpp → utils/enumerations.cpp
- `isIn` --calls--> `get_enum_memory_operation_char()`  [INFERRED]
  cache/cache_manager.hpp → utils/enumerations.cpp
- `cache_search` --calls--> `get_enum_memory_operation_char()`  [INFERRED]
  cache/cache_manager.hpp → utils/enumerations.cpp

## Import Cycles
- None detected.

## Hyperedges (group relationships)
- **SPEC CPU benchmark suites** — trace_generator_scripts_apps_benchmarks_apps_spec_cpu2000_list, trace_generator_scripts_apps_benchmarks_apps_spec_cpu2006_list, trace_generator_scripts_apps_benchmarks_apps_spec_cpu2017_list [INFERRED 0.85]
- **AVX vector-width benchmark sets** — trace_generator_scripts_apps_benchmarks_apps_avx128_list, trace_generator_scripts_apps_benchmarks_apps_avx256_list, trace_generator_scripts_apps_benchmarks_apps_avx512_list [INFERRED 0.85]
- **Trace generation toolchain** — docs_readme_orcs_tracer_sinuca_tracer, docs_readme_orcs_tracer_pin_pinplay, docs_gerando_tra_os_com_o_sinuca_pinpoints_sinuca_pinpoints, docs_gerando_tra_os_com_o_sinuca_pinpoints_simpoints, docs_gerando_tra_os_do_spec_2006_spec_cpu_2006 [INFERRED 0.85]
- **In-order pipeline modes** — docs_altera__es_orcs_in_order_in_order_mode, docs_altera__es_orcs_superscalar_inorder_superscalar_in_order, docs_altera__es_uops_dependencies_fix_uops_link_register, docs_altera__es_orcs_in_order_processor_t [INFERRED 0.85]
- **RVV memory access handling** — docs_altera__es_altera__es_no_tratamento_de_acessos___mem_ria_para_instru__es_rvv_no_spike___orcs_variable_mem_accesses, docs_altera__es_orcs_rvv_lds_sts_generating_many_uops_mob_split_uops, docs_altera__es_memory_accesses_fusing_memory_access_fusing, docs_altera__es_orcs_rvv_lds_sts_generating_many_uops_processor_decode [INFERRED 0.75]

## Communities (85 total, 10 thin omitted)

### Community 0 - "Processor Core Types"
Cohesion: 0.01
Nodes (180): circular_buffer_t, container_ptr_reorder_buffer_line_t, disambiguation_method_t, float_t, memory_order_buffer_line_t, opcode_package_t, uop_package_t, vector (+172 more)

### Community 1 - "DRAM Controller Header"
Cohesion: 0.03
Nodes (56): vector, memory_controller_t, BANK, bank_bits_mask, bank_bits_shift, BANK_ROW_BUFFER_SIZE, BURST_WIDTH, cache_line_latency_burst (+48 more)

### Community 2 - "DRAM Channel Header"
Cohesion: 0.03
Nodes (55): memory_controller_command_t, request_priority_t, vector, memory_channel_t, BANK, bank_bits_mask, bank_bits_shift, BANK_BUFFER_SIZE (+47 more)

### Community 3 - "Cache Manager Header"
Cohesion: 0.04
Nodes (55): cache_manager_t, CACHE_MANAGER_DEBUG, data_cache, DATA_LEVELS, dataSearch, DCACHE_AMOUNT, i, ICACHE_AMOUNT (+47 more)

### Community 4 - "Cache Header"
Cohesion: 0.04
Nodes (47): cache_t, associativity, cache_access, cache_count_per_type, cache_eviction, cache_hit, cache_hit_per_type, cache_inst (+39 more)

### Community 5 - "Memory Package"
Cohesion: 0.04
Nodes (49): cacheId_t, memory_operation_t, package_state_t, vector, vima_vector_t, memory_package_t, born_cycle, cache_latency (+41 more)

### Community 6 - "Simulation Engine"
Cohesion: 0.05
Nodes (42): branch_predictor_t, cache_manager_t, configure_t, hive_controller_t, instruction_set_t, memory_controller_t, vector, orcs_engine_t (+34 more)

### Community 7 - "Utilities"
Cohesion: 0.09
Nodes (42): TYPE, FILE, hash_function_t, reorder_buffer_line_t, string, pointer_rob_to_string(), template_allocate_array(), template_allocate_initialize_array() (+34 more)

### Community 8 - "PinPlay Debugger Shell"
Cohesion: 0.07
Nodes (34): CALL_ORDER, CHAR, LOCALFUN, ADDRINT, BBL, BOOL, CONTEXT, INS (+26 more)

### Community 9 - "HIVE Controller"
Cohesion: 0.07
Nodes (41): FILE, hive_controller_t, allocate, check_sub_requests, check_transmit, check_wait, clock, CORE_TO_BUS_CLOCK_RATIO (+33 more)

### Community 10 - "PinPlay Tracepoints"
Cohesion: 0.07
Nodes (26): EVENTS, HELP_CATEGORIES, ADDRINT, PIN_LOCK, UINT32, DR_SHELL, _clientArgs, _events (+18 more)

### Community 11 - "VIMA Controller Header"
Cohesion: 0.05
Nodes (42): vima_vector_t, vima_controller_t, bits_shift, cache, cache_accesses, cache_hits, cache_misses, cache_reads (+34 more)

### Community 12 - "Simulator Includes"
Cohesion: 0.05
Nodes (39): branch_predictor_t, btb_line_t, btb_t, cache_manager_t, cache_t, cacheSet_t, circular_buffer_t, configure_t (+31 more)

### Community 13 - "Trace Reader"
Cohesion: 0.09
Nodes (35): branch_t, FILE, opcode_package_t, gzFile, opcode_package_t, trace_reader_t, address_translation, allocate (+27 more)

### Community 14 - "Circular Buffer"
Cohesion: 0.07
Nodes (24): CB_TYPE, circular_buffer_t, allocate, back, beg_index, capacity, circular_buffer_t<CB_TYPE>::back(), circular_buffer_t<CB_TYPE>::front() (+16 more)

### Community 15 - "Memory Disambiguation"
Cohesion: 0.07
Nodes (31): desambiguation_t, allocate, make_memory_dependences, reset_statistics, solve_memory_dependences, statistics, FILE, memory_order_buffer_line_t (+23 more)

### Community 16 - "Memory Order Buffer"
Cohesion: 0.06
Nodes (33): memory_order_buffer_line_t, package_state_t, reorder_buffer_line_t, memory_order_buffer_line_t, core_generate_miss, cycle_send_request, cycle_sent_to_DRAM, find_free (+25 more)

### Community 17 - "Processor Pipeline Stages"
Cohesion: 0.10
Nodes (33): cacheId_t, FILE, opcode_package_t, Setting, allocate, clock, commit, decode (+25 more)

### Community 18 - "Branch Predictor"
Cohesion: 0.08
Nodes (31): branch_prediction_method_t, branch_predictor_t, allocate, BRANCH_PREDICTION_METHOD, branches, branchNotTaken, branchNotTakenMiss, branchPredictor (+23 more)

### Community 19 - "Debugger Interpreter"
Cohesion: 0.24
Nodes (4): ETYPE, T, string, ParseNumber()

### Community 20 - "Memory Request Client"
Cohesion: 0.06
Nodes (34): memory_operation_t, package_state_t, memory_request_client_t, memory_operation, readyAt, status, waiting_DRAM, instruction_operation_t (+26 more)

### Community 21 - "VIMA Vector Unit"
Cohesion: 0.07
Nodes (31): package_state_t, vima_vector_t, address, allocate, clock, column, dirty, fetch (+23 more)

### Community 22 - "x86 Intrinsics Tracing"
Cohesion: 0.16
Nodes (32): INT, RTN, arch_x86_set_data_instr(), arch_x86_trace_instruction(), ADDRINT, string, THREADID, UINT32 (+24 more)

### Community 23 - "Uop Package"
Cohesion: 0.06
Nodes (32): functional_unit_t, instruction_operation_t, package_state_t, uop_package_t, born_cycle, functional_unit, hive_read1, hive_read2 (+24 more)

### Community 24 - "PinPlay Instrumentation"
Cohesion: 0.18
Nodes (19): BBL, INS, IPOINT, TRACE, VOID, DR_DEBUGGER_SHELL::OutputTargetRemoteInfo(), DR_DEBUGGER_SHELL::SetGdbMode(), EVENT (+11 more)

### Community 25 - "Reorder Buffer Line"
Cohesion: 0.09
Nodes (24): reorder_buffer_line_t, string, memory_order_buffer_line_t, processor_stage_t, uop_package_t, reorder_buffer_line_t, committed, content_to_string (+16 more)

### Community 26 - "Branch Profiler Pintool"
Cohesion: 0.10
Nodes (24): CONTEXT, INT32, THREADID, TRACE, UINT32, UINT64, UINT8, VOID (+16 more)

### Community 27 - "Sinuca Tracer"
Cohesion: 0.16
Nodes (23): EVENT_TYPE, control_instrumented_bbl(), count_instructions(), ADDRINT, BOOL, CONTEXT, IMG, INT32 (+15 more)

### Community 28 - "Memory Package Impl"
Cohesion: 0.17
Nodes (22): addRequest, print_hive_instructions, updatePackageDRAMFetch, updatePackageDRAMReady, updatePackageFree, updatePackageHive, updatePackageReady, updatePackageTransmit (+14 more)

### Community 29 - "Debugger Help System"
Cohesion: 0.10
Nodes (12): FORMAT_PAIRS, HELPS, iterator, HELP, HELP_CATEGORY, _description, _formattedHelp, _helpStrings (+4 more)

### Community 30 - "BBL Tracer"
Cohesion: 0.13
Nodes (13): bbl_executed(), BlocksLoaded, BBLs, opcode_package_t, TRACE, vector, Fini(), size (+5 more)

### Community 31 - "Tracer Shared Headers"
Cohesion: 0.20
Nodes (7): vector, string, ascii_to_hex(), hex_to_dec(), main(), Usage(), xed_uint8_t

### Community 32 - "RVV Memory Access Docs"
Cohesion: 0.14
Nodes (21): Alterações no tratamento de acessos à memória para instruções RVV no SPIKE + OrCS, orcs_tracer_t::fill_instruction_info, SPIKE RISC-V simulator, trace_reader_t::trace_string_to_opcode, Variable number of RVV memory accesses (-1 + $<n> annotation), Memory accesses fusing, get_instruction_loads_stride, Memory accesses fusing (+13 more)

### Community 33 - "Prefetcher"
Cohesion: 0.12
Nodes (17): cache_t, FILE, memory_order_buffer_line_t, vector, prefetcher_t, allocate, latePrefetches, PARALLEL_PREFETCH (+9 more)

### Community 34 - "Instruction Set Config"
Cohesion: 0.11
Nodes (17): fu_info_t, size, wait_next, string, vector, instruction_set_t, allocate, fu_id (+9 more)

### Community 35 - "Set-Associative Template"
Cohesion: 0.12
Nodes (14): C_TYPE, set_associative_t, allocate, associativity, set_associative_t<C_TYPE>::map_set(), set_associative_t<C_TYPE>::operator[](), get_associativity, get_num_sets (+6 more)

### Community 36 - "Piecewise Predictor"
Cohesion: 0.13
Nodes (15): taken_t, predictor_t, piecewise_t, allocate, GA, GHR, H, M (+7 more)

### Community 37 - "Instruction Type Counter"
Cohesion: 0.15
Nodes (16): CONTEXT, INT32, THREADID, TRACE, UINT64, UINT8, VOID, docount() (+8 more)

### Community 38 - "Instruction Count Checks"
Cohesion: 0.14
Nodes (11): AT_ICOUNT, _icount, _tid, AT_MCOUNT, _mcount, _tid, UINT64, THREAD_DATA (+3 more)

### Community 39 - "Cache Manager Impl"
Cohesion: 0.20
Nodes (15): available, check_cache, clock, generateIndexArray, install, installCacheLines, isBusy, isIn (+7 more)

### Community 40 - "Benchmark Lists"
Cohesion: 0.15
Nodes (17): Variable Cache Line Size Support, libconfig, OrCS Simulator, zlib, AVX 128 Apps Benchmark List, AVX 256 Apps Benchmark List, AVX 512 Apps Benchmark List, HIVE Apps Benchmark List (+9 more)

### Community 41 - "ROB and Functional Units"
Cohesion: 0.12
Nodes (13): functional_unit_t, dispatch_cnt, id, size, slot, wait_next, reorder_buffer_line_t, ROB_t (+5 more)

### Community 42 - "Cache Impl"
Cohesion: 0.22
Nodes (14): allocate, copyLevels, copyNextLevels, installLine, peek, read, reset_statistics, returnLine (+6 more)

### Community 43 - "Stride Prefetcher"
Cohesion: 0.20
Nodes (13): status_stride_prefetcher_t, stride_prefetcher_t, allocate, DISTANCE, installStride, searchLRU, searchPattern, statistics (+5 more)

### Community 44 - "Cache Line"
Cohesion: 0.14
Nodes (11): line_t, dirty, line_ptr_caches, lru, NUMBER_OF_PROCESSORS, POINTER_LEVELS, prefetched, print_line (+3 more)

### Community 45 - "MOB Execute Stage"
Cohesion: 0.20
Nodes (15): memory_order_buffer_line_t, reorder_buffer_line_t, clean_mob_hive, clean_mob_read, clean_mob_vima, execute, get_next_op_hive, get_next_op_load (+7 more)

### Community 46 - "Tracer Thread Control"
Cohesion: 0.21
Nodes (5): BOOL, CONTEXT, INT32, THREADID, DR_DEBUGGER_SHELL::CreatePinPlayShell()

### Community 47 - "OMP Tracer Pintool"
Cohesion: 0.18
Nodes (14): AFUNPTR, ADDRINT, CONTEXT, IMG, INT32, THREADID, TRACE, VOID (+6 more)

### Community 48 - "Trace Generation Docs"
Cohesion: 0.26
Nodes (13): Direct tracer, Gerando traços com o Sinuca PinPoints, SimPoints, Sinuca PinPoints, Gerando traços do SPEC 2006, pinplay.cfg slice_size, SPEC CPU 2006, Manual atualização pinpoints 3.11 (+5 more)

### Community 49 - "DRAM Channel Impl"
Cohesion: 0.23
Nodes (9): memory_controller_command_t, addRequest, allocate, clock, findNext, findNextRead, findNextWrite, get_minimum_latency (+1 more)

### Community 50 - "VIMA Controller Impl"
Cohesion: 0.23
Nodes (9): cache_status_t, vima_vector_t, addRequest, allocate, check_completion, clock, instruction_ready, reset_statistics (+1 more)

### Community 51 - "Uop Package Impl"
Cohesion: 0.18
Nodes (10): string, uop_package_t, content_to_string, content_to_string2, operator==, package_clean, updatePackageFree, updatePackageReady (+2 more)

### Community 52 - "x86 Opcode Conversion"
Cohesion: 0.18
Nodes (12): INS, opcode_package_t, string, gen_icode(), to_string(), x86_to_static(), opcode_package_t, opcode_to_trace_string() (+4 more)

### Community 53 - "Library Call Tracer"
Cohesion: 0.29
Nodes (10): called_rtn(), IMG, INT32, THREADID, VOID, Fini(), ImageLoad(), ImageUnload() (+2 more)

### Community 54 - "Two-Bit Predictor"
Cohesion: 0.22
Nodes (7): FILE, predictor_t, twoBit_t, allocate, predict, statistics, train

### Community 55 - "DRAM Controller Impl"
Cohesion: 0.22
Nodes (8): FILE, allocate, clock, isBusy, requestDRAM, reset_statistics, set_masks, statistics

### Community 56 - "Stride Table"
Cohesion: 0.22
Nodes (7): status_stride_prefetcher_t, stride_table_t, last_address, lru, status, stride, tag

### Community 57 - "Cache Search Flow"
Cohesion: 0.25
Nodes (9): cache_search, finishRequest, process, searchAddress, cache_status_t, cache_t, vector, cacheId_t (+1 more)

### Community 58 - "Tracer Thread Data"
Cohesion: 0.22
Nodes (9): gzFile, PIN_LOCK, UINT8, thread_data_t, dyn_lock, gzDynamicTraceFile, gzMemoryTraceFile, is_instrumented_bbl (+1 more)

### Community 59 - "Enum Helpers"
Cohesion: 0.22
Nodes (8): hash_function_t, memory_controller_command_t, request_priority_t, status_stride_prefetcher_t, get_enum_hash_function_char(), get_enum_memory_controller_command_char(), get_enum_request_priority_char(), get_enum_status_stride_prefetcher_char()

### Community 60 - "BTB Line"
Cohesion: 0.25
Nodes (7): btb_line_t, bht, lru, tag, targetAddress, typeBranch, validade

### Community 61 - "Opcode Package"
Cohesion: 0.32
Nodes (4): string, opcode_package_t::content_to_string(), opcode_package_t::content_to_string2(), opcode_package_t::log_registers()

### Community 63 - "Config Loading"
Cohesion: 0.33
Nodes (5): Config, configure_t, cfg, configure_t::configure_t(), loadConfig

### Community 64 - "Pintool Thread Counters"
Cohesion: 0.29
Nodes (7): PIN_LOCK, UINT64, UINT8, thread_data_t, counter, dyn_lock, pad

### Community 65 - "Cache Instantiation"
Cohesion: 0.60
Nodes (6): allocate, get_cache_info, get_cache_levels, instantiate_cache, cacheId_t, Setting

### Community 66 - "Cache Set"
Cohesion: 0.33
Nodes (3): cacheSet_t, lines, n_lines

### Community 68 - "Simulator Main"
Cohesion: 0.53
Nodes (5): string, display_use(), get_status_execution(), main(), process_argv()

### Community 70 - "BTB"
Cohesion: 0.40
Nodes (3): btb_t, btb_entry, btb_line_t

### Community 71 - "Register Info"
Cohesion: 0.40
Nodes (4): REG, REG_INFO, _name, _reg

### Community 72 - "Opcode to Uop"
Cohesion: 0.50
Nodes (4): functional_unit_t, instruction_operation_t, opcode_package_t, opcode_to_uop

### Community 73 - "Unknown Memory Size"
Cohesion: 0.67
Nodes (4): PIN_MULTI_MEM_ACCESS_INFO, VOID, touch_memory(), unknown_memory_size_f()

### Community 74 - "Memory Access Record"
Cohesion: 0.50
Nodes (4): MemoryAccess, addr, bbl, is_read

### Community 75 - "BOOM v3 Parameters"
Cohesion: 1.00
Nodes (3): BOOM v3 processor parameters, Boom V3 processor parameters, Next-line prefetcher

## Ambiguous Edges - Review These
- `SPEC OMP 2001 Benchmark List` → `SPEC OMP 2012 Benchmark List`  [AMBIGUOUS]
  trace_generator/scripts/apps_benchmarks/apps_spec_omp2012.txt · relation: semantically_similar_to

## Knowledge Gaps
- **906 isolated node(s):** `processor_id`, `BTB_ENTRIES`, `BTB_WAYS`, `BTB_MISS_PENALITY`, `MISSPREDICTION_PENALITY` (+901 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1147 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **10 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **What is the exact relationship between `SPEC OMP 2001 Benchmark List` and `SPEC OMP 2012 Benchmark List`?**
  _Edge tagged AMBIGUOUS (relation: semantically_similar_to) - confidence is low._
- **Why does `processor_t` connect `Processor Core Types` to `ROB and Functional Units`, `MOB Execute Stage`, `Memory Disambiguation`, `Processor Pipeline Stages`, `Memory Package Impl`?**
  _High betweenness centrality (0.150) - this node is a cross-community bridge._
- **Why does `DR_SHELL` connect `PinPlay Tracepoints` to `Instruction Count Checks`, `Register Info`, `PinPlay Debugger Shell`, `Tracer Thread Control`, `Word Splitting`, `Debugger Interpreter`, `PinPlay Instrumentation`, `Debugger Help System`?**
  _High betweenness centrality (0.145) - this node is a cross-community bridge._
- **Why does `memory_package_t` connect `Memory Package` to `DRAM Controller Header`, `DRAM Channel Header`, `Cache Manager Header`, `Cache Manager Impl`, `HIVE Controller`, `Cache Impl`, `VIMA Controller Header`, `DRAM Channel Impl`, `VIMA Controller Impl`, `Memory Request Client`, `VIMA Vector Unit`, `DRAM Controller Impl`, `Cache Search Flow`, `Memory Package Impl`?**
  _High betweenness centrality (0.135) - this node is a cross-community bridge._
- **What connects `processor_id`, `BTB_ENTRIES`, `BTB_WAYS` to the rest of the system?**
  _906 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Processor Core Types` be split into smaller, more focused modules?**
  _Cohesion score 0.011111111111111112 - nodes in this community are weakly interconnected._
- **Should `DRAM Controller Header` be split into smaller, more focused modules?**
  _Cohesion score 0.03278688524590164 - nodes in this community are weakly interconnected._