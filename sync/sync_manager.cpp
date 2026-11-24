#include "../simulator.hpp"

// =====================================================================
sync_manager_t::sync_manager_t() {
    this->active_cores = 0;
    this->arrived = 0;
    this->generation = 0;
    this->lock_owner = -1;
}

// =====================================================================
void sync_manager_t::allocate(uint32_t NUMBER_OF_PROCESSORS) {
    this->active_cores = NUMBER_OF_PROCESSORS;
}

// =====================================================================
uint64_t sync_manager_t::barrier_arrive() {
    uint64_t gen = this->generation;
    this->arrived++;
    /// Last core in: open the barrier for everyone waiting on gen
    if (this->arrived >= this->active_cores) {
        this->arrived = 0;
        this->generation++;
    }
    return gen;
}

// =====================================================================
bool sync_manager_t::lock_acquire(uint32_t core) {
    if (this->lock_owner == -1) {
        this->lock_owner = core;
    }
    return this->lock_owner == (int32_t) core;
}

// =====================================================================
void sync_manager_t::lock_release(uint32_t core) {
    ERROR_ASSERT_PRINTF(this->lock_owner == (int32_t) core, "Core %u released a lock held by core %d\n", core, this->lock_owner);
    this->lock_owner = -1;
}
