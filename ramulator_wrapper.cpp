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
  ramulator2_memorysystem->tick();
}

void read_callback() {
    printf("Read completed! - %f\n", get_memory_tCK());
    requests_sent--;
}

void send_request(bool is_read_request, int64_t memory_address, int context_id, memory_controller_t *mem_ctrl) {
    if (is_read_request) {
      printf("Receiving read request...");
      bool enqueue_success = ramulator2_frontend->receive_external_requests(0, memory_address, context_id, [mem_ctrl](Ramulator::Request& req) {
      // your read request callback 
      printf("Callback received!");
      uint64_t addr = (uint64_t)req.addr;
      memory_operation_t mem_op = (req.type_id == 0) ? MEMORY_OPERATION_READ : MEMORY_OPERATION_WRITE;
      uint32_t source_core = req.source_id;
      mem_ctrl->request_finished(addr, mem_op, source_core);
    });

  if (enqueue_success) {
    // What happens if the memory request is accepted by Ramulator 2.0
    printf("Ramulator received the request!\n");
  } else {
    // What happens if the memory request is rejected by Ramulator 2.0 (e.g., request queue full)
    printf("Ramulator requested the request!\n");
  }
}
}

void ramulator_statistics_and_finish() {
  ramulator2_frontend->finalize();
  ramulator2_memorysystem->finalize();
}