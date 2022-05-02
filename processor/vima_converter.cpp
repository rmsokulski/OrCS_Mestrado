#include "./../simulator.hpp"



// TODO -> Definir mais especificamente
instruction_operation_t vima_converter_t::define_vima_operation() {
    // Dados retirados de:
    // instrinsics_extension.cpp:408 (verificar em vima_defines.h a qual lista pertence)
    switch(this->is_mov) {
        case true:
            return INSTRUCTION_OPERATION_VIMA_INT_ALU; //_vim64_icpyu
            break;
        case false:
            // Só definido pra soma ainda
            return INSTRUCTION_OPERATION_VIMA_INT_ALU; // _vim64_iadds
            break;
    }
    return INSTRUCTION_OPERATION_VIMA_FP_ALU;
}

void vima_converter_t::generate_VIMA_instruction(uint32_t instruction_converted_size) {
        
        // ******************************************
        // Cria uma instrução para enviar para a VIMA
        // ******************************************

        opcode_package_t base_opcode;
        uop_package_t base_uop;
        base_opcode.package_clean();
        base_uop.package_clean();



		base_uop.opcode_to_uop(0, this->define_vima_operation(),
								this->mem_operation_latency, this->mem_operation_wait_next, this->mem_operation_fu,
								base_opcode, 0, false);
		base_uop.add_memory_operation(0, 1);

		base_uop.unique_conversion_id = this->unique_conversion_id;
		base_uop.is_hive = false;
		base_uop.hive_read1 = -1;
		base_uop.hive_read2 = -1;
		base_uop.hive_write = -1;

		base_uop.is_vima = true;
        /* Começa a partir da que usou para identificar os addrs iniciais, por isso sem o + stride */
		base_uop.read_address = this->mem_addr[0] + (instruction_converted_size * this->conversion_beginning);
		base_uop.read2_address = (this->is_mov) ? 0x0 : this->mem_addr[1] + (instruction_converted_size * this->conversion_beginning);
		base_uop.write_address = this->mem_addr[3] + (instruction_converted_size * this->conversion_beginning);

		base_uop.updatePackageWait(0);
		base_uop.born_cycle = orcs_engine.get_global_cycle();

        assert (!this->vima_instructions_buffer.is_full());
        this->vima_instructions_buffer.push_front(base_uop);
        this->vima_instructions_launched++;

#if VIMA_CONVERSION_DEBUG
        printf("Instrução VIMA inserida!\n");
#endif
}


