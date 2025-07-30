#include "./../simulator.hpp"
#include <string>

memory_package_t::memory_package_t() {
    this->processor_id = 0;                  /// if (read / write) PROCESSOR.ID   else if (write-back / prefetch) CACHE_MEMORY.ID
    this->opcode_number = 0;                 /// initial opcode number
    this->opcode_address = 0;                /// initial opcode address
    this->uop_number = 0;                    /// initial uop number (Instruction == 0)
    this->memory_address = 0;                /// memory address
    this->memory_size = 0;                   /// operation size after offset

    status = PACKAGE_STATE_FREE;                  /// package state
    this->readyAt = 0;                   /// package latency
    this->born_cycle = 0;                    /// package create time
    this->ram_cycle = 0;
    this->vima_cycle = 0;
    this->hive_cycle = 0;
       
    sent_to_ram = false;
    next_level = L1;
    cache_latency = 0;
    is_hive = false;
    hive_read1 = 0;
    hive_read2 = 0;
    hive_write = 0;

    this->is_vima = false;
    this->vima_read1 = 0;
    this->vima_read1_vec = NULL;
    this->vima_read2 = 0;
    this->vima_read2_vec = NULL;
    this->vima_write = 0;
    this->vima_write_vec = NULL;

    this->is_vectorial_part = -1;
    this->free = true;

    row_buffer = false;
    type = DATA;
    op_count = new uint64_t[MEMORY_OPERATION_LAST]();
    sent_to_cache_level = new uint32_t[END]();
    sent_to_cache_level_at = new uint32_t[END]();
    this->latency = 0;

    this->subrequest_from = std::vector<memory_package_t*>();
    this->num_subrequests = 0;

    memory_operation = MEMORY_OPERATION_LAST;    /// memory operation
}

memory_package_t::~memory_package_t(){
    vector<memory_request_client_t*>().swap(this->clients);
    delete[] op_count;
    delete[] sent_to_cache_level;
    delete[] sent_to_cache_level_at;
}

void memory_package_t::updatePackageUntreated (uint32_t stallTime){
    #if MEMORY_DEBUG 
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_UNTREATED;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::updatePackageReady(){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))        
    #endif
    this->status = PACKAGE_STATE_READY;
    this->readyAt = orcs_engine.get_global_cycle();
    #if MEMORY_DEBUG 
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency)
    #endif
}

void memory_package_t::updatePackageWait (uint32_t stallTime){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_WAIT;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::updatePackageTransmit (uint32_t stallTime){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_TRANSMIT;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::updatePackageFree (uint32_t stallTime){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_FREE;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::updatePackageHive (uint32_t stallTime){
    #if MEMORY_DEBUG || HIVE_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_HIVE;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG || HIVE_DEBUG 
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::updatePackageVima (uint32_t stallTime){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_VIMA;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::updatePackageDRAMFetch (uint32_t stallTime){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_DRAM_FETCH;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::updatePackageDRAMReady (uint32_t stallTime){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_DRAM_READY;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::printPackage(){
    ORCS_PRINTF ("%lu Address: %lu | Operation: %s | Status: %s | Uop: %lu | ReadyAt: %lu\n", orcs_engine.get_global_cycle(), memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status), uop_number, readyAt)
}

void memory_package_t::updateClients(){
    #if MEMORY_REQUESTS_DEBUG
    printf("memory_package_t - updateClients - Updating clients from request [Addr: %lu - Size: %u]\n", this->memory_address, this->memory_size);
    #endif
    
    for (size_t i = 0; i < clients.size(); i++) {
        clients[i]->updatePackageReady (0);
    }
}

void memory_package_t::package_clean() {
    this->processor_id = 0;                  /// if (read / write) PROCESSOR.ID   else if (write-back / prefetch) CACHE_MEMORY.ID
    this->opcode_number = 0;                 /// initial opcode number
    this->opcode_address = 0;                /// initial opcode address
    this->uop_number = 0;                    /// initial uop number (Instruction == 0)
    this->memory_address = 0;                /// memory address
    this->memory_size = 0;                   /// operation size after offset

    status = PACKAGE_STATE_FREE;                  /// package state
    this->readyAt = 0;                   /// package latency
    this->born_cycle = 0;                    /// package create time
    this->ram_cycle = 0;
    this->vima_cycle = 0;
    this->hive_cycle = 0;
       
    sent_to_ram = false;
    next_level = L1;
    cache_latency = 0;
    is_hive = false;
    hive_read1 = 0;
    hive_read2 = 0;
    hive_write = 0;

    this->is_vima = false;
    this->vima_read1 = 0;
    this->vima_read1_vec = NULL;
    this->vima_read2 = 0;
    this->vima_read2_vec = NULL;
    this->vima_write = 0;
    this->vima_write_vec = NULL;

    this->is_vectorial_part = -1;

    row_buffer = false;
    type = DATA;

    for (int i=0; i < MEMORY_OPERATION_LAST; ++i) { op_count[i] = 0; }
    for (int i=0; i < END; ++i) { sent_to_cache_level[i] = 0; }
    for (int i=0; i < END; ++i) { sent_to_cache_level_at[i] = 0; }

    this->latency = 0;

    this->subrequest_from = std::vector<memory_package_t*>();
    this->num_subrequests = 0;

    memory_operation = MEMORY_OPERATION_LAST;    /// memory operation
}

void memory_package_t::copy(memory_package_t *other) {
    this->processor_id   = other->processor_id;
    this->opcode_number  = other->opcode_number;
    this->opcode_address = other->opcode_address;
    this->uop_number     = other->uop_number;
    this->memory_address = other->memory_address;
    this->memory_size    = other->memory_size;

    this->status     = other->status;
    this->readyAt    = other->readyAt;
    this->born_cycle = other->born_cycle;
    this->ram_cycle  = other->ram_cycle;
    this->vima_cycle = other->vima_cycle;
    this->hive_cycle = other->hive_cycle;
       
    sent_to_ram      = other->sent_to_ram;
    next_level       = other->next_level;
    cache_latency    = other->cache_latency;
    is_hive          = other->is_hive;
    hive_read1       = other->hive_read1;
    hive_read2       = other->hive_read2;
    hive_write       = other->hive_write;

    this->is_vima        = other->is_vima;
    this->vima_read1     = other->vima_read1;
    this->vima_read1_vec = other->vima_read1_vec;
    this->vima_read2     = other->vima_read2;
    this->vima_read2_vec = other->vima_read2_vec;
    this->vima_write     = other->vima_write;
    this->vima_write_vec = other->vima_write_vec;

    this->is_vectorial_part = other->is_vectorial_part;

    row_buffer = other->row_buffer;
    type       = other->type;

    for (int i=0; i < MEMORY_OPERATION_LAST; ++i) { op_count[i] = other->op_count[i]; }
    for (int i=0; i < END; ++i) { sent_to_cache_level[i] = other->sent_to_cache_level[i]; }
    for (int i=0; i < END; ++i) { sent_to_cache_level_at[i] = other->sent_to_cache_level_at[i]; }

    this->latency    = other->latency;

    memory_operation = other->memory_operation;    /// memory operation

    // This must not be copied!
    // this->subrequest_from = std::vector<memory_package_t*>();
    // this->num_subrequests = other->num_subrequests;

    // this->free = other->free;
}
