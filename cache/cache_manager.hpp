#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H
using namespace std;

class cache_manager_t {

    private:
        uint64_t i;
        uint64_t reads;
        uint64_t read_miss;
        uint64_t read_hit;
        uint64_t writes;
        uint64_t write_miss;
        uint64_t write_hit;
        uint64_t offset;
        uint64_t mshr_index;

        uint64_t min_sent_ram;
        uint64_t max_sent_ram;
        uint64_t sent_ram;
        uint64_t sent_ram_cycles;
        uint64_t sent_hive;
        uint64_t sent_hive_cycles;
        uint64_t sent_vima;
        uint64_t sent_vima_cycles;

        uint64_t max_vima;

        uint64_t* total_latency;
        uint64_t* total_operations;
        uint64_t* min_wait_operations;
        uint64_t* max_wait_operations;
        uint64_t wait_time;

        uint32_t LINE_SIZE;
        uint32_t PREFETCHER_ACTIVE;
        uint32_t DATA_LEVELS;
        uint32_t INSTRUCTION_LEVELS;
        uint32_t POINTER_LEVELS;
        uint32_t LLC_CACHES;
        uint32_t CACHE_MANAGER_DEBUG;
        uint32_t WAIT_CYCLE;

        uint32_t NUMBER_OF_PROCESSORS;
        uint32_t MAX_PARALLEL_REQUESTS_CORE;


        std::vector<memory_package_t*> ongoing_requests;
        std::vector<memory_package_t*> requests;


        // Statistics
        uint64_t prefetch_requests_sent;

        cache_t **instantiate_cache(cacheId_t cache_type, libconfig::Setting &cfg_cache_defs);
        void get_cache_levels(cacheId_t cache_type, libconfig::Setting &cfg_cache_defs);
        void get_cache_info(cacheId_t cache_type, libconfig::Setting &cfg_cache_defs, cache_t *cache, uint32_t cache_level, uint32_t CACHE_AMOUNT);
        void check_cache(uint32_t cache_size, uint32_t cache_level);
        void print_requests();
        bool isIn (memory_package_t* subrequest, memory_package_t* request);
        void installCacheLines(memory_package_t* request, int32_t *cache_indexes, uint32_t latency_request, cacheId_t cache_type);
        uint32_t searchAddress(uint64_t instructionAddress, cache_t *cache, uint32_t *latency_request, uint32_t *ttc);
        cache_status_t recursiveInstructionSearch(memory_package_t *mob_line, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, uint32_t cache_level);
        cache_status_t instructionSearch(memory_package_t *request, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc);
        cache_status_t recursiveDataSearch(memory_package_t *mob_line, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, uint32_t cache_level, cacheId_t cache_type);
        cache_status_t dataSearch(memory_package_t *request, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc);
        cache_status_t recursiveDataWrite(memory_package_t *mob_line, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, uint32_t cache_level, cacheId_t cache_type);
        cache_status_t cache_search (memory_package_t* request, cache_t* cache, int32_t* cache_indexes);
        void process (memory_package_t* request, int32_t* cache);
        void requestCache (memory_package_t* request);
        void finishRequest (memory_package_t* request, int32_t* cache_indexes, std::vector<memory_package_t *>* requests_list);
        void install (memory_package_t* request);

    public:
        // instruction and data caches dynamically allocated
        cache_t **data_cache;
        cache_t **instruction_cache;
        uint32_t *ICACHE_AMOUNT;
        uint32_t *DCACHE_AMOUNT;

        uint64_t** op_count;
        uint64_t* op_max;

        bool print_rob;

        cache_manager_t();
        ~cache_manager_t();
        void allocate(uint32_t NUMBER_OF_PROCESSORS);
        bool isBusy();
        void clock();//for prefetcher
        void statistics(FILE *output, uint32_t core_id);
        void reset_statistics(uint32_t core_id);
        void generateIndexArray(uint32_t processor_id, int32_t *cache_indexes);
        bool searchData(memory_package_t *mob_line);
        bool available(uint32_t processor_id, memory_operation_t op);
        
        // Getters and setters
        INSTANTIATE_GET_SET_ADD(uint64_t, reads)
        INSTANTIATE_GET_SET_ADD(uint64_t, read_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t, read_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, writes)
        INSTANTIATE_GET_SET_ADD(uint64_t, write_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t, write_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, offset)

        INSTANTIATE_GET_SET_ADD(uint64_t, min_sent_ram)
        INSTANTIATE_GET_SET_ADD(uint64_t, max_sent_ram)
        INSTANTIATE_GET_SET_ADD(uint64_t, sent_ram)
        INSTANTIATE_GET_SET_ADD(uint64_t, sent_ram_cycles)
        INSTANTIATE_GET_SET_ADD(uint64_t, sent_hive)
        INSTANTIATE_GET_SET_ADD(uint64_t, sent_hive_cycles)
        INSTANTIATE_GET_SET_ADD(uint64_t, sent_vima)
        INSTANTIATE_GET_SET_ADD(uint64_t, sent_vima_cycles)

        INSTANTIATE_GET_SET_ADD(uint32_t, LINE_SIZE)
        INSTANTIATE_GET_SET_ADD(uint32_t, PREFETCHER_ACTIVE)

        INSTANTIATE_GET_SET_ADD(uint32_t, DATA_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t, INSTRUCTION_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t, POINTER_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t, LLC_CACHES)
        INSTANTIATE_GET_SET_ADD(uint32_t, CACHE_MANAGER_DEBUG)
        INSTANTIATE_GET_SET_ADD(uint32_t, WAIT_CYCLE)

        INSTANTIATE_GET_SET_ADD(uint32_t, NUMBER_OF_PROCESSORS)
        INSTANTIATE_GET_SET_ADD(uint32_t, MAX_PARALLEL_REQUESTS_CORE)


        prefetcher_t *prefetcher;
};  

#endif // !CACHE_MANAGER_H
