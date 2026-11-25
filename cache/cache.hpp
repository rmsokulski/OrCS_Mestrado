#ifndef CACHE_H
#define CACHE_H
using namespace std;

class cache_t {

    private:
        uint64_t cache_hit;
        uint64_t cache_miss;
        uint64_t cache_access;
        uint64_t cache_eviction;   ///< valid lines replaced (set full, LRU victim)
        uint64_t cache_install;    ///< lines installed (into a free slot or over a victim)
        uint64_t cache_back_invalidation; ///< upper-level copies removed by this cache's evictions
        uint64_t cache_write_merged;  ///< writes merged into a pending request (secondary misses)
        uint64_t cache_write_merged_missing; ///< ... whose line had already left this cache
        uint64_t cache_merged_miss;   ///< misses merged with another core's miss to the same line (no MSHR entry)
        uint64_t cache_read;
        uint64_t cache_write;
        uint64_t cache_inst;
        uint64_t cache_writeback;


        void unlink_line(line_t *line, uint32_t line_level);
        void send_writeback(uint64_t memory_address, uint32_t processor_id);

        uint64_t offset_bits_shift;
        uint64_t index_bits_shift;
        uint64_t tag_bits_shift;

        uint64_t offset_bits_mask;
        uint64_t index_bits_mask;
        uint64_t tag_bits_mask;

        // Get channel to access DATA
        inline uint64_t get_index(uint64_t addr) {
            return (addr & this->index_bits_mask) >> this->index_bits_shift;
        }

        inline uint64_t get_tag(uint64_t addr) {
            return (addr & this->tag_bits_mask) >> this->tag_bits_shift;
        }
        
        uint32_t LINE_SIZE;
        uint32_t PREFETCHER_ACTIVE;
        uint32_t INSTRUCTION_LEVELS;
        uint32_t DATA_LEVELS;
        uint32_t POINTER_LEVELS;
        uint32_t CACHE_MANAGER_DEBUG;
        uint32_t WAIT_CYCLE;

    public:
        cache_t();
        ~cache_t();

        uint32_t id;
        uint32_t level;
        uint32_t size;
        uint32_t latency;
        uint32_t associativity;
        uint32_t n_sets;
        cacheSet_t *sets;
        uint32_t offset;
        int32_t mshr_size;
        int32_t concurrent_cache_accesses;
        int32_t max_reached;
        int32_t mshr_occupied_entries;
        int32_t max_MSHR_reached;
        int32_t num_mshr_stall;
        /* 
        When MSHR is full and receives a new miss, the cache stalls until one of the misses arive.
        */
        bool mshr_stall;

        bool is_inst_cache;

        uint64_t* cache_hit_per_type;
        uint64_t* cache_miss_per_type;
        uint64_t* cache_count_per_type;

        int64_t* total_per_type;
        int64_t* max_per_type;
        int64_t* min_per_type;

        void statistics(FILE *output);
        void reset_statistics();
        void allocate(uint32_t NUMBER_OF_PROCESSORS, uint32_t INSTRUCTION_LEVELS, uint32_t DATA_LEVELS);//allocate data structure
        bool inclusive;   ///< LLC only: evicting a line removes its upper-level copies (INCLUSION in CACHE_MEMORY.CONFIG)
        void evict(line_t *victim, uint32_t processor_id); //remove a valid line before reusing its slot
        void returnLine(memory_package_t* request, cache_t *cache);//return line from lower cache level
        void tagIdxSetCalculation(uint64_t address, uint64_t *idx, uint64_t *tag); //calculate index of data, makes tag from address
        /// Tag and set index of an address, or of the address a line holds, in
        /// this cache (a line stores its address; the tag depends on the level)
        inline uint64_t addr_tag(uint64_t address) { return address >> this->offset; }
        inline uint64_t addr_idx(uint64_t address) { return (address >> this->offset) & (this->n_sets - 1); }
        inline uint64_t line_tag(const line_t *line) { return addr_tag(line->address_line); }
        inline uint64_t line_idx(const line_t *line) { return addr_idx(line->address_line); }
        uint32_t searchLru(cacheSet_t *set);//searh LRU to substitue
        bool peek(uint64_t address);
        uint32_t read(uint64_t address, uint32_t &ttc);
        uint32_t write(memory_package_t* request);
        bool mark_dirty(uint64_t address);
        void printTagIdx(uint64_t address);
        line_t *installLine(memory_package_t* request, uint32_t latency, uint64_t &idx, uint64_t &line); //install line of cache |mem_controller -> caches|

        void print_cache() {
            for (uint32_t i=0; i < n_sets; ++i) {
                printf("SET %u\n", i);
                sets[i].print_lines();
            }
        }

        // Getters and setters
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_access)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_eviction)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_read)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_write)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_inst)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_writeback)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_install)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_back_invalidation)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_write_merged)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_write_merged_missing)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_merged_miss)
        INSTANTIATE_GET_SET_ADD(int32_t,concurrent_cache_accesses)
        INSTANTIATE_GET_SET_ADD(int32_t,max_reached)
        INSTANTIATE_GET_SET_ADD(int32_t,mshr_occupied_entries)
        INSTANTIATE_GET_SET_ADD(int32_t,max_MSHR_reached)
        INSTANTIATE_GET_SET_ADD(int32_t, num_mshr_stall)
        

        INSTANTIATE_GET_SET_ADD(uint32_t,LINE_SIZE)
        INSTANTIATE_GET_SET_ADD(uint32_t,PREFETCHER_ACTIVE)
        INSTANTIATE_GET_SET_ADD(uint32_t,INSTRUCTION_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t,DATA_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t,POINTER_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t,CACHE_MANAGER_DEBUG)
        INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_CYCLE)
        INSTANTIATE_GET_SET(bool, mshr_stall)
};

#endif // CACHE_H
