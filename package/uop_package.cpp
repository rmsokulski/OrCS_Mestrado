#include "../simulator.hpp"
#include <string>
uop_package_t::uop_package_t(){}

uop_package_t::~uop_package_t(){}
void uop_package_t::package_clean()
{
    /// TRACE Variables
    strcpy(this->opcode_assembly, "N/A");
    this->opcode_operation = INSTRUCTION_OPERATION_NOP;
    this->opcode_address = 0;
    this->opcode_size = 0;

    this->is_hive = false;
    this->hive_read1 = -1;
    this->hive_read2 = -1;
    this->hive_write = -1;

    this->is_vima = false;
    

    memset(this->read_regs, POSITION_FAIL, sizeof(int32_t) * MAX_REGISTERS);
    memset(this->write_regs, POSITION_FAIL, sizeof(int32_t) * MAX_REGISTERS);

    this->uop_operation = INSTRUCTION_OPERATION_NOP;
    memset(this->memory_address, 0, sizeof(uint64_t) * MAX_MEM_OPERATIONS);
    memset(this->memory_size, 0, sizeof(uint32_t) * MAX_MEM_OPERATIONS);
    this->num_mem_operations = 0;

    //controle
    this->opcode_number = 0;
    this->uop_number = 0;
    this->born_cycle = 0;
    this->readyAt = orcs_engine.get_global_cycle();
    this->status =PACKAGE_STATE_FREE;
}

bool uop_package_t::operator==(const uop_package_t &package) {
    /// TRACE Variables
    if (strcmp(this->opcode_assembly, package.opcode_assembly) != 0) return FAIL;
    if (this->opcode_operation != package.opcode_operation) return FAIL;
    if (this->opcode_address != package.opcode_address) return FAIL;
    if (this->opcode_size != package.opcode_size) return FAIL;

    if ( memcmp(this->read_regs, package.read_regs, sizeof(int32_t)*MAX_REGISTERS) != 0) return FAIL;
    if ( memcmp(this->write_regs, package.write_regs, sizeof(int32_t)*MAX_REGISTERS) != 0) return FAIL;

    if (this->uop_operation != package.uop_operation) return FAIL;
    if ( memcmp(this->memory_address, package.memory_address, sizeof(uint64_t)*MAX_MEM_OPERATIONS) != 0) return FAIL;
    if ( memcmp(this->memory_size, package.memory_size, sizeof(uint32_t)*MAX_MEM_OPERATIONS) != 0) return FAIL;
    if (this->num_mem_operations != package.num_mem_operations) return FAIL;

    
    if (this->opcode_number != package.opcode_number) return FAIL;
    if (this->uop_number != package.uop_number) return FAIL;
    if (this->readyAt != package.readyAt) return FAIL;
    if (this->status != package.status) return FAIL;
   

    return OK;
}
void uop_package_t::opcode_to_uop(
        uint64_t uop_number, 
        instruction_operation_t uop_operation,
        uint32_t latency, uint32_t throughput, functional_unit_t *fu_id,
        opcode_package_t opcode, uint8_t uop_id, bool is_masked)
{
    // ERROR_ASSERT_PRINTF(this->state == PACKAGE_STATE_FREE,
    //                     "Trying to decode to uop in a non-free location\n");

    this->uop_number = uop_number;
    /// TRACE Variables
    strncpy(this->opcode_assembly, opcode.opcode_assembly, sizeof(this->opcode_assembly));
    this->opcode_operation = opcode.opcode_operation;
    this->opcode_address = opcode.opcode_address;
    this->opcode_size = opcode.opcode_size;
    this->opcode_number = opcode.opcode_number;

    this->latency = latency;
    this->throughput = throughput;
    this->functional_unit = fu_id;
    this->uop_id = uop_id;
    this->is_masked = is_masked;

    memcpy(this->read_regs, opcode.read_regs, sizeof(int32_t) * MAX_REGISTERS);
    memcpy(this->write_regs, opcode.write_regs, sizeof(int32_t) * MAX_REGISTERS);

    this->uop_operation = uop_operation;
    //this->memory_address = memory_address;
    //this->memory_size = memory_size;
    this->num_mem_operations = 0;
                    
}

void uop_package_t::updatePackageUntrated(uint32_t stallTime){
    this->status = PACKAGE_STATE_UNTREATED;
    this->readyAt = orcs_engine.get_global_cycle()+stallTime;
}
void uop_package_t::updatePackageReady(uint32_t stallTime){
    this->status = PACKAGE_STATE_READY;
    this->readyAt = orcs_engine.get_global_cycle()+stallTime;
}
void uop_package_t::updatePackageWait(uint32_t stallTime){
    this->status = PACKAGE_STATE_WAIT;
    this->readyAt = orcs_engine.get_global_cycle()+stallTime;
}
void uop_package_t::updatePackageFree(uint32_t stallTime){
    this->status = PACKAGE_STATE_FREE;
    this->readyAt = orcs_engine.get_global_cycle()+stallTime;
}
/// Convert Instruction variables into String
std::string uop_package_t::content_to_string() {
    std::string content_string;
    content_string = "";

    content_string = content_string + "Uop Number " + utils_t::uint64_to_string(this->uop_number);
    
    content_string = content_string + " " + utils_t::uint64_to_string(this->opcode_address);
    content_string = content_string + " " + get_enum_instruction_operation_char(this->uop_operation);

    for (uint32_t i = 0; i < num_mem_operations; ++i){
        content_string = content_string + " Address[" + utils_t::uint32_to_string(i); + "]: $" + utils_t::big_uint64_to_string(this->memory_address[i]);
        content_string = content_string + " Size[" + utils_t::uint32_to_string(i); + "]:" + utils_t::uint32_to_string(this->memory_size[i]);
    }

    content_string = content_string + " | RRegs[";
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->read_regs[i] >= 0) {
            content_string = content_string + " " + utils_t::uint32_to_string(this->read_regs[i]);
        }
    }

    content_string = content_string + " ] | WRegs[";
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->write_regs[i] >= 0) {
            content_string = content_string + " " + utils_t::uint32_to_string(this->write_regs[i]);
        }
    }
    content_string = content_string + " ]";

    return content_string;
}
/// Convert Instruction variables into String
std::string uop_package_t::content_to_string2() {
    std::string content_string;
    content_string = "";

    content_string = content_string + "Uop Number " + utils_t::uint64_to_string(this->uop_number);
    content_string = content_string + " Opcode Number " + utils_t::uint64_to_string(this->opcode_number);


    content_string = content_string + " " + utils_t::uint64_to_string(this->opcode_address);
    content_string = content_string + " " + get_enum_instruction_operation_char(this->uop_operation);

    content_string = content_string + " Status Opcode "+ get_enum_package_state_char(this->status);
    content_string = content_string + " Ready At" + utils_t::uint64_to_string(this->readyAt);

    return content_string;
}
