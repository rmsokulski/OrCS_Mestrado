# Graph Report - OrCS_Mestrado  (2026-11-24)

## Corpus Check
- 95 files · ~202,024 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 3270 nodes · 5015 edges · 220 communities (155 shown, 62 thin omitted)
- Extraction: 97% EXTRACTED · 3% INFERRED · 0% AMBIGUOUS · INFERRED: 142 edges (avg confidence: 0.84)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- Processor Core Types
- Util
- Drd Util
- Memory Package
- DRAM Controller Header
- DRAM Channel Header
- Cache Header
- Cache Manager Header
- Cmd Options
- Cmd Options 2
- Simulation Engine
- Regions
- Memory Order Buffer
- Correct Size Matrix
- Utils
- Pinplay Debugger Shell
- Pcregions
- Pinplay Debugger Shell 2
- Vima Controller
- Util 2
- Simulator Includes
- Util 3
- Circular Buffer
- Processor
- Branch Predictor
- Pinplay Debugger Shell 3
- Vima Vector
- Pinplay Debugger Shell 4
- Intrinsics Extension
- Disambiguation Hashed
- Pinpoints
- Uop Package
- Cmd Options 3
- Cache Manager Requests
- Hive Controller
- Opcode Package and Sync Type
- Sinuca Tracer
- Reorder Buffer Line
- Kit
- OrCS README Concepts
- Branch Profiler
- Record
- Replay
- Hive Controller 2
- Sde Pinpoints
- Linha
- Replay Dir
- Alterações No Tratamento De Acessos À Me
- Trace Reader
- Prefetcher
- Instruction Set
- Gdb Record
- Gdb Replay
- Sniper Pinpoints
- Set Associative
- Processor 2
- Util 4
- Direct Tracer
- Piecewise
- Cache Eviction and Install
- Simpoint
- Instruction Type Count
- Pinplay Debugger Shell 5
- Opcode Package
- Memory Request Client
- Memory Dependences
- Sinuca Pinpoints
- Profile
- Stride Prefetcher
- Cmd Options 4
- Pinplay Debugger Shell 6
- Sync Manager
- Log
- Make Simpoint Weights
- Phases
- Pinplay Debugger Shell 7
- Trace Reader Sync Markers
- Vima Controller 2
- Cbsp
- Config
- Pin
- Sim Run
- Gerando Traços Com O Sinuca Pinpoints
- Memory Channel
- Config 2
- Logger
- Sde Kit
- Sim Kit
- Conversions
- Cbsp Phases
- Config 3
- Sde Cbsp
- Sde Phases
- Brpred Metric
- Replayer
- Library Call
- Enumerations
- Twobit
- Memory Controller
- Stride Table
- Brpred Kit
- Brpred Pinpoints
- Config 4
- Config 5
- Pin 2
- Relog
- Sde Cmpsim Kit
- Uop Package 2
- Pin 3
- Sim Run Dir
- X86 Run
- Intrinsics Extension 2
- OpenMP Sync Test Kernel
- Btb Line
- Msg
- Phases 2
- Phases 3
- Phases 4
- Pin 4
- X86 Run Dir
- Predictor
- Cache Manager
- Pinplay Debugger Shell 8
- Brpred Replay Dir
- Brpred Replayer
- Rename
- Sde Cmpsim Metric
- Sde Cmpsim Replay Dir
- Sde Cmpsim Replayer
- Sinuca Replay Dir
- Sinuca Replayer
- Profile 2
- Config 6
- Simulator
- Config 7
- Phases 5
- Pin 5
- Pin 6
- Pin 7
- Pin 8
- Pin 9
- Sde Replayer
- Spec Cpu2006 Tracer
- Btb
- Uop Package 3
- Sde Log
- Sde Logger
- Sde Replay Dir
- Traceinfo
- Pinplay Debugger Shell 9
- Uop Package 4
- Cmd Options 5
- Cmd Options 6
- Sinuca Kit
- X86 Kit
- Direct Tracer 2
- Run Traces.Sh
- Trace Reader 2
- Cache
- Boom V3 Processor Parameters
- Autofix.Sh
- Install Tracer.Sh
- Uop Package 5
- Trace Creation Vima.Sh
- Trace Creation X86.Sh
- Cmd Options 7
- Cmd Options 8
- Cmd Options 9
- Cmd Options 10
- Cmd Options 11
- Cmd Options 12
- Cmd Options 13
- Cmd Options 14
- Cmd Options 15
- Cmd Options 16
- Cmd Options 17
- Phases 6
- Phases 7
- Phases 8
- Phases 9
- Phases 10
- Phases 11
- Phases 12
- Phases 13
- Pinplay Debugger Shell 10
- Trace Reader 3
- Misc
- Misc 2
- Misc 3
- Misc 4
- Misc 5
- Misc 6
- Misc 7
- Misc 8
- Misc 9
- Misc 10
- Misc 11
- Misc 12
- Misc 13
- Misc 14
- Misc 15
- Misc 16
- Misc 17
- Misc 18
- Misc 19
- Misc 20
- Misc 21
- Misc 22
- Misc 23
- Misc 24
- Misc 25
- Misc 26
- Misc 27
- Misc 28
- Misc 29
- Misc 30
- Misc 31

## God Nodes (most connected - your core abstractions)
1. `processor_t` - 232 edges
2. `DR_SHELL` - 120 edges
3. `memory_package_t` - 93 edges
4. `GetMethod()` - 86 edges
5. `cache_manager_t` - 82 edges
6. `memory_controller_t` - 69 edges
7. `memory_channel_t` - 69 edges
8. `cache_t` - 69 edges
9. `vima_controller_t` - 54 edges
10. `ensure_string()` - 47 edges

## Surprising Connections (you probably didn't know these)
- `addRequest` --calls--> `get_enum_memory_operation_char()`  [INFERRED]
  main_memory/memory_channel.hpp → utils/enumerations.cpp
- `clock` --calls--> `get_enum_memory_operation_char()`  [INFERRED]
  main_memory/memory_channel.hpp → utils/enumerations.cpp
- `clock` --calls--> `get_enum_memory_operation_char()`  [INFERRED]
  main_memory/memory_controller.hpp → utils/enumerations.cpp
- `requestDRAM` --calls--> `get_enum_memory_operation_char()`  [INFERRED]
  main_memory/memory_controller.hpp → utils/enumerations.cpp
- `statistics` --calls--> `get_enum_memory_operation_char()`  [INFERRED]
  main_memory/memory_controller.hpp → utils/enumerations.cpp

## Import Cycles
- None detected.

