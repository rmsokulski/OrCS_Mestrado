class conversion_status_t {
    public:
        uint64_t unique_conversion_id;
        bool conversion_started; // Conversion enabled, pattern found and first iteration to alignment calculated
        uint64_t conversion_beginning; // First iteration to convert
        uint64_t conversion_ending;    // Last iteration to convert

        uint8_t infos_remaining; // Addresses needed from AGU to generate VIMA instruction
        int64_t mem_addr_confirmations_remaining;


        uint64_t base_addr[4];
        uint8_t base_uop_id[4];
        uint64_t base_mem_addr[4]; // 0 -> Ld1; 1 -> Ld2; 2 -> Op [NULL]; 3 -> St
        uint32_t mem_size; // AVX-256 (32) or AVX-512 (64)

        bool vima_sent;
        bool CPU_requirements_meet;
        bool VIMA_requirements_meet;

        bool entry_can_be_removed; // Indicative that the operations related with this conversion are completed

        bool is_mov; // (is_mov) ? (ld1 -> st) : (Ld1 + Ld 2 -> Op -> St)
    
        void package_clean() {
            this->unique_conversion_id = 0;
            this->conversion_started = false;
            this->conversion_beginning = 0;
            this->conversion_ending = 0;

            this->infos_remaining = 0;
            this->mem_addr_confirmations_remaining = 0;

            for (uint8_t i=0; i<4; ++i) {
                this->base_addr[i] = 0x0;
                this->base_uop_id[i] = 0x0;
                this->base_mem_addr[i] = 0x0;
            }

            this->mem_size = 0;

            this->vima_sent = false;
            this->CPU_requirements_meet = false;
            this->VIMA_requirements_meet = false;

            this->entry_can_be_removed = false;

            this->is_mov = false;

        }

};

class back_list_entry_t {
    public:
        uint64_t pc;
        uint8_t uop_id;

        back_list_entry_t () {
            pc = 0x0;
            uop_id = 0;
        }

        void package_clean() {
            pc = 0x0;
            uop_id = 0;
        }
};

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
        uint64_t next_unique_conversion_id;
        conversion_status_t *current_conversion;

        bool regs_list[259];


        circular_buffer_t<conversion_status_t> current_conversions;

        // ***********
        // Black list
        // ***********
        circular_buffer_t<back_list_entry_t> conversions_blacklist;



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
        uint64_t original_program_instructions;

        uint64_t conversion_entry_allocated;
        uint64_t not_enough_conversion_entries;


        uint64_t AGU_result_from_wrong_conversion;
        uint64_t AGU_result_from_current_conversion;

        uint64_t AVX_256_to_VIMA_conversions;
        uint64_t AVX_512_to_VIMA_conversions;


        uint64_t conversions_blacklisted;
        uint64_t avoided_by_the_blacklist;

        uint64_t time_waiting_for_infos;
            uint64_t time_waiting_for_infos_start;
            uint64_t time_waiting_for_infos_stop;



        vima_converter_t() {
            this->iteration = 0;
            this->state_machine = 0;
            this->vima_instructions_buffer.allocate(1);


            // Write register control
            for (uint32_t i=0; i < 259; ++i) regs_list[i] = false;

            // Conversions data
            this->next_unique_conversion_id = 1;
            this->current_conversion = NULL;
            this->current_conversions.allocate(CURRENT_CONVERSIONS_SIZE); // Greater than any ROB could support
            this->conversions_blacklist.allocate(CONVERSION_BLACKLIST_SIZE);


            // Statistics
            this->vima_instructions_launched = 0;

            this->conversion_failed = 0;
            this->conversion_successful = 0;

            this->instructions_intercepted = 0;
            this->instructions_intercepted_until_commit = 0;
            this->instructions_reexecuted = 0;
            this->original_program_instructions = 0;

            this->conversion_entry_allocated = 0;
            this->not_enough_conversion_entries = 0;


            this->AGU_result_from_wrong_conversion = 0;
            this->AGU_result_from_current_conversion = 0;

            this->AVX_256_to_VIMA_conversions = 0;
            this->AVX_512_to_VIMA_conversions = 0;

            this->conversions_blacklisted = 0;
            this->avoided_by_the_blacklist = 0;

            this->time_waiting_for_infos = 0;
            this->time_waiting_for_infos_start = 0;
            this->time_waiting_for_infos_stop = 0;

            // *******************************
			// Create a new conversion manager
			// *****************************
			this->start_new_conversion();

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

        void generate_VIMA_instruction(conversion_status_t *conversion_data);
        instruction_operation_t define_vima_operation(conversion_status_t *conversion_data);

        inline void start_new_conversion();
        void continue_conversion(conversion_status_t *prev_conversion); // After a successfull conversion tries to convert the following loop iterations


        // Conversion went wrong
        void invalidate_conversion(conversion_status_t *invalidated_conversion);

        // Conversion address
        void AGU_result(uop_package_t *);
        void get_index_for_alignment(conversion_status_t * current_conversion, uint32_t access_size);

        // Completed execution
        void vima_execution_completed(memory_package_t *vima_package);

        // Black list
        bool is_blacklisted(uop_package_t *uop);

        // Statistics
        inline void statistics(FILE *);
};

