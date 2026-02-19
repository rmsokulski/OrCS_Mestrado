#include "./../simulator.hpp"
#include <string>
// ============================================================================
memory_controller_t::memory_controller_t(){
    this->use_orcs = true;
    this->use_ramulator = false;
    this->requests_made = 0; //Data Requests made
    this->sub_requests_made = 0;
    this->operations_executed = 0; // number of operations executed
    this->requests_llc = 0; //Data Requests made to LLC
    this->requests_hive = 0;
    this->requests_vima = 0;
    this->requests_prefetcher = 0; //Data Requests made by prefetcher
    this->row_buffer_miss = 0; //Counter row buffer misses
    this->row_buffer_hit = 0;

    this->channel_bits_mask = 0;
    this->rank_bits_mask = 0;
    this->bank_bits_mask = 0;
    this->row_bits_mask = 0;
    this->colrow_bits_mask = 0;
    this->colbyte_bits_mask = 0;
    this->not_column_bits_mask = 0;
        
    // Shifts bits
    this->channel_bits_shift = 0;
    this->colbyte_bits_shift = 0;
    this->colrow_bits_shift = 0;
    this->bank_bits_shift = 0;
    this->row_bits_shift = 0;
    this->controller_bits_shift = 0;

    this->total_latency = NULL;
    this->total_operations = NULL;
        
    this->CHANNEL = 0;
    this->WAIT_CYCLE = 0;
    this->LINE_SIZE = 0;

    this->CORE_TO_BUS_CLOCK_RATIO = 0.0;
    this->TIMING_AL = 0;     // Added Latency for column accesses
    this->TIMING_CAS = 0;    // Column Access Strobe (CL) latency
    this->TIMING_CCD = 0;    // Column to Column Delay
    this->TIMING_CWD = 0;    // Column Write Delay (CWL) or simply WL
    this->TIMING_FAW = 0;   // Four (row) Activation Window
    this->TIMING_RAS = 0;   // Row Access Strobe
    this->TIMING_RC = 0;    // Row Cycle
    this->TIMING_RCD = 0;    // Row to Column comand Delay
    this->TIMING_RP = 0;     // Row Precharge
    this->TIMING_RRD = 0;    // Row activation to Row activation Delay
    this->TIMING_RTP = 0;    // Read To Precharge
    this->TIMING_WR = 0;    // Write Recovery time
    this->TIMING_WTR = 0;

    this->channels = NULL;
    this->i = 0;

}
// ============================================================================
memory_controller_t::~memory_controller_t(){
    if (use_ramulator) { // TODO

    }


    delete[] this->total_operations;
    delete[] this->total_latency;
    delete[] this->min_wait_operations;
    delete[] this->max_wait_operations;
    delete[] this->channels;
}
// ============================================================================