## Hyperedges (group relationships)
- **RVV memory access handling** — docs_altera__es_altera__es_no_tratamento_de_acessos___mem_ria_para_instru__es_rvv_no_spike___orcs_variable_mem_accesses, docs_altera__es_orcs_rvv_lds_sts_generating_many_uops_mob_split_uops, docs_altera__es_memory_accesses_fusing_doc, docs_altera__es_orcs_rvv_lds_sts_generating_many_uops_processor_decode [INFERRED 0.75]
- **AVX vector-width benchmark sets** — trace_generator_scripts_apps_benchmarks_apps_avx128_list, trace_generator_scripts_apps_benchmarks_apps_avx256_list, trace_generator_scripts_apps_benchmarks_apps_avx512_list [INFERRED 0.85]
- **In-order pipeline modes** — docs_altera__es_orcs_in_order_in_order_mode, docs_altera__es_orcs_superscalar_inorder_superscalar_in_order, docs_altera__es_uops_dependencies_fix_uops_link_register, docs_altera__es_orcs_in_order_processor_t [INFERRED 0.85]
- **SPEC CPU benchmark suites** — trace_generator_scripts_apps_benchmarks_apps_spec_cpu2000_list, trace_generator_scripts_apps_benchmarks_apps_spec_cpu2006_list, trace_generator_scripts_apps_benchmarks_apps_spec_cpu2017_list [INFERRED 0.85]
- **Trace generation toolchain** — docs_readme_orcs_tracer_sinuca_tracer, docs_readme_orcs_tracer_pin_pinplay, docs_gerando_tra_os_com_o_sinuca_pinpoints_sinuca_pinpoints, docs_gerando_tra_os_com_o_sinuca_pinpoints_simpoints, docs_gerando_tra_os_do_spec_2006_spec_cpu_2006 [INFERRED 0.85]
- **Multi-threaded tracing pipeline** — readme_multithreaded_tracing, readme_orcs_tracing_start_stop, readme_noclone3, readme_pin_3_11 [EXTRACTED 1.00]

## Communities (220 total, 62 thin omitted)

### Community 0 - "Processor Core Types"
Cohesion: 0.01
Nodes (180): container_ptr_reorder_buffer_line_t, desambiguation_t, disambiguation_method_t, float_t, opcode_package_t, sync_type_t, vector, processor_t (+172 more)

### Community 1 - "Util"
Cohesion: 0.02
Nodes (88): GetOptions(), Get users command line options/args and check to make sure they are correct.…, AddCfgFile(), AddCfgFileList(), AddCompressed(), AddGlobalFile(), AddMethodcbsp(), AddMt() (+80 more)

### Community 2 - "Drd Util"
Cohesion: 0.07
Nodes (31): Config, # TODO: Fix print so it prints CBSP info, not PP info, AddAdditionalOptions(), AddPinKnobsGDB(), debug(), DrDebugScriptCmd(), FinalizeGDB(), FinalizeNoGDB() (+23 more)

### Community 3 - "Memory Package"
Cohesion: 0.04
Nodes (63): cacheId_t, memory_operation_t, memory_request_client_t, vector, memory_package_t, born_cycle, cache_latency, clients (+55 more)

### Community 4 - "DRAM Controller Header"
Cohesion: 0.03
Nodes (56): vector, memory_controller_t, BANK, bank_bits_mask, bank_bits_shift, BANK_ROW_BUFFER_SIZE, BURST_WIDTH, cache_line_latency_burst (+48 more)

### Community 5 - "DRAM Channel Header"
Cohesion: 0.03
Nodes (55): memory_controller_command_t, request_priority_t, vector, memory_channel_t, BANK, bank_bits_mask, bank_bits_shift, BANK_BUFFER_SIZE (+47 more)

### Community 6 - "Cache Header"
Cohesion: 0.04
Nodes (51): cache_t, associativity, cache_access, cache_back_invalidation, cache_count_per_type, cache_eviction, cache_hit, cache_hit_per_type (+43 more)

### Community 7 - "Cache Manager Header"
Cohesion: 0.04
Nodes (53): cache_manager_t, CACHE_MANAGER_DEBUG, data_cache, DATA_LEVELS, dataSearch, DCACHE_AMOUNT, i, ICACHE_AMOUNT (+45 more)

### Community 8 - "Cmd Options"
Cohesion: 0.04
Nodes (51): archsim_cf_file(), basic_block_vector(), bbv_file(), cb_match(), csv_region(), date(), default_phases(), dimensions() (+43 more)

### Community 9 - "Cmd Options 2"
Cohesion: 0.04
Nodes (11): CallbackList(), cbsp_cfgs(), config_file(), PhaseGroup(), Call back used for options which can have multiple instances. Allows multiple…, Define the group for 'default' options., Define the group for options which apply to Simponts., SimpointPhaseGroup() (+3 more)

### Community 10 - "Simulation Engine"
Cohesion: 0.04
Nodes (44): cache_manager_t, vector, orcs_engine_t, allocate, arg_trace_file_name, branchPredictor, cacheManager, config_file (+36 more)

### Community 11 - "Regions"
Cohesion: 0.06
Nodes (46): CheckRegions(), cleanup(), GenRegionCSV(), GetBlockIDs(), GetDimRandomVector(), GetLDVWeights(), GetOptions(), GetRegionBBV() (+38 more)

### Community 12 - "Memory Order Buffer"
Cohesion: 0.05
Nodes (42): memory_operation_t, package_state_t, memory_request_client_t, memory_operation, readyAt, status, waiting_DRAM, memory_order_buffer_line_t (+34 more)

### Community 13 - "Correct Size Matrix"
Cohesion: 0.07
Nodes (44): CheckRegions(), cleanup(), GenRegionCSV(), GetBlockIDs(), GetDimRandomVector(), GetLDVWeights(), GetOptions(), GetRegionBBV() (+36 more)

### Community 14 - "Utils"
Cohesion: 0.09
Nodes (42): TYPE, FILE, hash_function_t, reorder_buffer_line_t, string, pointer_rob_to_string(), template_allocate_array(), template_allocate_initialize_array() (+34 more)

### Community 15 - "Pinplay Debugger Shell"
Cohesion: 0.07
Nodes (34): CALL_ORDER, CHAR, LOCALFUN, ADDRINT, BBL, BOOL, CONTEXT, INS (+26 more)

### Community 16 - "Pcregions"
Cohesion: 0.09
Nodes (43): ensure_string(), CheckRegions(), cleanup(), GenRegionCSV(), GetFirstPcinfo(), GetMarker(), GetRegionBBV(), GetSimpoints() (+35 more)

### Community 17 - "Pinplay Debugger Shell 2"
Cohesion: 0.07
Nodes (26): EVENTS, HELP_CATEGORIES, ADDRINT, PIN_LOCK, UINT32, DR_SHELL, _clientArgs, _events (+18 more)

### Community 18 - "Vima Controller"
Cohesion: 0.05
Nodes (42): vima_vector_t, vima_controller_t, bits_shift, cache, cache_accesses, cache_hits, cache_misses, cache_reads (+34 more)