// Conversion address
void vima_converter_t::AGU_result(uop_package_t *uop) {
    
    if (this->addr[uop->linked_to_converter] == uop->opcode_address && this->uop_id[uop->linked_to_converter] == uop->uop_id && this->unique_conversion_id == uop->unique_conversion_id) {
        if (uop->linked_to_iteration < 0) {
            assert (this->infos_remaining);
            this->mem_addr[uop->linked_to_converter] = uop->memory_address[0];
            --this->infos_remaining;
            ++this->AGU_result_from_current_conversion;
#if VIMA_CONVERSION_DEBUG == 1
            printf("//*********************************\n");
            printf(" %lu - Received address from iteration %ld [%u Remaining]\n", uop->opcode_address, uop->linked_to_iteration, this->infos_remaining);
            printf("//*********************************\n");
#endif
        }


        if ((this->infos_remaining == 0) && (!this->vima_sent)) {
#if VIMA_CONVERSION_DEBUG
            printf("Required address data acquired\n");
#endif


            // ****************
            // Calculate stride
            // ****************
            int64_t stride;
            for (int8_t i=0; i<4; ++i) {
                if (i == 1) continue; // Operation
                if (this->is_mov && i == 2) continue;// Ld 2

                // Loads/store
                stride = this->mem_addr[7-i] - this->mem_addr[3-i];
                if (stride != uop->memory_size[0]) {
#if VIMA_CONVERSION_DEBUG
                    printf("Stride %ld ([%d] %lu - [%d] %lu) != %u ==>> Invalidating conversion\n", stride, 7-i, this->mem_addr[7-i], 3-i, this->mem_addr[3-i], uop->memory_size[0]);
#endif
                    this->invalidate_conversion();
                    return;
                }
            }

            // *****************************
            // Required offset for alignment
            // *****************************
            this->get_index_for_alignment(uop->memory_size[0]);
#if VIMA_CONVERSION_DEBUG
            printf("//****************************\n");
            printf("Generating VIMA instruction...\n");
            printf("//****************************\n");
#endif
            this->generate_VIMA_instruction(uop->memory_size[0]);
            this->vima_sent = true;

            if (uop->memory_size[0] == AVX_256_SIZE) {
                this->mem_addr_confirmations_remaining = (this->is_mov) ? 2 * this->necessary_AVX_256_iterations_to_one_vima : 3 * this->necessary_AVX_256_iterations_to_one_vima;
                ++AVX_256_to_VIMA_conversions;
            } else {
                this->mem_addr_confirmations_remaining = (this->is_mov) ? 2 * this->necessary_AVX_512_iterations_to_one_vima : 3 * this->necessary_AVX_512_iterations_to_one_vima;
                ++AVX_512_to_VIMA_conversions;
            }

            this->time_waiting_for_infos_stop = orcs_engine.get_global_cycle();
            this->time_waiting_for_infos += (this->time_waiting_for_infos_stop - this->time_waiting_for_infos_start);
            this->conversion_started = true;
        }
        /* Ignored instruction trying to confirm its stride */
        else if (this->vima_sent) {
            // **********************************
            // Check for the memory access stride
            // **********************************
            uint64_t expected = this->mem_addr[uop->linked_to_converter] + uop->memory_size[0] * uop->linked_to_iteration;
            if (expected == uop->memory_address[0]) {
                this->mem_addr_confirmations_remaining--;
		 ++this->AGU_result_from_current_conversion;
#if VIMA_CONVERSION_DEBUG
                printf("//**************************\n");
                printf("[%s]Confirmations remaining: %ld\n", (uop->uop_operation == INSTRUCTION_OPERATION_MEM_LOAD) ? "LOAD" : (uop->uop_operation == INSTRUCTION_OPERATION_MEM_STORE) ? "STORE": "???", this->mem_addr_confirmations_remaining);
                printf("//**************************\n");
#endif
                if (this->mem_addr_confirmations_remaining == 0) {
#if VIMA_CONVERSION_DEBUG
                    printf("***********************************************\n");
                    printf("CPU requirements achieved! [conversion ID: %lu]\n", this->unique_conversion_id);
                    printf("***********************************************\n");
#endif
                    this->CPU_requirements_meet = true;
#if VIMA_CONVERSION_DEBUG
                    printf("%lu CPU informing VIMA...[conversion ID: %lu]\n", orcs_engine.get_global_cycle(), this->unique_conversion_id);
#endif                    
		     orcs_engine.vima_controller->confirm_transaction(1 /* Success */, this->unique_conversion_id);
                }
            } else {
                invalidate_conversion();
            }
        }
    }
    /* Is a result from an older conversion, already invalidated */
     else {
        ++this->AGU_result_from_wrong_conversion;
#if VIMA_CONVERSION_DEBUG
        printf("AGU result from wrong conversion!\n");
#endif
    }
}


// Based in the memory addresses accessed by each load and store and the last iteration inside
// ROB, this procedure defines the first and last vector indexes to be converted into a VIMA instruction
void vima_converter_t::get_index_for_alignment(uint32_t access_size) {
    // Acredito que pode não existir intersecção na maior parte dos casos, então apenas utiliza os próximos índices que entrarão pelo Rename
    this->conversion_beginning = this->iteration + 1;

    if (access_size == AVX_256_SIZE) {
        this->conversion_ending = this->iteration + necessary_AVX_256_iterations_to_one_vima;
    } else {
        this->conversion_ending = this->iteration + necessary_AVX_512_iterations_to_one_vima;
    }
#if VIMA_CONVERSION_DEBUG
    printf("**** Definição de limites para conversão ****\n");
    printf("Iteração de início da conversão: %lu\n", this->conversion_beginning);
    printf("Iteração de fim da conversão:    %lu\n", this->conversion_ending);
    printf("*********************************************\n");
#endif
}


// TODO
// Get data from VIMA and set registers with it before meet the requirements
void vima_converter_t::vima_execution_completed(memory_package_t *vima_package, uint64_t readyAt) {
#if VIMA_CONVERSION_DEBUG
    printf("***************************\n");
    printf("VIMA requirements achieved! (%lu == %lu)\n", this->unique_conversion_id, vima_package->unique_conversion_id);
    printf("***************************\n");
#endif
    if (this->unique_conversion_id == vima_package->unique_conversion_id) {
        this->VIMA_requirements_meet = true;
	    this->VIMA_requirements_meet_readyAt = readyAt;
    }
}

void vima_converter_t::invalidate_conversion() {
#if VIMA_CONVERSION_DEBUG
    printf("Conversão invalidada!\n");
#endif

    // **********
    // Statistics
    // **********
    ++this->conversion_failed;


    // *****************************************
    // Libera instruções em espera para execução
    // *****************************************
    orcs_engine.processor->conversion_invalidation();


    // *************************************
    // Avisa VIMA para descartar o resultado
    // *************************************
    orcs_engine.vima_controller->confirm_transaction(2 /* Failure */, this->unique_conversion_id);


    // ******************************
    // Libera para futuras conversões
    // ******************************
    this->start_new_conversion();

}