// ============================================================================
// @allocate objects to#if MEMORY_REQUESTS_DEBUG EMC
void memory_controller_t::allocate() {

    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    int using_ramulator = 0;
    if (!cfg_root.lookupValue("USE_RAMULATOR", using_ramulator)) {
        printf("Using default memory system (USE_RAMULATOR not defined)!\n");
        this->use_ramulator = false;
        this->use_orcs = true;
    } else {
        if (using_ramulator == 0) {
            printf("Using default memory system (USE_RAMULATOR == 0)!\n");
            this->use_ramulator = false;
            this->use_orcs = true;
        } else {
            printf("Using ramulator memory system (USE_RAMULATOR == 1)!\n");
            this->use_ramulator = true;
            this->use_orcs = false;

            // Calculate the ratio (how many Mem ticks per CPU tick)
            // Ratio is usually < 1.0 (e.g., 800/3200 = 0.25)
            // 1. Check for CPU Frequency
            if (!cfg_root.lookupValue("cpu_frequency_mhz", this->cpu_frequency_mhz)) {
                fprintf(stderr, "ERROR: 'cpu_frequency_mhz' not found in configuration file!\n");
                exit(1);
            }

            // 2. Check for Memory Frequency
            if (!cfg_root.lookupValue("memory_frequency_mhz", this->memory_frequency_mhz)) {
                fprintf(stderr, "ERROR: 'memory_frequency_mhz' not found in configuration file!\n");
                exit(1);
            }

            // Optional: Debug print to confirm values are loaded
            printf("Frequencies Loaded: CPU %lf MHz, MEM %lf MHz\n", 
                    this->cpu_frequency_mhz, this->memory_frequency_mhz);
            this->memory_cpu_ratio = this->memory_frequency_mhz / this->cpu_frequency_mhz;
            this->cycles_accumulated = 0.0;
            
        }
    }

    // Memory simulation
    this->use_orcs = use_orcs;
    this->use_ramulator = use_ramulator;



    // Connect to the ramulator and configure it
    if (use_ramulator) {
        std::string ramulator_configuration;

        if (!cfg_root.lookupValue("RAMULATOR_CONFIGURATION", ramulator_configuration)) {
            printf("Ramulator configuration not defined!\n");
            exit(1);
        }
        printf("Using configuration %s for ramulator..\n", ramulator_configuration.c_str());
        connect_to_ramulator(ramulator_configuration);
        printf("Ramulator memory system configured!\n");
    }


    // Configure the OrCS memory system simulation
    if (!use_orcs) {
        return;
    }

    libconfig::Setting &cfg_memory_ctrl = cfg_root["MEMORY_CONTROLLER"];
    libconfig::Setting &cfg_cache_defs = cfg_root["CACHE_MEMORY"];
    
    if (cfg_memory_ctrl.exists("LINE_SIZE")) {
        printf("WARNING: The cache line size should be only defined in the cache configuration. Check your configuration files!\n");
        exit(1);
    }

    set_BANK (cfg_memory_ctrl["BANK"]);
    set_BANK_ROW_BUFFER_SIZE (cfg_memory_ctrl["BANK_ROW_BUFFER_SIZE"]);
    set_CHANNEL (cfg_memory_ctrl["CHANNEL"]);
    if (cfg_memory_ctrl.exists("SUB_CHANNEL")) {
        uint32_t subchannel = (uint32_t) cfg_memory_ctrl["SUB_CHANNEL"];
        set_CHANNEL(get_CHANNEL() * subchannel);
    }

    if (cfg_memory_ctrl.exists("RANK")) {
        set_RANK(cfg_memory_ctrl["RANK"]);
    }

    set_BURST_WIDTH (cfg_memory_ctrl["BURST_WIDTH"]);
    set_LINE_SIZE (cfg_cache_defs["CONFIG"]["LINE_SIZE"]);
    set_WAIT_CYCLE (cfg_memory_ctrl["WAIT_CYCLE"]);
    set_CORE_TO_BUS_CLOCK_RATIO (cfg_memory_ctrl["CORE_TO_BUS_CLOCK_RATIO"]);

    if ((int32_t)cfg_memory_ctrl["LATENCY_BURST_REDUCTION_FACTOR"] < 0) {
        set_cache_line_latency_burst (ceil ((LINE_SIZE/BURST_WIDTH) * this->CORE_TO_BUS_CLOCK_RATIO));
        set_latency_burst (ceil ((min(LINE_SIZE, BANK_ROW_BUFFER_SIZE)/BURST_WIDTH) * this->CORE_TO_BUS_CLOCK_RATIO));
    } else if ((int32_t)cfg_memory_ctrl["LATENCY_BURST_REDUCTION_FACTOR"] == 0) {
        set_cache_line_latency_burst (0);
        set_latency_burst (0);
    } else {
        set_cache_line_latency_burst (ceil ((LINE_SIZE/BURST_WIDTH) * this->CORE_TO_BUS_CLOCK_RATIO / 
                                 ((int32_t)cfg_memory_ctrl["LATENCY_BURST_REDUCTION_FACTOR"] + 0.0)));
        set_latency_burst (ceil ((min(LINE_SIZE, BANK_ROW_BUFFER_SIZE)/BURST_WIDTH) * this->CORE_TO_BUS_CLOCK_RATIO / 
                                 ((int32_t)cfg_memory_ctrl["LATENCY_BURST_REDUCTION_FACTOR"] + 0.0)));
    }

    printf("MEMORY_CONTROLLER_T::set_latency_burst (For channel and banks usage)= %lu\n", this->latency_burst);
    printf("MEMORY_CONTROLLER_T::set_cache_line_latency_burst (for cache installation) = %lu\n", this->cache_line_latency_burst);
    
    this->total_latency = new uint64_t [MEMORY_OPERATION_LAST]();
    this->total_operations = new uint64_t [MEMORY_OPERATION_LAST]();
    this->min_wait_operations = new uint64_t [MEMORY_OPERATION_LAST]();
    for (i = 0; i < MEMORY_OPERATION_LAST; i++) this->min_wait_operations[i] = UINT64_MAX;
    this->max_wait_operations = new uint64_t [MEMORY_OPERATION_LAST]();

    set_TIMING_AL (cfg_memory_ctrl["TIMING_AL"]);     // Added Latency for column accesses
    set_TIMING_CAS (cfg_memory_ctrl["TIMING_CAS"]);    // Column Access Strobe (CL]) latency
    set_TIMING_CCD (cfg_memory_ctrl["TIMING_CCD"]);    // Column to Column Delay
    set_TIMING_CWD (cfg_memory_ctrl["TIMING_CWD"]);    // Column Write Delay (CWL]) or simply WL
    set_TIMING_FAW (cfg_memory_ctrl["TIMING_FAW"]);   // Four (row]) Activation Window
    set_TIMING_RAS (cfg_memory_ctrl["TIMING_RAS"]);   // Row Access Strobe
    set_TIMING_RC (cfg_memory_ctrl["TIMING_RC"]);    // Row Cycle
    set_TIMING_RCD (cfg_memory_ctrl["TIMING_RCD"]);    // Row to Column comand Delay
    set_TIMING_RP (cfg_memory_ctrl["TIMING_RP"]);     // Row Precharge
    set_TIMING_RRD (cfg_memory_ctrl["TIMING_RRD"]);    // Row activation to Row activation Delay
    set_TIMING_RTP (cfg_memory_ctrl["TIMING_RTP"]);    // Read To Precharge
    set_TIMING_WR (cfg_memory_ctrl["TIMING_WR"]);    // Write Recovery time
    set_TIMING_WTR (cfg_memory_ctrl["TIMING_WTR"]);
    
    this->channels = new memory_channel_t[CHANNEL]();
    for (i = 0; i < this->CHANNEL; i++) this->channels[i].allocate();
    for (i = 0; i < this->CHANNEL; i++){
        channels[i].set_latency_burst (this->latency_burst);
        channels[i].set_TIMING_AL (ceil (this->TIMING_AL * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_CAS (ceil (this->TIMING_CAS * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_CCD (ceil (this->TIMING_CCD * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_CWD (ceil (this->TIMING_CWD * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_FAW (ceil (this->TIMING_FAW * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_RAS (ceil (this->TIMING_RAS * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_RC (ceil (this->TIMING_RC * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_RCD (ceil (this->TIMING_RCD * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_RP (ceil (this->TIMING_RP * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_RRD (ceil (this->TIMING_RRD * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_RTP (ceil (this->TIMING_RTP * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_WR (ceil (this->TIMING_WR * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_WTR (ceil (this->TIMING_WTR * this->CORE_TO_BUS_CLOCK_RATIO));
    }
    
    this->set_masks();
    printf("OrCS memory system configured!\n");
}
// ============================================================================
void memory_controller_t::statistics(FILE *output){

    if (use_ramulator) {
        ramulator_statistics_and_finish();
    }

    if (!use_orcs) {
        return;
    }

	if (output != NULL){
        uint32_t total_rb_hits = 0, total_rb_misses = 0;
        utils_t::largestSeparator(output);
        fprintf(output,"#Memory Controller\n");
        utils_t::largestSeparator(output);
        fprintf(output,"Requests_Made:               %lu\n",this->get_requests_made());
        fprintf(output,"Sub_Requests_Made:               %lu\n",this->get_sub_requests_made());
        if (this->get_requests_prefetcher() > 0)  fprintf(output,"Requests_from_Prefetcher:    %lu\n",this->get_requests_prefetcher());
        fprintf(output,"Requests_from_LLC:           %lu\n",this->get_requests_llc());
        if (this->get_requests_hive() > 0) fprintf(output,"Requests_from_HIVE:          %lu\n",this->get_requests_hive());
        if (this->get_requests_vima() > 0) fprintf(output,"Requests_from_VIMA:          %lu\n",this->get_requests_vima());
        for (i = 0; i < CHANNEL; i++){
            if (i > 9) {
                fprintf(output,"Row_Buffer_Hit,  Channel %lu: %lu\n",i,this->channels[i].get_stat_row_buffer_hit());
                fprintf(output,"Row_Buffer_Miss, Channel %lu: %lu\n",i,this->channels[i].get_stat_row_buffer_miss());
            } else {
                fprintf(output,"Row_Buffer_Hit,  Channel  %lu: %lu\n",i,this->channels[i].get_stat_row_buffer_hit());
                fprintf(output,"Row_Buffer_Miss, Channel  %lu: %lu\n",i,this->channels[i].get_stat_row_buffer_miss());
            }
            total_rb_hits += this->channels[i].get_stat_row_buffer_hit();
            total_rb_misses += this->channels[i].get_stat_row_buffer_miss();
        }
        fprintf(output,"Row_Buffer Total_Hits:       %u\n",total_rb_hits);
        fprintf(output,"Row_Buffer_Total_Misses:     %u\n",total_rb_misses);
        fprintf(output,"Row_Buffer_Miss_Ratio:       %f\n", (float) total_rb_misses/this->get_sub_requests_made());
        for (i = 0; i < MEMORY_OPERATION_LAST; i++){
            if (this->total_operations[i] > 0) {
                fprintf(output,"%s_Tot._Latency:          %lu\n", get_enum_memory_operation_char ((memory_operation_t) i), this->total_latency[i]);
                fprintf(output,"%s_Avg._Latency:          %lu\n", get_enum_memory_operation_char ((memory_operation_t) i), this->total_latency[i]/this->total_operations[i]);
                fprintf(output,"%s_Min._Latency:          %lu\n", get_enum_memory_operation_char ((memory_operation_t) i), this->min_wait_operations[i]);
                fprintf(output,"%s_Max._Latency:          %lu\n", get_enum_memory_operation_char ((memory_operation_t) i), this->max_wait_operations[i]);
            }
        }
        utils_t::largestSeparator(output);
    }
}// ============================================================================
void memory_controller_t::reset_statistics(){
    if (use_ramulator) {
        printf("WARNING: reset_statistics not implemented with ramulator2!\n");    
    }

    if (!use_orcs) {
        return;
    }
    this->set_requests_made(0);
    this->set_requests_prefetcher(0);
    this->set_requests_llc(0);
    this->set_requests_hive(0);
    this->set_requests_vima(0);
    for (i = 0; i < CHANNEL; i++){
        this->channels[i].set_stat_row_buffer_hit(0);
        this->channels[i].set_stat_row_buffer_miss(0);
    }
    for (i = 0; i < MEMORY_OPERATION_LAST; i++){
        this->total_operations[i] = 0;
        this->total_latency[i] = 0;
        this->min_wait_operations[i] = 0;
        this->max_wait_operations[i] = 0;
    }
}


// ============================================================================
// Returns OK if there is still a requisition to be finished
bool memory_controller_t::isBusy(){
    if (use_ramulator) {
        if (working.size() != 0) return true;
    }
    if (use_orcs) {
        if (working.size() != 0) return true;
    }
    return false;
}
uint32_t subrequests_on_track = 0;
void memory_controller_t::request_finished(uint64_t addr, memory_operation_t mem_op, uint32_t source_core) {
    // uint64_t addr = (uint64_t)req.addr;
    // memory_operation_t mem_op = (req.type_id == 0) ? MEMORY_OPERATION_READ : MEMORY_OPERATION_WRITE;
    // uint32_t source_core = req.source_id;
    #if MEMORY_DEBUG
    printf("[MEM_CTRL] Sub-request finished: Addr %lu, Op %d, Core %u at cycle %lu\n", 
                    addr, mem_op, source_core, orcs_engine.get_global_cycle());
    #endif
    ERROR_ASSERT_PRINTF(subrequests_on_track > 0, "Subrequests on track below zero!!\n");

    subrequests_on_track--;
    
    // 1. Debug Print
    printf("[CALLBACK RECEIVED] Addr: %lu, Op: %d, Core: %u\n", addr, mem_op, source_core);



    for (i = 0; i < working.size(); i++) {
        if (working[i]->memory_operation == mem_op &&
            working[i]->memory_address == addr &&
            working[i]->processor_id == source_core) {
                // Requisição completa
                working[i]->updatePackageDRAMReady(0);
                printf("Set to ready!\n");
                return;
            }
    }
    
    // 3. Fallback: If no match was found, print the vector to see what went wrong
    printf("ERROR: Match NOT found. Printing working vector for comparison:\n");
    for (uint32_t j = 0; j < working.size(); j++) {
        printf("  - Target [%u] Addr: %lu, Op: %d, Core: %u\n", 
                j, working[j]->memory_address, working[j]->memory_operation, working[j]->processor_id);
    }

    #if MEMORY_DEBUG
    printf("memory_controller_t::request_finished\n");
    #endif

}


// ============================================================================
void memory_controller_t::clock(){
    // ************************************************************************
    // Sending clock to the memory
    // ************************************************************************
    if (use_ramulator) {  
        // 1. Accumulate the "fractional" memory cycle for this CPU tick
        this->cycles_accumulated += this->memory_cpu_ratio;

        // 2. Tick Ramulator as many times as needed to "catch up"
        // While accumulation is >= 1, it means at least one DRAM cycle has passed
        while (this->cycles_accumulated >= 1.0) {
            if (use_ramulator) {
                 send_tick(); 
            }
            this->cycles_accumulated -= 1.0;
        }
        //printf("subrequests_on_track: %d\n", subrequests_on_track);

        // 3. Log
//       if (subrequests_on_track > 0) printf("Waiting for %u sub-requests on track\n", subrequests_on_track);
//       if (working.size() > 0) printf("Waiting for %ld sub-requests\n", working.size());
//           if (ongoing_requests.size() > 0) {
//               printf("Waiting for %ld ongoing requests\n", ongoing_requests.size());
//               for (uint32_t i=0; i < ongoing_requests.size(); ++i) {
//                   printf("%lu\n", ongoing_requests[i]->memory_address);
//                   printf("   waiting for %u subrequests!\n", ongoing_requests[i]->num_subrequests);
//               }

//       }
        
        
        

    }
    if (use_orcs) {
        for (i = 0; i < this->CHANNEL; i++) this->channels[i].clock();
    }

    // ************************************************************************
    // Processing the data
    // ************************************************************************

    if (working.size() == 0) return;

    // -----------------------------------------------------------------------------------------
    // Verifica cada requisição recebida
    // -----------------------------------------------------------------------------------------
    for (i = 0; i < working.size(); i++){
        // -----------------------------------------------------------------------------------------
        // Se ainda não foi buscada, mas está pronta
        // -----------------------------------------------------------------------------------------
        if (working[i]->status != PACKAGE_STATE_DRAM_FETCH && working[i]->status != PACKAGE_STATE_DRAM_READY){
            if (use_ramulator) {
                // -----------------------------------------------------------------------------------------
                // Envia essa requisição para o ramulator
                // Se ele rejeitar, tenta no próximo ciclo
                // -----------------------------------------------------------------------------------------

                subrequests_on_track++; // Deixar aqui para evitar ficar negativo em stores que concluem imediatamente...
                if(!send_request((working[i]->memory_operation == MEMORY_OPERATION_READ), working[i]->memory_address, working[i]->processor_id, this)) {
                  subrequests_on_track--; // Se não conseguiu enviar, reduz o valor novamente
                }
            }
            
            if (use_orcs) {
                // -----------------------------------------------------------------------------------------
                // Envia essa requisição ao canal de memória correto
                // -----------------------------------------------------------------------------------------

                    working[i]->ram_cycle = orcs_engine.get_global_cycle();
                    working[i]->updatePackageDRAMFetch (0);
                }
            }
        // -----------------------------------------------------------------------------------------
        // Se já foi buscada e os dados estão prontos
        // -----------------------------------------------------------------------------------------
        else if (working[i]->status == PACKAGE_STATE_DRAM_READY && working[i]->readyAt <= orcs_engine.get_global_cycle()){
            // -----------------------------------------------------------------------------------------
            // Contabiliza estatísticas
            // -----------------------------------------------------------------------------------------
            wait_time = (orcs_engine.get_global_cycle() - working[i]->ram_cycle);
            #if MEMORY_DEBUG
                ORCS_PRINTF ("[MEMC] %lu %lu %s finishes at main memory! Took %lu cycles.\n", orcs_engine.get_global_cycle(), working[i]->memory_address, get_enum_memory_operation_char (working[i]->memory_operation), wait_time)
            #endif

            working[i]->updatePackageWait (1);
            this->total_operations[working[i]->memory_operation]++;
            if (wait_time < this->min_wait_operations[working[i]->memory_operation]) this->min_wait_operations[working[i]->memory_operation] = wait_time;
            if (wait_time > this->max_wait_operations[working[i]->memory_operation]) this->max_wait_operations[working[i]->memory_operation] = wait_time;
            this->total_latency[working[i]->memory_operation] += wait_time;
            
            // -----------------------------------------------------------------------------------------
            // Atualiza a requisição original
            // -----------------------------------------------------------------------------------------
            #if MEMORY_REQUESTS_DEBUG
            printf("memory_controller_t - requestDRAM - Completing sub-request [Addr: %lu - Size: %u]\n", working[i]->memory_address, working[i]->memory_size);    
            printf("memory_controller_t - requestDRAM - Updating original request [Addr: %lu - Size: %u - Num. Subrequests: %u -> %u]\n", working[i]->subrequest_from[0]->memory_address, working[i]->subrequest_from[0]->memory_size, working[i]->subrequest_from[0]->num_subrequests, working[i]->subrequest_from[0]->num_subrequests - 1);
            #endif

            working[i]->subrequest_from[0]->num_subrequests--;
            //printf("Remaining subrequests for requisition: %u\n", working[i]->subrequest_from[0]->num_subrequests);

            if (working[i]->subrequest_from[0]->num_subrequests == 0) {
                working[i]->subrequest_from[0]->updatePackageWait (this->cache_line_latency_burst);

                #if MEMORY_REQUESTS_DEBUG
                printf("memory_controller_t - requestDRAM - Completing original request [Addr: %lu - Size: %u]\n", working[i]->subrequest_from[0]->memory_address, working[i]->subrequest_from[0]->memory_size);
                #endif
                ongoing_requests.erase(std::remove(ongoing_requests.begin(), ongoing_requests.end(), working[i]->subrequest_from[0]), ongoing_requests.end());
                ongoing_requests.shrink_to_fit();
            }

            
            // -----------------------------------------------------------------------------------------
            // Libera a subrequisição e remove da lista de requisições
            // -----------------------------------------------------------------------------------------
            //printf("Removing sub-request\n");
            delete working[i];
            working.erase(std::remove(working.begin(), working.end(), working[i]), working.end());
            working.shrink_to_fit();

        }
    }
}
// ============================================================================
void memory_controller_t::set_masks(){ 

    if (!use_orcs) {
        return;
    }
    ERROR_ASSERT_PRINTF(CHANNEL > 1 && utils_t::check_if_power_of_two(CHANNEL),"Wrong number of memory_channels (%u).\n",CHANNEL);
    
    this->channel_bits_shift=0;
    this->colbyte_bits_shift=0;
    this->colrow_bits_shift=0;
    
    this->bank_bits_shift=0;
    this->row_bits_shift=0;
    this->colbyte_bits_shift = 0;

    this->channel_bits_mask = 0;
    this->bank_bits_mask = 0;
    this->rank_bits_mask = 0;
    this->row_bits_mask = 0;
    this->colrow_bits_mask = 0;
    this->colbyte_bits_mask = 0;
    // =======================================================
    this->controller_bits_shift = 0;
    this->colbyte_bits_shift = 0;
    this->colrow_bits_shift = utils_t::get_power_of_two(this->LINE_SIZE);
    this->channel_bits_shift = utils_t::get_power_of_two(this->BANK_ROW_BUFFER_SIZE);
    this->bank_bits_shift = this->channel_bits_shift + utils_t::get_power_of_two(this->CHANNEL);
    this->row_bits_shift = this->bank_bits_shift + utils_t::get_power_of_two(this->BANK);
    
    /// COLBYTE MASK
    for (i = 0; i < utils_t::get_power_of_two(this->LINE_SIZE); i++) {
        this->colbyte_bits_mask |= (uint64_t)1 << (i + this->colbyte_bits_shift);
    }

    /// COLROW MASK
    for (i = 0; i < utils_t::get_power_of_two(this->BANK_ROW_BUFFER_SIZE / this->LINE_SIZE); i++) {
        this->colrow_bits_mask |= (uint64_t)1 << (i + this->colrow_bits_shift);
    }

    this->not_column_bits_mask = ~(colbyte_bits_mask | colrow_bits_mask);

    /// CHANNEL MASK
    for (i = 0; i < utils_t::get_power_of_two(this->CHANNEL); i++) this->channel_bits_mask |= 1 << (i + channel_bits_shift);

    /// BANK MASK
    for (i = 0; i < utils_t::get_power_of_two(this->BANK); i++) this->bank_bits_mask |= 1 << (i + bank_bits_shift);

    /// ROW MASK
    for (i = row_bits_shift; i < 64; i++) {
        this->row_bits_mask |= (uint64_t)1 << i;
    }

    // printf("colbyte_bits_mask - colbyte_bits_shift: %lu %lu", this->colbyte_bits_mask, this->colbyte_bits_shift);
    // printf("colrow_bits_mask - colrow_bits_shift: %lu %lu", this->colrow_bits_mask, this->colrow_bits_shift);
    // printf("channel_bits_mask - channel_bits_shift: %lu %lu", this->channel_bits_mask, this->channel_bits_shift);
    // printf("bank_bits_mask - bank_bits_shift: %lu %lu", this->bank_bits_mask, this->bank_bits_shift);
    // printf("row_bits_mask - row_bits_shift: %lu %lu", this->row_bits_mask, this->row_bits_shift);
}
//=====================================================================
uint64_t memory_controller_t::requestDRAM (memory_package_t* request){

    if (request != NULL) {
        this->add_requests_made();
        if (request->is_hive) this->add_requests_hive();
        if (request->is_vima) this->add_requests_vima();
        request->sent_to_ram = true;
        
        #if MEMORY_REQUESTS_DEBUG
        printf("memory_controller_t - requestDRAM - Receiving request [Addr: %lu - Size: %u]\n", request->memory_address, request->memory_size);
        #endif

        ongoing_requests.push_back(request);
        //printf("Adding main request, now with %ld ongoing requests and %ld subrequests\n", ongoing_requests.size(), working.size());

        if (use_ramulator) {
            memory_package_t *subrequest = new memory_package_t();
            subrequest->copy(request);
            subrequest->subrequest_from.push_back(request); // Only one [0]
            subrequest->num_subrequests = 0;
            request->num_subrequests++;

            if (request->is_load_from_write) {
                subrequest->memory_operation = MEMORY_OPERATION_READ;
            }
          
            if (request->memory_operation == MEMORY_OPERATION_INST) {
              subrequest->memory_operation = MEMORY_OPERATION_READ;
            }

            ERROR_ASSERT_PRINTF (subrequest->memory_operation == MEMORY_OPERATION_READ || subrequest->memory_operation == MEMORY_OPERATION_WRITE, "Memory operation type in memory different from READ/WRITE: %d\n", subrequest->memory_operation);



            subrequest->memory_address = request->memory_address;
            subrequest->memory_size = request->memory_size;

            #if MEMORY_REQUESTS_DEBUG
            printf("memory_controller_t - requestDRAM - Generating sub-request [Addr: %lu - Size: %u]\n", subrequest->memory_address, subrequest->memory_size);
            #endif

            this->add_sub_requests_made();
            this->working.push_back (subrequest);
            this->working.shrink_to_fit();
            //printf("Adding subrequests, now with %ld ongoing requests and %ld subrequests\n", ongoing_requests.size(), working.size());
        }

        if (use_orcs) {
            // *********************************************************************
            // Get the first cache line to be loaded
            // *********************************************************************
            uint32_t minimum_unit = (LINE_SIZE < BANK_ROW_BUFFER_SIZE) ? LINE_SIZE : BANK_ROW_BUFFER_SIZE;

            // Align to the unit
            uint64_t base_address = request->memory_address  & (~(((uint64_t)minimum_unit)-1));
            
            // *********************************************************************
            // Get the number of bytes to be loaded
            // Where started in the line, plus the size to be actually loaded
            // *********************************************************************
            uint64_t loaded_bytes = (request->memory_size + (request->memory_address & (((uint64_t)minimum_unit) - 1)));
            
            // *********************************************************************
            // Generate the new sub-requests
            // *********************************************************************
            uint32_t num_subrequests = ceil((loaded_bytes + 0.0f) / minimum_unit);

            //printf("Generating %u subrequests...\n", num_subrequests);
            //printf("Memory address and size: %lu and %u\n", request->memory_address, request->memory_size);
            //printf("Base address and Loaded bytes: %lu and %lu\n", base_address, loaded_bytes);

            for (uint32_t i=0; i < num_subrequests; ++i) {
                memory_package_t *subrequest = new memory_package_t();
                subrequest->copy(request);
                subrequest->subrequest_from.push_back(request); // Only one [0]
                subrequest->num_subrequests = 0;
                request->num_subrequests++;

                if (request->is_load_from_write) {
                    subrequest->memory_operation = MEMORY_OPERATION_READ;
                }

                if (request->memory_operation == MEMORY_OPERATION_INST) {
                  subrequest->memory_operation = MEMORY_OPERATION_READ;
                }

                subrequest->memory_address = base_address;
                subrequest->memory_size = minimum_unit;
                base_address += minimum_unit; // For the next subrequest

                #if MEMORY_REQUESTS_DEBUG
                printf("memory_controller_t - requestDRAM - Generating sub-request [Addr: %lu - Size: %u]\n", subrequest->memory_address, subrequest->memory_size);
                #endif

                this->add_sub_requests_made();
                this->working.push_back (subrequest);
                this->working.shrink_to_fit();
                //printf("Adding subrequests, now with %ld ongoing requests and %ld subrequests\n", ongoing_requests.size(), working.size());

            }
        }

        #if DEBUG
            ORCS_PRINTF ("Memory Controller requestDRAM(): receiving memory request from uop %lu, %s.\n", request->uop_number, get_enum_memory_operation_char (request->memory_operation))
        #endif
        #if MEMORY_DEBUG 
            ORCS_PRINTF ("[MEMC] %lu %lu %s enters.\n", orcs_engine.get_global_cycle(), request->memory_address, get_enum_memory_operation_char (request->memory_operation))
        #endif
        return 0;
    }
    return 0;
}