### Community 19 - "Util 2"
Cohesion: 0.06
Nodes (38): CheckGdb(), GdbInitialize(), Check GDB to see if it's compatible with the DrDebug environment. Make sure GDB…, Get a user specific file name for the commands which will be used to invoke…, Ensure the correct version of GDB is installed and initialize the GDB command…, SetGdbCmdFile(), AddScriptPath(), BackgroundRunCmd() (+30 more)

### Community 20 - "Simulator Includes"
Cohesion: 0.05
Nodes (31): btb_line_t, btb_t, cache_manager_t, cache_t, circular_buffer_t, directory_line_t, directory_manager_t, directory_set_t (+23 more)

### Community 21 - "Util 3"
Cohesion: 0.07
Nodes (37): ChangeExtension(), FindDynamicICount(), FindResultString(), FindString(), FindStringRaw(), GetAllIcount(), GetBaseWPDir(), GetDefaultWPDir() (+29 more)

### Community 22 - "Circular Buffer"
Cohesion: 0.07
Nodes (24): CB_TYPE, circular_buffer_t, allocate, back, beg_index, capacity, circular_buffer_t<CB_TYPE>::back(), circular_buffer_t<CB_TYPE>::front() (+16 more)

### Community 23 - "Processor"
Cohesion: 0.09
Nodes (34): cacheId_t, FILE, opcode_package_t, Setting, allocate, clock, commit, decode (+26 more)

### Community 24 - "Branch Predictor"
Cohesion: 0.08
Nodes (31): branch_prediction_method_t, branch_predictor_t, allocate, BRANCH_PREDICTION_METHOD, branches, branchNotTaken, branchNotTakenMiss, branchPredictor (+23 more)

### Community 25 - "Pinplay Debugger Shell 3"
Cohesion: 0.24
Nodes (4): ETYPE, T, string, ParseNumber()

### Community 26 - "Vima Vector"
Cohesion: 0.07
Nodes (31): package_state_t, vima_vector_t, address, allocate, clock, column, dirty, fetch (+23 more)

### Community 27 - "Pinplay Debugger Shell 4"
Cohesion: 0.17
Nodes (20): AFUNPTR, BBL, INS, IPOINT, TRACE, VOID, DR_DEBUGGER_SHELL::OutputTargetRemoteInfo(), DR_DEBUGGER_SHELL::SetGdbMode() (+12 more)

### Community 28 - "Intrinsics Extension"
Cohesion: 0.16
Nodes (32): INT, RTN, arch_x86_set_data_instr(), arch_x86_trace_instruction(), ADDRINT, string, THREADID, UINT32 (+24 more)

### Community 29 - "Disambiguation Hashed"
Cohesion: 0.06
Nodes (28): desambiguation_t, allocate, make_memory_dependences, reset_statistics, solve_memory_dependences, statistics, FILE, disambiguation_hashed_t (+20 more)

### Community 30 - "Pinpoints"
Cohesion: 0.08
Nodes (19): main(), PinPoints, object, Add the PinPlay kit option to the current list of options. @return No return…, Process command line arguments and run the script @return Exit code from…, Get the PinPlay kit. @return Pinplay kit object, Clean up when a signal is sent and exit. @param signal Signal to handle @param…, Install a signal handler to make a clean exit. @return No return value (+11 more)

### Community 31 - "Uop Package"
Cohesion: 0.06
Nodes (32): functional_unit_t, instruction_operation_t, package_state_t, uop_package_t, born_cycle, functional_unit, hive_read1, hive_read2 (+24 more)

### Community 32 - "Cmd Options 3"
Cohesion: 0.07
Nodes (29): add_program_wp(), archsim_config_dir(), combine(), command(), compressed(), coop_lit(), coop_pinball(), cross_os() (+21 more)

### Community 33 - "Cache Manager Requests"
Cohesion: 0.14
Nodes (27): available, cache_search, check_consistency, clock, count_llc_secondary_miss, fill_upper_levels, finishRequest, generateIndexArray (+19 more)

### Community 34 - "Hive Controller"
Cohesion: 0.07
Nodes (28): hive_controller_t, CORE_TO_BUS_CLOCK_RATIO, data_cache, HIVE_BUFFER, hive_instructions, hive_lock, hive_op_latencies, hive_register_free (+20 more)

### Community 35 - "Opcode Package and Sync Type"
Cohesion: 0.07
Nodes (29): memory_request_client_t, sync_type_t, opcode_package_t, base_reg, branch_type, hive_read1, hive_read2, hive_write (+21 more)

### Community 36 - "Sinuca Tracer"
Cohesion: 0.14
Nodes (25): ADDRINT, BOOL, CONTEXT, EVENT_TYPE, IMG, INT32, PIN_MULTI_MEM_ACCESS_INFO, THREADID (+17 more)

### Community 37 - "Reorder Buffer Line"
Cohesion: 0.09
Nodes (24): reorder_buffer_line_t, string, memory_order_buffer_line_t, processor_stage_t, uop_package_t, reorder_buffer_line_t, committed, content_to_string (+16 more)

### Community 38 - "Kit"
Cohesion: 0.10
Nodes (15): Kit, object, Method called when object is instantiated to initalize object. @return No…, Set the file type: either 32-bit or 64-bit. @param binary Binary used to…, Is this a path to a kit with a valid pinplay driver? @param path Path to kit to…, Is this a path to a valid kit? A valid kit must contain both the binary…, Get the location defined by the user's 'home' parameter/option., Look for a kit in several locations, including the 'home' directory, if it's… (+7 more)

### Community 39 - "OrCS README Concepts"
Cohesion: 0.10
Nodes (26): Variable Cache Line Size Support, libconfig, Multi-threaded Trace Generation, noclone3 Pin workaround, OrCS Simulator, ORCS_tracing_start / ORCS_tracing_stop, Intel Pin 3.11, sandy_bridge.cfg configuration (+18 more)

### Community 40 - "Branch Profiler"
Cohesion: 0.10
Nodes (24): CONTEXT, INT32, THREADID, TRACE, UINT32, UINT64, UINT8, VOID (+16 more)

### Community 41 - "Record"
Cohesion: 0.11
Nodes (15): main(), object, There are no additional options for non-GDB scripts. This is only a stub.…, Parse command line arguments. @return tuple with: @return - options given on…, Get the logging options required for DrDebug. @return string of knobs, No knobs to add. However must return a string so just return the empty string.…, Nothing to initialize, just return @return no return value, Run the script in the foreground. @param cmd Command line to run lower level… (+7 more)

### Community 42 - "Replay"
Cohesion: 0.11
Nodes (15): main(), object, There are no additional options for non-GDB scripts. This is only a stub.…, Parse command line arguments. @return tuple with: @return - options given on…, No pin knobs to add. @return empty string, Do nothing. @return no return, Add the default knobs required for replaying and any user defined options.…, Run the script in the foreground. @param cmd Command line to run lower level… (+7 more)

### Community 43 - "Hive Controller 2"
Cohesion: 0.16
Nodes (20): FILE, addRequest, allocate, check_sub_requests, check_transmit, check_wait, clock, hive_dispatch (+12 more)

