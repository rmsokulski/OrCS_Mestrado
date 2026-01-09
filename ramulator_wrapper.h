#ifndef __RAMULATOR_WRAPPER__
#define __RAMULATOR_WRAPPER__
#include "ramulator_lib/base/base.h"
#include "ramulator_lib/base/request.h"
#include "ramulator_lib/base/config.h"
#include "ramulator_lib/frontend/frontend.h"
#include "ramulator_lib/memory_system/memory_system.h"
#include <string>
#include "simulator.hpp"


extern int requests_sent;

extern Ramulator::IFrontEnd* ramulator2_frontend;
extern Ramulator::IMemorySystem* ramulator2_memorysystem;

void connect_to_ramulator(std::string config_path);

void send_tick();

float get_memory_tCK();

int get_clock_ratio();

void read_callback();

void send_request(bool is_read_request, int64_t memory_address, int context_id, memory_controller_t *mem_ctrl);

void ramulator_statistics_and_finish();

#endif