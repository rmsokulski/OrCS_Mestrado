
class vima_prefetcher_t {
        circular_buffer_t<conversion_status_t> prefetches;
    public:
        vima_prefetcher_t() {
            assert (VIMA_CONVERSION_PREFETCHER_SIZE >= PREFETCH_SIZE);
            prefetches.allocate(VIMA_CONVERSION_PREFETCHER_SIZE);
        }

        void make_prefetch(conversion_status_t *prev_conversion);

        conversion_status_t* get_prefetch();

        void pop_prefetch();

        void vima_execution_completed(memory_package_t *vima_package, uint64_t readyAt);

        void shift_sequential_conversion(conversion_status_t *status);

};