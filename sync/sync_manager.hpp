#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

/// Synchronization shared by all cores: one barrier and one lock.
/// Cores call it from fetch() when the trace has a $N sync marker.
class sync_manager_t {
    private:
        uint32_t active_cores;   ///< cores that must arrive to open a barrier
        uint32_t arrived;        ///< cores waiting at the current barrier
        uint64_t generation;     ///< number of barriers opened so far
        int32_t lock_owner;      ///< core holding the critical section, -1 = free
        void open_if_complete();

    public:
        sync_manager_t();
        void allocate(uint32_t NUMBER_OF_PROCESSORS);

        /// Arrive at the barrier; returns the generation to wait for.
        uint64_t barrier_arrive();
        /// True once the barrier of that generation has opened.
        bool barrier_released(uint64_t gen) { return this->generation > gen; }

        /// Enter the critical section; false while another core holds it.
        bool lock_acquire(uint32_t core);
        void lock_release(uint32_t core);

        /// A core finished its trace: it counts as arrived at every later
        /// barrier, so the other cores do not wait for it forever.
        void retire(uint32_t core);
};

#endif // SYNC_MANAGER_H