### Community 44 - "Sde Pinpoints"
Cohesion: 0.10
Nodes (13): main(), Get the simulator kit. @return Simulator kit, Add additional default options which SDE needs, but PinPlay doesn't use. @param…, Add additional phase options which SDE needs, but PinPlay doesn't use. @param…, Add additional modify options which SDE needs, but PinPlay doesn't use. @param…, Add the options which used for LIT file verification. @param parser Optparse…, Run the additional phases for SDE which are not run for PinPlay. @param…, Process command line arguments and run the script @return Exit code from… (+5 more)

### Community 45 - "Linha"
Cohesion: 0.10
Nodes (14): cacheSet_t, lines, n_lines, line_t, dirty, line_ptr_caches, lru, NUMBER_OF_PROCESSORS (+6 more)

### Community 46 - "Replay Dir"
Cohesion: 0.13
Nodes (13): main(), object, Replay a single pinball given the command line options and the name of the…, Replays one set of pinballs. Calls Replay() with the pinball name. @param…, Replays all pinballs based on a partial file name. @param options Options given…, Replay pinballs using one of the 4 possible methods. 1) Replay one pinball 2)…, Get all the user options and replay all the desired pinballs. @return Exit code…, Process command line arguments and run the replayer @return Exit code from… (+5 more)

### Community 47 - "Alterações No Tratamento De Acessos À Me"
Cohesion: 0.14
Nodes (20): Alterações no tratamento de acessos à memória para instruções RVV no SPIKE + OrCS, orcs_tracer_t::fill_instruction_info, SPIKE RISC-V simulator, trace_reader_t::trace_string_to_opcode, Variable number of RVV memory accesses (-1 + $<n> annotation), Memory accesses fusing, get_instruction_loads_stride, OrCS In-Order (+12 more)

### Community 48 - "Trace Reader"
Cohesion: 0.10
Nodes (19): gzFile, opcode_package_t, trace_reader_t, address_translation, binary_bbl_size, binary_dict, binary_total_bbls, currect_bbl (+11 more)

### Community 49 - "Prefetcher"
Cohesion: 0.12
Nodes (17): cache_t, FILE, memory_order_buffer_line_t, vector, prefetcher_t, allocate, latePrefetches, PARALLEL_PREFETCH (+9 more)

### Community 50 - "Instruction Set"
Cohesion: 0.11
Nodes (17): fu_info_t, size, wait_next, string, vector, instruction_set_t, allocate, fu_id (+9 more)

### Community 51 - "Gdb Record"
Cohesion: 0.10
Nodes (12): GdbRecord, main(), Add additional default options which GDB version of scripts need. @param parser…, Get the GDB specific DrDebug logging options. When using GDB, there are…, Add pin knobs (not pintool knobs) needed when running with GDB. @return string…, Initialize GDB and do some basic checks to see how well this version of GDB…, When running with GDB, run the script in the background, not the foreground…, Run code required to complete the GDB scripts. @param kit_script_path Explicit… (+4 more)

### Community 52 - "Gdb Replay"
Cohesion: 0.10
Nodes (12): GdbReplay, main(), Add pin knobs (not pintool knobs) needed when running with GDB. @return string…, Add additional default options which GDB version of scripts need. @param parser…, Initialize GDB and do some basic checks to see how well this version of GDB…, Add the default knobs required for replaying and any user defined options. Need…, When running with GDB, run the script in the background, not the foreground.…, Run code required to complete the GDB scripts. @param kit_script_path Explicit… (+4 more)

### Community 53 - "Sniper Pinpoints"
Cohesion: 0.15
Nodes (15): CalcPredError(), ignore_sniper_error(), main(), no_sniperlite(), Print the directories where the Pin kit and Sniper are located. @return No…, Add additional phase options which Sniper needs, but PinPlay doesn't use., Add additional phase options which SDE needs, but PinPlay doesn't use., Run the additional phases for SDE which are not run for PinPlay. (+7 more)

### Community 54 - "Set Associative"
Cohesion: 0.12
Nodes (14): C_TYPE, set_associative_t, allocate, associativity, set_associative_t<C_TYPE>::map_set(), set_associative_t<C_TYPE>::operator[](), get_associativity, get_num_sets (+6 more)

### Community 55 - "Processor 2"
Cohesion: 0.11
Nodes (15): functional_unit_t, dispatch_cnt, id, size, slot, wait_next, searchPositionROB, update_registers (+7 more)

### Community 56 - "Util 4"
Cohesion: 0.12
Nodes (19): CountClusters(), GetClusterInfo(), GetCSVFiles(), GetIterDir(), GetRegionInfo(), ParseFileName(), ParseFullFileName(), ParsePartialFileNameIregion() (+11 more)

### Community 57 - "Direct Tracer"
Cohesion: 0.16
Nodes (11): bbl_executed(), BlocksLoaded, BBLs, opcode_package_t, vector, Fini(), size, My_BBL (+3 more)

### Community 58 - "Piecewise"
Cohesion: 0.13
Nodes (15): taken_t, predictor_t, piecewise_t, allocate, GA, GHR, H, M (+7 more)

### Community 59 - "Cache Eviction and Install"
Cohesion: 0.20
Nodes (16): allocate, evict, installLine, mark_dirty, peek, read, reset_statistics, returnLine (+8 more)

### Community 60 - "Simpoint"
Cohesion: 0.15
Nodes (11): main(), object, Normalize and project the basic block vector file instead of doing this in…, Normalize and apply weights to LRU stack distance vector files. @param options…, Combine the BBV and LDV files, applying a scaling factor to allow different…, Format and execute the command to run Simpoint. @param options Options given on…, Format and execute the command to generate the regions CSV file. @param options…, Run the scripts required to run simpoint and generate a region CSV file with… (+3 more)

### Community 61 - "Instruction Type Count"
Cohesion: 0.15
Nodes (16): CONTEXT, INT32, THREADID, TRACE, UINT64, UINT8, VOID, docount() (+8 more)

### Community 62 - "Pinplay Debugger Shell 5"
Cohesion: 0.14
Nodes (11): AT_ICOUNT, _icount, _tid, AT_MCOUNT, _mcount, _tid, UINT64, THREAD_DATA (+3 more)

### Community 63 - "Opcode Package"
Cohesion: 0.19
Nodes (8): vector, string, string, opcode_package_t::content_to_string(), opcode_package_t::content_to_string2(), opcode_package_t::log_registers(), opcode_package_t, opcode_to_trace_string()

### Community 64 - "Memory Request Client"
Cohesion: 0.14
Nodes (6): PIN_MULTI_MEM_ACCESS_INFO, TRACE, VOID, touch_memory(), tracer(), unknown_memory_size_f()

### Community 65 - "Memory Dependences"
Cohesion: 0.17
Nodes (17): make_memory_dependences, solve_memory_dependences, clean_mob_hive, clean_mob_read, clean_mob_vima, execute, get_next_op_hive, get_next_op_load (+9 more)

