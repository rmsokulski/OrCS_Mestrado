#include "../simulator.hpp"

prefetcher_t::prefetcher_t(){
    this->prefetcher = NULL;
    //ctor
}

prefetcher_t::~prefetcher_t()
{
    if(this->prefetcher!=NULL) delete &this->prefetcher;
    //dtor
}
void prefetcher_t::allocate(uint32_t NUMBER_OF_PROCESSORS){
    // libconfig::Setting* cfg_root = orcs_engine.configuration->getConfig();
    // set_PARALLEL_PREFETCH (cfg_root[0]["NUMBER_OF_PROCESSORS"]);

    this->set_latePrefetches(0);
    this->set_usefulPrefetches(0);
    this->set_latePrefetches(0);
    this->set_totalCycleLate(0);
    //#if STRIDE
        this->prefetcher = new stride_prefetcher_t();
        this->prefetcher->allocate(NUMBER_OF_PROCESSORS);
    //#endif  
    // List of cycle completation prefetchs. Allows control issue prefetchers
    this->prefetch_waiting_complete.reserve(NUMBER_OF_PROCESSORS);
}
// ================================================================
// @mobLine - references to index the prefetch
// @*cache - cache to be instaled line prefetched
// ================================================================
void prefetcher_t::prefecht(memory_order_buffer_line_t *mob_line, cache_t *cache){
    
    // Dados da linha de cache onde o dado de prefetch será inserido.
    uint32_t idx_padding, line_padding;

    // Ciclo atual
    uint64_t cycle = orcs_engine.get_global_cycle();

    // ******************************
    // Remove um prefetch já completo
    // ******************************

    if(
       (this->prefetch_waiting_complete.size()!=0) /* Existe ao menos um prefetch */ &&
       (this->prefetch_waiting_complete.front() <= cycle) /* Já completou o prefetch mais antigo */
      ){
        /* Remove esse mais antigo que já está completo */
        this->prefetch_waiting_complete.erase(this->prefetch_waiting_complete.begin());
    }

    // ****************************************
    // Com base no acesso à memória atual,
    // calcula o próximo endereço de prefetch
    // ****************************************
    int64_t newAddress = this->prefetcher->verify(mob_line->opcode_address,mob_line->memory_address);

    // *****************************************************************
    // Impede que sejam feitas mais requisições de prefetch que o limite
    // *****************************************************************
    if(this->prefetch_waiting_complete.size()>= PARALLEL_PREFETCH){
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

            // ##################################################
            // Problema com o código:
            //  As requisições à DRAM costumavam apresentar uma 
            //  latência fixa.
            //  Hoje, essa latência varia e só sabemos qual é ela
            //  após o pacote ser finalizado.

            //  Alteração necessária: 
            //    Aguardar pacote da DRAM para contabilizar tempo
            //    do prefetch.
            //    Aguardar pacote da DRAM para inserir na cache.


            // ##################################################
            


            // Faz requisição à DRAM
            memory_package_t *prefetch_package = ...;
            orcs_engine.memory_controller->requestDRAM(NULL, newAddress);

            // Informa ao memory controller sobre o pedido de prefetch
            orcs_engine.memory_controller->add_requests_prefetcher();

            // Adiciona a nova linha na cache
            line_t *linha = cache->installLine(newAddress, latency_prefetch, NULL, idx_padding, line_padding);
            
            // Marca como obtida pelo prefetch
            linha->prefetched=1;

            // Armazena quando o prefetch ficou completo, para poder lançar os próximos.
            this->prefetch_waiting_complete.push_back(cycle+latency_prefetch);
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
