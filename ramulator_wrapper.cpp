#include "ramulator_wrapper.h"

int requests_sent;
Ramulator::IFrontEnd* ramulator2_frontend;
Ramulator::IMemorySystem* ramulator2_memorysystem;

void connect_to_ramulator(std::string config_path) {
    YAML::Node config = Ramulator::Config::parse_config_file(config_path, {});
    ramulator2_frontend = Ramulator::Factory::create_frontend(config);
    ramulator2_memorysystem = Ramulator::Factory::create_memory_system(config);

    ramulator2_frontend->connect_memory_system(ramulator2_memorysystem);
    ramulator2_memorysystem->connect_frontend(ramulator2_frontend);
}

float get_memory_tCK() {
    float memory_tCK = ramulator2_memorysystem->get_tCK();
    return memory_tCK;
}

int get_clock_ratio() {
  return ramulator2_memorysystem->get_clock_ratio();
}

void send_tick() {
  ramulator2_frontend->tick();
  ramulator2_memorysystem->tick();

}

// context_id iditifies which core is sending the request (https://github.com/CMU-SAFARI/ramulator2/blob/main/src/base/request.h)
bool send_request(bool is_read_request, int64_t memory_address, int context_id, memory_controller_t *mem_ctrl) {
    // 1. Determine the request type based on Ramulator 2's specific type IDs
    // 0 is Read and 1 is Write
    int req_type = is_read_request ? 0 : 1; 

    
    // 2. Attempt to enqueue the request
    bool enqueue_success = ramulator2_frontend->receive_external_requests(
        req_type, 
        memory_address, 
        context_id, 
        [mem_ctrl](Ramulator::Request& req) {
            // --- CALLBACK LOGIC (Executes when DRAM finishes) ---
            uint64_t addr = (uint64_t)req.addr;
            
            // Map Ramulator type back to OrCS type
            memory_operation_t mem_op = (req.type_id == 0) ? 
                                         MEMORY_OPERATION_READ : 
                                         MEMORY_OPERATION_WRITE;
            
            uint32_t source_core = req.source_id;

            #if MEMORY_DEBUG
            printf("[RAMULATOR2] Callback for Addr: %lu\n", addr);
            printf("Ramulator request finished!\n");
            #endif

            // Notify the memory controller that this specific address is done
            mem_ctrl->request_finished(addr, mem_op, source_core);
        }
    );

    // Manually calling the callback for writes (Ramulator never calls)
    if (is_read_request == false && enqueue_success) {
      
      #if MEMORY_DEBUG
      printf("[RAMULATOR2] Callback for Addr: %lu\n", addr);
      printf("Ramulator request finished!\n");
      #endif

      mem_ctrl->request_finished(memory_address, MEMORY_OPERATION_WRITE, context_id);
    }


    //printf("Enqueue success: %s\n", enqueue_success ? "true" : "false");
    // 3. Return the status to OrCS
    if (!enqueue_success) {
        // If Ramulator is full, OrCS needs to know
        return false;
    }


    return true;
}

void ramulator_statistics_and_finish() {
  ramulator2_frontend->finalize();
  ramulator2_memorysystem->finalize();
}