### Community 66 - "Sinuca Pinpoints"
Cohesion: 0.16
Nodes (11): main(), Get the simulator kit., Add additional phase options which Sniper needs, but PinPlay doesn't use., Add additional phase options which the simulator needs, but PinPlay doesn't use., Run the additional phases for the simulator which are not run as part of the…, Process command line arguments and run the script, Sinuca_tracer_output(), Sinuca_tracer_parallel_end() (+3 more)

### Community 67 - "Profile"
Cohesion: 0.18
Nodes (15): ADDRINT, CONTEXT, IMG, INT32, THREADID, TRACE, VOID, DynamicOMP_char() (+7 more)

### Community 68 - "Stride Prefetcher"
Cohesion: 0.20
Nodes (13): status_stride_prefetcher_t, stride_prefetcher_t, allocate, DISTANCE, installStride, searchLRU, searchPattern, statistics (+5 more)

### Community 69 - "Cmd Options 4"
Cohesion: 0.14
Nodes (15): app_version(), cbsp_name(), compiler_version(), cutoff(), epilog_length(), maxk(), platform(), prolog_length() (+7 more)

### Community 70 - "Pinplay Debugger Shell 6"
Cohesion: 0.13
Nodes (10): HELPS, HELP, HELP_CATEGORY, _description, _formattedHelp, _helpStrings, _intro, _name (+2 more)

### Community 71 - "Sync Manager"
Cohesion: 0.20
Nodes (11): sync_manager_t, active_cores, allocate, arrived, barrier_arrive, generation, lock_acquire, lock_owner (+3 more)

### Community 72 - "Log"
Cohesion: 0.18
Nodes (10): Log, main(), object, Add multi-process logging mode options. @param options Options given on cmd…, Parse command line arguments and check to make sure all required options were…, Get all the user options and run the logger. @return Exit code from the logger…, This method allows the script to be run in stand alone mode. @return Exit code…, Generate log files for an application This class is the low level primitive… (+2 more)

### Community 73 - "Make Simpoint Weights"
Cohesion: 0.18
Nodes (14): get_arg_num(), main(), measureWeightsWithLabels(), measures the weights for the trace @type labels: list @param labels: labels…, writes the output weights in file @type weights: string @param weights: name of…, :param argv: The command line :param op: we want to find how much arguments…, main function parses the arguments and calls functions to do the job, Reads the simpoints from points file and return them in a list @type… (+6 more)

### Community 74 - "Phases"
Cohesion: 0.13
Nodes (9): Phases, object, Delete files & directories generated for only the current tracing instance.…, Run the logger to generate whole program pinballs. @return error_code, This module contains all PinPlay tracing phases used in high level scripts. In…, Generate the basic block vectors for the whole program pinballs. @return…, Run Simpoint with the basic block vectors for a whole program pinball. 'param'…, If number of clusters for this file > max_num_clusters then set class attribute… (+1 more)

### Community 75 - "Pinplay Debugger Shell 7"
Cohesion: 0.21
Nodes (5): BOOL, CONTEXT, INT32, THREADID, DR_DEBUGGER_SHELL::CreatePinPlayShell()

### Community 76 - "Trace Reader Sync Markers"
Cohesion: 0.24
Nodes (13): opcode_package_t, sync_type_t, allocate, define_binary_bbl_size, generate_binary_dict, get_total_bbls, get_trace_size, pin_next (+5 more)

### Community 77 - "Vima Controller 2"
Cohesion: 0.19
Nodes (11): cache_status_t, FILE, vima_vector_t, addRequest, allocate, check_completion, clock, instruction_ready (+3 more)

### Community 78 - "Cbsp"
Cohesion: 0.19
Nodes (9): CBSP, main(), Process command line arguments and read in config file/globals. @return List of…, There are no additional phases for PinPlay. This is only a stub. @param…, Get the user's options, read config file and execute the phases desired by the…, Process command line arguments and run the script @return Exit code from…, Top level script for generating traces using CBSP (Cross Binary Simpoints).…, For Pin/PinPlay CBSP, redefine the option '--default_phases' instead of using… (+1 more)

### Community 79 - "Config"
Cohesion: 0.16
Nodes (8): ConfigClass, Tracing configuration parameters are read in from a file, if it exists. Any…, Explicitly set a subset of config global attributes to specific values for each…, Explicitly clear a subset of config global attributes for each binary in CBSP.…, Print a error msg about forbidden characters. @param string String with name of…, Generate the basename for the PinPoint per instance files. The instance name…, Generate the basename for the CBSP per instance files. The instance name…, Get either PinPoint or CBSP instance base file name. @param options Options…

### Community 80 - "Pin"
Cohesion: 0.20
Nodes (12): getLineAddress(), getLineAddressRange(), handleBreakAt(), handleTraceAt(), Get the code starting and ending address for a given line number, Get the code starting address for a given line number, Parse command arguments to determine source line address range, Parse comand arguments to get variable address and length (+4 more)

### Community 81 - "Sim Run"
Cohesion: 0.20
Nodes (9): main(), object, Get the simulator kit., Get all the user options and run the simulator., Process command line arguments and run the simulator, This class is the low level primative which runs a simulator on one set of…, There are no additional options for the base clase simulator. This is only a…, Process command line arguments. Generate pin and tool options, and their paths. (+1 more)

### Community 82 - "Gerando Traços Com O Sinuca Pinpoints"
Cohesion: 0.26
Nodes (13): Direct tracer, Gerando traços com o Sinuca PinPoints, SimPoints, Sinuca PinPoints, Gerando traços do SPEC 2006, pinplay.cfg slice_size, SPEC CPU 2006, Manual atualização pinpoints 3.11 (+5 more)

### Community 83 - "Memory Channel"
Cohesion: 0.23
Nodes (9): memory_controller_command_t, addRequest, allocate, clock, findNext, findNextRead, findNextWrite, get_minimum_latency (+1 more)

### Community 84 - "Config 2"
Cohesion: 0.15
Nodes (10): GlobalVar, PhaseStr(), object, Clean up any global data files which may still exist. For instance, if there…, # NOTE: Don't forget to declare the attribute being set, # NOTE: If you are adding a kit for a new simulator, then you need, Format a unique string for each phase. @param phase Phase for string @return…, Contains methods to dump/save global data attributes. When one script invokes… (+2 more)

### Community 85 - "Logger"
Cohesion: 0.17
Nodes (9): Logger, main(), object, Get the PinPlay kit. @param parser Command line parser @return PinPlay kit, Get all the user options and run the logger. @return Exit code from the logger…, This method allows the script to be run in stand alone mode. @return Exit code…, Generate log files for an application This class is the mid level primitive…, This function adds an option to the tool if the option was not already passed… (+1 more)

