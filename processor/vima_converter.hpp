class vima_converter_t {
    public:
        uint32_t iteration;
        // State machine
        // 0 -> Waiting first avx 256 load
        // 1 -> Waiting second avx 256 load
        // 2 -> Waiting operation
        // 3 -> Waiting avx 256 store
        uint8_t state_machine;


        // Instructions info
        // [Iter 0] Load 1 -> 0
        // [Iter 0] Load 2 -> 1
        // [Iter 0] Op     -> 2
        // [Iter 0] Store  -> 3
        // --------------------
        // [Iter 1] Load 1 -> 4
        // [Iter 1] Load 2 -> 5
        // [Iter 1] Op     -> 6
        // [Iter 1] Store  -> 7

        uint64_t addr[8];
        uint8_t uop_id[8];
        uint64_t mem_addr[8];
        uint8_t infos_remaining; // Addresses needed from AGU to generate VIMA instruction
        uint64_t unique_conversion_id;
        int64_t mem_addr_confirmations_remaining;
	bool is_mov;


        bool regs_list[259];

        bool conversion_started; // Conversion enabled, pattern found and first iteration to alignment calculated
        uint64_t conversion_beginning; // First iteration to convert
        uint64_t conversion_ending;    // Last iteration to convert
        bool vima_sent;
        bool CPU_requirements_meet;
        bool VIMA_requirements_meet;



        // ***********************
        // Launch vima instruction
        // ***********************
        circular_buffer_t<uop_package_t> vima_instructions_buffer;

        // *******************
        // VIMA configurations
        // *******************
        uint32_t mem_operation_latency;
        uint32_t mem_operation_wait_next;
        functional_unit_t *mem_operation_fu;
        uint32_t necessary_AVX_256_iterations_to_one_vima;
        uint32_t necessary_AVX_512_iterations_to_one_vima;


        // **********
        // Statistics
        // **********
        uint64_t vima_instructions_launched;

        uint64_t conversion_failed;
        uint64_t conversion_successful;
        uint64_t instructions_intercepted;
        uint64_t instructions_intercepted_until_commit;
        uint64_t instructions_reexecuted;

        uint64_t AGU_result_from_wrong_conversion;
        uint64_t AGU_result_from_current_conversion;

        uint64_t AVX_256_to_VIMA_conversions;
        uint64_t AVX_512_to_VIMA_conversions;

        uint64_t time_waiting_for_infos;
            uint64_t time_waiting_for_infos_start;
            uint64_t time_waiting_for_infos_stop;



        vima_converter_t() {
            this->iteration = 0;
            this->state_machine = 0;
            this->conversion_started = false;
            this->conversion_beginning = 0;
            this->conversion_ending = 0;
            this->vima_sent = false;
            this->CPU_requirements_meet = false;
            this->VIMA_requirements_meet = false;
            this->vima_instructions_buffer.allocate(1);
            this->infos_remaining = 0;
            this->unique_conversion_id = 1;
	    this->is_mov = false;

            // Write register control
            for (uint32_t i=0; i < 259; ++i) regs_list[i] = false;


            // Statistics
            this->vima_instructions_launched = 0;

            this->conversion_failed = 0;
            this->conversion_successful = 0;

            this->instructions_intercepted = 0;
            this->instructions_intercepted_until_commit = 0;
            this->instructions_reexecuted = 0;

            this->AGU_result_from_wrong_conversion = 0;
            this->AGU_result_from_current_conversion = 0;

            this->AVX_256_to_VIMA_conversions = 0;
            this->AVX_512_to_VIMA_conversions = 0;
            
            this->time_waiting_for_infos = 0;
            this->time_waiting_for_infos_start = 0;
            this->time_waiting_for_infos_stop = 0;


        }

        void initialize(uint32_t mem_operation_latency, uint32_t mem_operation_wait_next, functional_unit_t *mem_operation_fu, uint32_t VIMA_SIZE) {
            this->mem_operation_latency = mem_operation_latency;
            this->mem_operation_wait_next = mem_operation_wait_next;
            this->mem_operation_fu = mem_operation_fu;
            assert (VIMA_SIZE % AVX_256_SIZE == 0);
            assert (VIMA_SIZE % AVX_512_SIZE == 0);
            this->necessary_AVX_256_iterations_to_one_vima = (VIMA_SIZE/AVX_256_SIZE) + 1 /* Get address data */;
            this->necessary_AVX_512_iterations_to_one_vima = (VIMA_SIZE/AVX_512_SIZE) + 1 /* Get address data */;

        }

        void generate_VIMA_instruction(uint32_t instruction_converted_size);
        instruction_operation_t define_vima_operation();

        inline void start_new_conversion();


        // Conversion went wrong
        void invalidate_conversion();

        // Conversion address
        void AGU_result(uop_package_t *);
        void get_index_for_alignment(uint32_t access_size);

        // Completed execution
        void vima_execution_completed(memory_package_t *vima_package);


        // Statistics
        inline void statistics(FILE *);
};

inline void vima_converter_t::start_new_conversion() {
    printf("***************************************************\n");
    printf("Resetting vima converter to start new conversion...\n");
    printf("***************************************************\n");
    this->iteration = 0;
    this->state_machine = 0;
    

    // Conversion data
    for (uint32_t i=0; i < 8; ++i) {
        this->addr[i] = 0;
        this->uop_id[i] = 0;
        this->mem_addr[i] = 0;
    }
    this->infos_remaining = 0;
    ++this->unique_conversion_id;
    this->mem_addr_confirmations_remaining = 0;

    // Write register control
    for (uint32_t i=0; i < 259; ++i) regs_list[i] = false;

    this->conversion_started = false;
    this->conversion_beginning = 0;
    this->conversion_ending = 0;
    this->vima_sent = false;
    this->CPU_requirements_meet = false;
    this->VIMA_requirements_meet = false;
    this->is_mov = false;
}


// Statistics
inline void vima_converter_t::statistics(FILE *output) {
    fprintf(output, "vima_instructions_launched: %lu\n", this->vima_instructions_launched);
    fprintf(output, "AVX_256_to_VIMA_conversions: %lu\n", this->AVX_256_to_VIMA_conversions);
    fprintf(output, "AVX_512_to_VIMA_conversions: %lu\n", this->AVX_512_to_VIMA_conversions);
    fprintf(output, "conversion_failed: %lu\n", conversion_failed); 
    fprintf(output, "conversion_successful: %lu\n", conversion_successful); 
    fprintf(output, "instructions_intercepted: %lu\n", instructions_intercepted); 
    fprintf(output, "instructions_intercepted_until_commit: %lu\n", instructions_intercepted_until_commit); 
    fprintf(output, "instructions_reexecuted: %lu\n", instructions_reexecuted); 
    fprintf(output, "AGU_result_from_current_conversion: %lu\n", AGU_result_from_current_conversion); 
    fprintf(output, "AGU_result_from_wrong_conversion: %lu\n", AGU_result_from_wrong_conversion);
    fprintf(output, "time_waiting_for_infos: %lu\n", time_waiting_for_infos);
}