inline void vima_converter_t::start_new_conversion() {
#if VIMA_CONVERSION_DEBUG == 1
    printf("***************************************************\n");
    printf("Resetting vima converter to start new conversion...\n");
    printf("***************************************************\n");

    printf("Trying to allocate a new converter entry...\n");
#endif
    conversion_status_t new_conversion;
    new_conversion.package_clean();
    int32_t entry = this->current_conversions.push_back(new_conversion);
    if (entry != -1) {
#if VIMA_CONVERSION_DEBUG == 1
        printf("Entry allocated!\n");
#endif
        this->conversion_entry_allocated++;
        this->current_conversion = &this->current_conversions[entry];

        this->iteration = 0;
        this->state_machine = 0;

        // Write register control
        for (uint32_t i=0; i < 259; ++i) regs_list[i] = false;


        // Conversion data
        for (uint32_t i=0; i < 8; ++i) {
            this->addr[i] = 0;
            this->uop_id[i] = 0;
            this->mem_addr[i] = 0;
        }

#if VIMA_CONVERSION_DEBUG == 1
    printf("New conversion with conversion id: %lu\n", next_unique_conversion_id);
#endif
        this->current_conversion->unique_conversion_id = next_unique_conversion_id++;

        this->current_conversion->conversion_started = false;
        this->current_conversion->conversion_beginning = 0;
        this->current_conversion->conversion_ending = 0;

        this->current_conversion->infos_remaining = 0;
        this->current_conversion->mem_addr_confirmations_remaining = 0;



        this->current_conversion->vima_sent = false;
        this->current_conversion->CPU_requirements_meet = false;
        this->current_conversion->VIMA_requirements_meet = false;

    }
    // *******************************
    // An entry could not be allocated
    // *******************************
    else {
#if VIMA_CONVERSION_DEBUG == 1
        printf("Entry could not be allocated!\n");
#endif
        if (this->current_conversion != 0x0) {
            ++this->not_enough_conversion_entries;
        }
        this->current_conversion = 0x0;

        this->iteration = UINT32_MAX;
        this->state_machine = UINT8_MAX;

        // Write register control
        for (uint32_t i=0; i < 259; ++i) regs_list[i] = false;


        // Conversion data
        for (uint32_t i=0; i < 8; ++i) {
            this->addr[i] = 0;
            this->uop_id[i] = 0;
            this->mem_addr[i] = 0;
        }

    }

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
    fprintf(output, "original_program_instructions: %lu\n", original_program_instructions);
    fprintf(output, "conversion_entry_allocated: %lu\n", this->conversion_entry_allocated);
    fprintf(output, "not_enough_conversion_entries: %lu\n", this->not_enough_conversion_entries);
    fprintf(output, "AGU_result_from_current_conversion: %lu\n", AGU_result_from_current_conversion); 
    fprintf(output, "AGU_result_from_wrong_conversion: %lu\n", AGU_result_from_wrong_conversion);
    fprintf(output, "conversions_blacklisted: %lu\n", this->conversions_blacklisted);
    fprintf(output, "avoided_by_the_blacklist: %lu\n", this->avoided_by_the_blacklist);
    fprintf(output, "time_waiting_for_infos: %lu\n", time_waiting_for_infos);
}