### Community 86 - "Sde Kit"
Cohesion: 0.15
Nodes (7): For SDE, make sure the 'simpoint' binary is in the kit instead of verifying the…, Get the location defined by the user's 'home' parameter/option., Just return the Pintool. Even thought SDE no longer requires a pintool (because…, SDE by default does not need a pintool. However, if user defines one then need…, Get the location of the traceinfo 'blank' XML file fragments. Need to remove…, Get the path and pintool for an SDE kit, SDEKit

### Community 87 - "Sim Kit"
Cohesion: 0.19
Nodes (8): object, Look in several locations to find a kit., Get the path to a valid kit, the appropriate tool name and add a path to the…, Get the path to the processor configuration file for this simulator., Setup the path, directories and binary for the x86noas simulator., Method called when object is instantiated to initalize object., Is this a path to a valid simulator?, SimKit

### Community 88 - "Conversions"
Cohesion: 0.21
Nodes (11): ascii_to_hex(), INS, opcode_package_t, string, gen_icode(), hex_to_dec(), to_string(), x86_to_static() (+3 more)

### Community 89 - "Cbsp Phases"
Cohesion: 0.17
Nodes (7): CBSPPhases, Delete files & directories generated for only the current tracing instance.…, Delete ALL files & directories generated for all tracing instances. @param…, Run the cross binary matcher on all the binaries in the CBSP experiment. NOTE:…, Run Simpoint in the CBSP Data directory and generate weight files for each…, Module contains CBSP (Cross Binary SimPoint) methods implementing phases not…, Print parameters for both the master CBSP and for each binary. It's very handy…

### Community 90 - "Config 3"
Cohesion: 0.20
Nodes (6): Set the config global attributes from a dictionary of parameters/values. If an…, Print msg indicating user must define a parameter., Check to make sure a config global attribute has been initialized. @param var…, Check to make sure the two basic, required variables have been initialized.…, Check to make sure the two basic, required variables and two additional…, Set the config global attributes from all the different methods used to get…

### Community 91 - "Sde Cbsp"
Cohesion: 0.17
Nodes (8): main(), Run the CBSP additional phases for SDE which are not run for PinPlay. @param…, Process command line arguments and run the script @return Exit code from…, Top level script for generating traces using Cross Binary Simpoints with SDE.…, For SDE CBSP, redefine the option '--default_phases' instead of using the one…, Add additional default options which SDE needs, but PinPlay doesn't use. @param…, Add additional phase options CBSP which SDE needs, but PinPlay doesn't use.…, SDECBSP

### Community 92 - "Sde Phases"
Cohesion: 0.17
Nodes (7): Generate either the LIT files or warmup LMAT files for one pinball based on the…, Generate LIT files and warmup LMAT files for all region pinballs in the tracing…, This module contains the tracing phases used in high level scripts which are…, Use a simulator to verify all the LIT files for the tracing instance are valid., Generate the instruction mix for one pinball., Generate traceinfo files from sysinfo files. Also do some simple error…, SDEPhases

### Community 93 - "Brpred Metric"
Cohesion: 0.22
Nodes (8): BrPredMetric, main(), object, Execute the desired behavior., Process command line arguments and run the script, Top level script to calculate the predicted metric of interest, actual metric…, Get the Branch Predictor simulation kit., Process command line arguments and do error checking.

### Community 94 - "Replayer"
Cohesion: 0.22
Nodes (8): main(), object, Get the PinPlay kit. @return PinPlay kit, Get all the user options and run the replayer. @return Exit code from the…, Process command line arguments and run the replayer @return Exit code from…, Replays one set of log files. This class is the low level primative which…, Process command line arguments, get Kit, tool options, and their paths. @return…, Replayer

### Community 95 - "Library Call"
Cohesion: 0.29
Nodes (10): called_rtn(), IMG, INT32, THREADID, VOID, Fini(), ImageLoad(), ImageUnload() (+2 more)

### Community 96 - "Enumerations"
Cohesion: 0.18
Nodes (10): cacheId_t, hash_function_t, memory_controller_command_t, request_priority_t, status_stride_prefetcher_t, get_enum_cache_type_char(), get_enum_hash_function_char(), get_enum_memory_controller_command_char() (+2 more)

### Community 97 - "Twobit"
Cohesion: 0.22
Nodes (7): FILE, predictor_t, twoBit_t, allocate, predict, statistics, train

### Community 98 - "Memory Controller"
Cohesion: 0.22
Nodes (8): FILE, allocate, clock, isBusy, requestDRAM, reset_statistics, set_masks, statistics

### Community 99 - "Stride Table"
Cohesion: 0.22
Nodes (7): status_stride_prefetcher_t, stride_table_t, last_address, lru, status, stride, tag

### Community 100 - "Brpred Kit"
Cohesion: 0.20
Nodes (6): BrPredKit, Get the knob & base file name required to generate an output file for the…, Run Branch Predictor simulator on all the pinballs in a set of directories…, Get the last metric in a simulator output file. This is the value for running…, Get the metric of interest for just the representative region, not including…, This class contains methods to run the branch predictor simulator and extract…

### Community 101 - "Brpred Pinpoints"
Cohesion: 0.22
Nodes (6): BrPredPinPoints, main(), Run the additional phases for the simulator which are not run as part of the…, Process command line arguments and run the script, Get the simulator kit., Add additional phase options which the simulator needs, but PinPlay doesn't use.

### Community 102 - "Config 4"
Cohesion: 0.20
Nodes (5): Get a string parameter from a cfg file parser. @param section Section in the…, Get PinPoint or CBSP instance file name with the specified file extension.…, Save a parameter/value pair in a configuration file. 1) If the parameter is…, Get the current value of a parameter from the per instance configuration file.…, Clear the value for a parameter in the per instance configuration file. Do this…

### Community 103 - "Config 5"
Cohesion: 0.20
Nodes (5): Get a boolean parameter from a cfg file parser. @param section Section in the…, Get a integer parameter from a cfg file parser. @param section Section in the…, Get a float parameter from a cfg file parser. @param section Section in the…, For debugging, print out the sections & values. @param parser Configuration…, Read the parameters from a tracing configuration file parser. Need to add new…

