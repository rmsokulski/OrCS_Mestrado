class uop_package_t{
    
    public:
    uop_package_t();
    ~uop_package_t();
    
    /// TRACE Variables
    char opcode_assembly[MAX_ASSEMBLY_SIZE];
    instruction_operation_t opcode_operation;
    uint64_t opcode_address;
    uint32_t opcode_size;

    int32_t read_regs[MAX_REGISTERS];
    int32_t write_regs[MAX_REGISTERS];

    instruction_operation_t uop_operation;
    uint64_t memory_address[MAX_MEM_OPERATIONS];
    uint32_t memory_size[MAX_MEM_OPERATIONS];
    uint32_t num_mem_operations;

    uint64_t read_address;
    uint64_t read2_address;
    uint64_t write_address;

    bool is_hive;
    int32_t hive_read1;
    int32_t hive_read2;
    int32_t hive_write;

    bool is_vima;

    uint32_t latency;
    uint32_t throughput;
    functional_unit_t *functional_unit;

    uint8_t uop_id;
    bool is_masked;

    bool is_vector_insn;

    void opcode_to_uop(uint64_t uop_number, 
            instruction_operation_t uop_operation, 
            uint32_t latency, uint32_t throughput, functional_unit_t *fu_id,
            opcode_package_t opcode, uint8_t uop_id, bool is_masked);

    inline void add_memory_operation(uint64_t memory_address, uint32_t memory_size);


    bool operator==(const uop_package_t &package);
    void package_clean();
    void updatePackageUntrated(uint32_t stallTime);
    void updatePackageReady(uint32_t stallTime);
    void updatePackageWait(uint32_t stallTime);
    void updatePackageFree(uint32_t stallTime);
    std::string content_to_string();
    std::string content_to_string2();
    // Control variables
    uint64_t opcode_number;
    uint64_t uop_number;
    uint64_t born_cycle;
    uint64_t readyAt;
    package_state_t status;
};

inline void uop_package_t::add_memory_operation(uint64_t memory_address, uint32_t memory_size) {
    //printf("%s\n", this->opcode_assembly);
    //printf("add_memory_operation: num_mem_operations -> %u\n", num_mem_operations);
    assert(num_mem_operations < MAX_MEM_OPERATIONS);
    this->memory_address[num_mem_operations] = memory_address;
    this->memory_size[num_mem_operations] = memory_size;
    ++num_mem_operations;

    return;
}
