#include "../simulator.hpp"

prefetcher_t::prefetcher_t(){
    this->prefetcher = NULL;
    //ctor
}

prefetcher_t::~prefetcher_t()
{
    if(this->prefetcher!=NULL) delete this->prefetcher;
    //dtor
}
void prefetcher_t::allocate(uint32_t NUMBER_OF_PROCESSORS){
    // libconfig::Setting* cfg_root = orcs_engine.configuration->getConfig();
    // set_PARALLEL_PREFETCH (cfg_root[0]["NUMBER_OF_PROCESSORS"]);
    this->set_totalPrefetched(0);
    this->set_latePrefetches(0);
    this->set_usefulPrefetches(0);
    this->set_latePrefetches(0);
    this->set_totalCycleLate(0);
    //#if STRIDE
        this->prefetcher = new stride_prefetcher_t();
        this->prefetcher->allocate(NUMBER_OF_PROCESSORS);
    //#endif  
    // List of cycle completation prefetchs. Allows control issue prefetchers
    this->prefetched_lines.reserve(NUMBER_OF_PROCESSORS);
}
// ================================================================
// @mobLine - references to index the prefetch (algo como último acesso)
// @*cache - cache to be instaled line prefetched (LLC provavelmente)
// ================================================================
void prefetcher_t::prefetch(memory_order_buffer_line_t *mob_line, cache_t *cache){
    
    // Dados da linha de cache onde o dado de prefetch será inserido.
    uint64_t idx_padding, line_padding;

    // ******************************
    // Remove um prefetch já completo
    // ******************************

    // ******************************
    // Remove um prefetch já completo
    // ******************************

    if ((this->prefetched_lines.size() != 0) && 
        (this->prefetched_lines.front()->readyAt <= orcs_engine.get_global_cycle()) &&
        (this->prefetched_lines.front()->status == PACKAGE_STATE_WAIT))
    {
        /* Insere na cache recebida */
        line_t *linha = cache->installLine(this->prefetched_lines.at(0), cache->latency, idx_padding, line_padding);
        
        /* Marca como prefetched */
        linha->prefetched=1;
        
        /* Remove esse mais antigo que já está completo */
        delete this->prefetched_lines.at(0);
        this->prefetched_lines.erase(this->prefetched_lines.begin());

    }



    // ****************************************
    // Com base no acesso à memória atual,
    // calcula o próximo endereço de prefetch
    // ****************************************
    int64_t newAddress = this->prefetcher->verify(mob_line->opcode_address,mob_line->memory_address);

    // *****************************************************************
    // Impede que sejam feitas mais requisições de prefetch que o limite
    // *****************************************************************
    if (this->prefetched_lines.size() >= PARALLEL_PREFETCH) {
      return;
    }


    // ******************
    // Realiza o prefetch
    // ******************

    if(newAddress != POSITION_FAIL) { // Se houver uma previsão de endereço

        uint32_t ttc = 0; // Latência de acesso à cache
        uint32_t status = cache->read(newAddress, ttc); // Verifica se já está presente na cache
        if(status == MISS){ // Caso não esteja

            // ********************
            // Processo de prefetch
            // ********************
            this->add_totalPrefetched(); // Counter
           
            // Cria uma nova entrada para controle do prefetch
            memory_package_t *pk = new memory_package_t();
            this->prefetched_lines.push_back(pk);

            // Preeche com dados do prefetch
            pk->opcode_address = 0x0;
            pk->memory_address = newAddress;
            pk->memory_size = mob_line->memory_size;
            pk->memory_operation = mob_line->memory_operation;
            pk->status = PACKAGE_STATE_UNTREATED;
            pk->is_hive = false;
            pk->is_vima = false;
            pk->hive_read1 = mob_line->hive_read1;
            pk->hive_read2 = mob_line->hive_read2;
            pk->hive_write = mob_line->hive_write;
            pk->readyAt = orcs_engine.get_global_cycle();
            pk->born_cycle = orcs_engine.get_global_cycle();
            pk->sent_to_ram = false;
            pk->type = DATA;
            pk->uop_number = 0;
            pk->processor_id = 0;
            pk->op_count[pk->memory_operation]++;
            pk->clients.shrink_to_fit();

            // Faz a requisição para a DRAM
            orcs_engine.memory_controller->add_requests_prefetcher();
            orcs_engine.memory_controller->requestDRAM(pk);

        }
    }
}
void prefetcher_t::statistics(FILE *output){
   
	if (output != NULL){
            utils_t::largeSeparator(output);
            fprintf(output,"##############  PREFETCHER ##################\n");
            fprintf(output,"Total Prefetches: %u\n", this->get_totalPrefetched());
            fprintf(output,"Useful Prefetches: %u\n", this->get_usefulPrefetches());
            fprintf(output,"Late Prefetches: %u\n",this->get_latePrefetches());
            fprintf(output,"MediaAtraso: %.4f\n",(float)this->get_totalCycleLate()/(float)this->get_latePrefetches());
            utils_t::largeSeparator(output);
        }

}

void prefetcher_t::reset_statistics(){
    this->set_totalPrefetched(0);
    this->set_usefulPrefetches(0);
    this->set_latePrefetches(0);
    this->set_totalCycleLate(0);
}