### Community 104 - "Pin 2"
Cohesion: 0.24
Nodes (7): commonParse(), ForwardSliceCommand, Create a program slice pin slice <instance> <thread> { <variableName> |…, Top level handler for the slice command, Create a program forward slice pin forward-slice <instance> <thread> {…, Common parse code for slice, forward-slice and backward-slice commands, SliceCommand

### Community 105 - "Relog"
Cohesion: 0.20
Nodes (7): main(), Add the default knobs required for logging and any user defined options. @param…, This method allows the script to be run in stand alone mode. @return Exit code…, Replay DrDebug log files and generate a new log file, based on options used to…, Get the application specific usage and description strings for this module.…, Get arguments passed to the script. The relog script requires two command line…, Relog

### Community 106 - "Sde Cmpsim Kit"
Cohesion: 0.20
Nodes (6): CMPsimKit, Get the knob & base file name required to generate an output file for the…, Run a simulator on all the pinballs in a directory given by 'dirname'. @param…, Get the last metric in a CMPSim output file. This is the value for running the…, Get the metric of interest for just the representative region, not including…, This class contains methods to run CMPSim and analyze the output files it…

### Community 107 - "Uop Package 2"
Cohesion: 0.22
Nodes (7): uop_package_t, operator==, package_clean, updatePackageFree, updatePackageReady, updatePackageUntrated, updatePackageWait

### Community 108 - "Pin 3"
Cohesion: 0.25
Nodes (5): BackwardSliceCommand, Initialize the SliceCommand object, Initialize the ForwardSliceCommand object, Create a program backward slice pin backward-slice <instance> <thread> {…, Initialize the PinCommand object

### Community 109 - "Sim Run Dir"
Cohesion: 0.22
Nodes (6): main(), Process command line arguments and run the replayer, Run the simulator on multiple LIT files. This class is a wrapper which…, Add any additional options besides the default for all *_dir.py scripts., Run the Simulator on a single LIT file given the command line options and the…, SimRunMulti

### Community 110 - "X86 Run"
Cohesion: 0.22
Nodes (6): main(), Replays one set of trace files. This class is the low level primative which…, Add additional command line options for this simulator., If running x86, need to add archsim config directory., Process command line arguments and run x86, X86Run

### Community 111 - "Intrinsics Extension 2"
Cohesion: 0.22
Nodes (9): gzFile, PIN_LOCK, UINT8, thread_data_t, dyn_lock, gzDynamicTraceFile, gzMemoryTraceFile, is_instrumented_bbl (+1 more)

### Community 112 - "OpenMP Sync Test Kernel"
Cohesion: 0.39
Nodes (6): ORCS_tracing_start(), ORCS_tracing_stop(), main(), _vim2K_fadds(), main(), _vim2K_fadds()

### Community 113 - "Btb Line"
Cohesion: 0.25
Nodes (7): btb_line_t, bht, lru, tag, targetAddress, typeBranch, validade

### Community 114 - "Msg"
Cohesion: 0.25
Nodes (8): PrintMsg(), PrintMsgDate(), PrintMsgPlus(), PrintStart(), Print out a msg with three '*' and a timestamp. Use the method…, Print out a msg with three '+'. Use the method subprocess.Popen() in order to…, If not listing, print a string., Prints a message to stdout. Use the method subprocess.Popen() in order to…

### Community 115 - "Phases 2"
Cohesion: 0.25
Nodes (4): Generate a set of region pinballs for one whole program pinball. This method…, Generate all region pinballs for a set of whole program pinball directories. If…, Save the files from the current iteration and generate the CSV file with the…, For some multi-threaded runs, it's possible the region pinball generation phase…

### Community 116 - "Phases 3"
Cohesion: 0.25
Nodes (4): Relog the old whole program pinballs with options which modify the new whole…, Relog the whole program pinballs with a user defined name. No need for extra…, Relog the whole program pinballs with a focus thread. @return error_code, Relog the whole program pinballs with code exclusion using sets of start/stop…

### Community 117 - "Phases 4"
Cohesion: 0.32
Nodes (3): Relog the whole program pinballs to remove instructions between SSC marks.…, Relog the whole program pinballs to remove initialization instructions. @return…, Relog the whole program pinballs to remove OpenMP spin instructions. @return…

### Community 118 - "Pin 4"
Cohesion: 0.25
Nodes (6): PruneCommand, Get the address and length of the specified variable, Prune a program slice, removing variable references pin prune <slice_id> {…, Initialize the PruneCommand object, Top level handler for the prune command, resolveVariable()

### Community 119 - "X86 Run Dir"
Cohesion: 0.25
Nodes (6): main(), Run x86 on multiple pinballs. This class is a wrapper which processes either…, Get the x86 simulator kit., Add any additional options besides the default for all *_dir.py scripts., Process command line arguments and run the replayer, X86RunMulti

### Community 121 - "Cache Manager"
Cohesion: 0.48
Nodes (7): allocate, check_cache, get_cache_info, get_cache_levels, instantiate_cache, cacheId_t, Setting

### Community 123 - "Brpred Replay Dir"
Cohesion: 0.33
Nodes (5): BrPredReplayMulti, main(), Replay multiple pinballs. This class is a wrapper which replays either one…, Get the Branch predictor kit., Process command line arguments and run the replayer

### Community 124 - "Brpred Replayer"
Cohesion: 0.33
Nodes (5): BrPredReplayer, main(), Replays one set of log files using PinPlay. This class is the low level…, Get the Branch predictor kit., Process command line arguments and run the replayer

### Community 125 - "Rename"
Cohesion: 0.43
Nodes (6): CheckField(), CheckRequiredVariables(), GetOptions(), Get users command line options., Check to make sure a string doesn't contain any characters the GTR does not…, Check to make sure all the required input variables are defined.

### Community 126 - "Sde Cmpsim Metric"
Cohesion: 0.33
Nodes (5): CMPsimMetric, main(), Top level script for calculate the predicted metric of interest from the CMPSim…, Get the CMPSim simulation kit., Process command line arguments and run the script

### Community 127 - "Sde Cmpsim Replay Dir"
Cohesion: 0.33
Nodes (5): CMPsimReplayMulti, main(), Replay multiple pinballs. This class is a wrapper which replays either one…, Get the CMPSim SDE kit., Process command line arguments and run the replayer

### Community 128 - "Sde Cmpsim Replayer"
Cohesion: 0.33
Nodes (5): CMPsimReplayer, main(), Replays one set of log files using SDE. This class is the low level primative…, Get the CMPSim SDE kit., Process command line arguments and run the replayer

### Community 129 - "Sinuca Replay Dir"
Cohesion: 0.33
Nodes (5): main(), Replay multiple pinballs. This class is a wrapper which replays either one…, Get the Branch predictor kit., Process command line arguments and run the replayer, Sinuca_TracerReplayMulti

### Community 130 - "Sinuca Replayer"
Cohesion: 0.33
Nodes (5): main(), Replays one set of log files using PinPlay. This class is the low level…, Get the Branch predictor kit., Process command line arguments and run the replayer, Sinuca_TracerReplayer

### Community 131 - "Profile 2"
Cohesion: 0.29
Nodes (7): PIN_LOCK, UINT64, UINT8, thread_data_t, counter, dyn_lock, pad

### Community 132 - "Config 6"
Cohesion: 0.40
Nodes (4): configure_t, cfg, configure_t::configure_t(), loadConfig

### Community 133 - "Simulator"
Cohesion: 0.53
Nodes (5): string, display_use(), get_status_execution(), main(), process_argv()

### Community 134 - "Config 7"
Cohesion: 0.33
Nodes (3): If the config file exists, then parse it to get the parameters from the file.…, Generate an object of type optparse.Value which contains the required…, Get a set of parameters for each one of the CBSP binaries. For each binary,…

### Community 135 - "Phases 5"
Cohesion: 0.33
Nodes (3): Get the predicted value for a metric for the pinballs in a directory. The…, Get measured metric of interest from the WP pinballs and predicted metric from…, Calculate the prediction error correlation for a set of whole program pinballs…

### Community 136 - "Pin 5"
Cohesion: 0.33
Nodes (4): BreakCommand, Set a Pin conditional breakpoint pin break { <sourceLine> |…, Initialize the BreakCommand object, Top level handler for the break command

### Community 137 - "Pin 6"
Cohesion: 0.33
Nodes (5): execute(), PinCommand, Invoke a monitor command, Miscellaneous Pin commands and command prefix for all Pin subcommands pin list…, Top level handler for the pin command

### Community 138 - "Pin 7"
Cohesion: 0.33
Nodes (6): parseDwarfExpression(), Rewrite the register specification for special case registers, Parse a DWARF expression to get location of a variable, Get address of a variable which may be local to a function or block, resolveScopedVariable(), rewriteRegister()

### Community 139 - "Pin 8"
Cohesion: 0.33
Nodes (4): Start/stop recording a Pin program log pin record on | off on: Start recording…, Initialize the RecordCommand object, Top level handler for the record command, RecordCommand

### Community 140 - "Pin 9"
Cohesion: 0.33
Nodes (4): Monitor variable at a specified instruction address pin trace { <variable> |…, Initialize the TraceCommand object, Top level handler for the trace command, TraceCommand

### Community 141 - "Sde Replayer"
Cohesion: 0.40
Nodes (4): main(), Replays one set of log files using SDE. This class is the low level primative…, Process command line arguments and run the replayer, SDEReplayer

### Community 143 - "Btb"
Cohesion: 0.40
Nodes (3): btb_t, btb_entry, btb_line_t

### Community 144 - "Uop Package 3"
Cohesion: 0.50
Nodes (5): string, content_to_string, content_to_string2, instruction_operation_t, get_enum_instruction_operation_char()

### Community 145 - "Sde Log"
Cohesion: 0.40
Nodes (4): main(), Get the SDE kit. @return SDE kit, This method allows the script to be run in stand alone mode. @return Exit code…, SDELog

### Community 146 - "Sde Logger"
Cohesion: 0.40
Nodes (4): main(), Logger using SDE kit which is derived from the class logger., This method allows the script to be run in stand alone mode., SDELogger

### Community 147 - "Sde Replay Dir"
Cohesion: 0.40
Nodes (4): main(), Replay multiple pinballs. This class is a wrapper which replays either one…, Process command line arguments and run the replayer, SDEReplayMulti

### Community 148 - "Traceinfo"
Cohesion: 0.40
Nodes (4): FmtPrintCmd(), GetOptions(), Format the appropriate command to 'cat' a file for a given architecture.…, Get users command line options/args and check to make sure they are correct.…

### Community 149 - "Pinplay Debugger Shell 9"
Cohesion: 0.40
Nodes (4): REG, REG_INFO, _name, _reg

### Community 150 - "Uop Package 4"
Cohesion: 0.50
Nodes (4): functional_unit_t, instruction_operation_t, opcode_package_t, opcode_to_uop

### Community 151 - "Cmd Options 5"
Cohesion: 0.50
Nodes (3): BlankLinesIndentedHelpFormatter, Overload a method in class OptionParse which allows the programmer to insert…, Replace the two new lines '\n\n' (i.e. <CR><CR>) with special string and split…

### Community 152 - "Cmd Options 6"
Cohesion: 0.50
Nodes (3): input_name(), program_name(), Check to see if several parameters contain one of the 'forbidden' chars, '.' or…

### Community 153 - "Sinuca Kit"
Cohesion: 0.50
Nodes (3): Run Branch Predictor simulator on all the pinballs in a set of directories…, This class contains methods to run the branch predictor simulator and extract…, Sinuca_TracerKit

### Community 154 - "X86 Kit"
Cohesion: 0.50
Nodes (3): Setup the path, directories and binary for the x86 simulator., Get the path to a valid kit, the appropriate tool name and add a path to the…, X86Kit

### Community 155 - "Direct Tracer 2"
Cohesion: 0.50
Nodes (4): MemoryAccess, addr, bbl, is_read

### Community 156 - "Run Traces.Sh"
Cohesion: 0.50
Nodes (3): OMP_NUM_THREADS, OMP_WAIT_POLICY, run_traces.sh script

### Community 157 - "Trace Reader 2"
Cohesion: 0.67
Nodes (3): branch_t, instruction_operation_t, trace_string_to_opcode

## Ambiguous Edges - Review These
- `SPEC OMP 2001 Benchmark List` → `SPEC OMP 2012 Benchmark List`  [AMBIGUOUS]
  trace_generator/scripts/apps_benchmarks/apps_spec_omp2012.txt · relation: semantically_similar_to

## Knowledge Gaps
- **921 isolated node(s):** `BANK`, `bank_bits_mask`, `bank_bits_shift`, `BANK_ROW_BUFFER_SIZE`, `BURST_WIDTH` (+916 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1797 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **62 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **What is the exact relationship between `SPEC OMP 2001 Benchmark List` and `SPEC OMP 2012 Benchmark List`?**
  _Edge tagged AMBIGUOUS (relation: semantically_similar_to) - confidence is low._
- **Why does `DR_SHELL` connect `Pinplay Debugger Shell 2` to `Pinplay Debugger Shell 6`, `Pinplay Debugger Shell 7`, `Pinplay Debugger Shell`, `Pinplay Debugger Shell 9`, `Pinplay Debugger Shell 10`, `Pinplay Debugger Shell 3`, `Pinplay Debugger Shell 8`, `Pinplay Debugger Shell 4`, `Pinplay Debugger Shell 5`?**
  _High betweenness centrality (0.106) - this node is a cross-community bridge._
- **Why does `memory_package_t` connect `Memory Package` to `Cache Manager Requests`, `Hive Controller`, `Memory Controller`, `DRAM Controller Header`, `DRAM Channel Header`, `Cache Manager Header`, `Hive Controller 2`, `Vima Controller 2`, `Vima Controller`, `Memory Channel`, `Simulator Includes`, `Vima Vector`, `Cache Eviction and Install`?**
  _High betweenness centrality (0.097) - this node is a cross-community bridge._
- **Why does `processor_t` connect `Processor Core Types` to `Memory Dependences`, `Processor`, `Simulator Includes`, `Processor 2`?**
  _High betweenness centrality (0.054) - this node is a cross-community bridge._
- **What connects `BANK`, `bank_bits_mask`, `bank_bits_shift` to the rest of the system?**
  _921 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Processor Core Types` be split into smaller, more focused modules?**
  _Cohesion score 0.011111111111111112 - nodes in this community are weakly interconnected._
- **Should `Util` be split into smaller, more focused modules?**
  _Cohesion score 0.024860022396416572 - nodes in this community are weakly interconnected._