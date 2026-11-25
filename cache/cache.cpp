#include "../simulator.hpp"

// Constructor
cache_t::cache_t() {
    this->id = 0;
    this->level = 0;
    this->size = 0;
    this->latency = 0;
    this->associativity = 0;
    this->n_sets = 0;
    this->sets = NULL;
    this->offset = 0;
	this->cache_hit = 0;
    this->cache_miss = 0;
    this->cache_access = 0;
	this->cache_eviction = 0;
    this->cache_read = 0;
    this->cache_write = 0;
    this->cache_writeback = 0;
    this->cache_install = 0;
    this->cache_back_invalidation = 0;
    this->cache_write_merged = 0;
    this->cache_write_merged_missing = 0;
    this->cache_merged_miss = 0;
    this->inclusive = true;
	this->is_inst_cache = false;

	this->mshr_occupied_entries = 0;
	this->concurrent_cache_accesses = 0;
	this->mshr_stall = false;

	this->LINE_SIZE = 0;
    this->PREFETCHER_ACTIVE = 0;
    this->INSTRUCTION_LEVELS = 0;
    this->DATA_LEVELS = 0;
    this->POINTER_LEVELS = 0;
    this->CACHE_MANAGER_DEBUG = 0;
    this->WAIT_CYCLE = 0;
}

cache_t::~cache_t(){
	if (orcs_engine.get_global_cycle() == 0) return;
	delete[] this->cache_hit_per_type;
	delete[] this->cache_miss_per_type;
	delete[] this->cache_count_per_type;
	delete[] this->total_per_type;
	delete[] this->min_per_type;
	delete[] this->max_per_type;
	delete[] sets;
	//ORCS_PRINTF ("cycle: %lu\n", orcs_engine.get_global_cycle())
}

// Allocate each cache type
void cache_t::allocate(uint32_t NUMBER_OF_PROCESSORS, uint32_t INSTRUCTION_LEVELS, uint32_t DATA_LEVELS) {
    // Access configure file
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();

	// Get prefetcher info
	libconfig::Setting &prefetcher_defs = cfg_root["PREFETCHER"];
	set_PREFETCHER_ACTIVE(prefetcher_defs["PREFETCHER_ACTIVE"]);

	// Get general cache info
	libconfig::Setting &cfg_cache_defs = cfg_root["CACHE_MEMORY"]["CONFIG"];
	set_WAIT_CYCLE(cfg_cache_defs["WAIT_CYCLE"]);

	/// LLC inclusion policy (optional): "INCLUSIVE" (default) or "NON_INCLUSIVE"
	std::string inclusion = "INCLUSIVE";
	cfg_cache_defs.lookupValue("INCLUSION", inclusion);
	ERROR_ASSERT_PRINTF(inclusion == "INCLUSIVE" || inclusion == "NON_INCLUSIVE", "INCLUSION must be INCLUSIVE or NON_INCLUSIVE, got %s\n", inclusion.c_str())
	this->inclusive = (inclusion == "INCLUSIVE");

	set_INSTRUCTION_LEVELS (INSTRUCTION_LEVELS);
	set_DATA_LEVELS (DATA_LEVELS);
	POINTER_LEVELS = ((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS);
	//POINTER_LEVELS = 3;

	uint32_t line_number = this->size/this->LINE_SIZE;
	uint32_t total_sets = line_number/associativity;

	this->offset_bits_shift = 0;
    this->index_bits_shift = utils_t::get_power_of_two(this->get_LINE_SIZE());
    this->tag_bits_shift = index_bits_shift + utils_t::get_power_of_two(total_sets);
	
	uint64_t i;
    /// OFFSET MASK
    for (i = 0; i < utils_t::get_power_of_two(this->get_LINE_SIZE()); i++) {
        this->offset_bits_mask |= 1 << i;
    }
    
    /// INDEX MASK
    for (i = 0; i < utils_t::get_power_of_two(total_sets); i++) {
        this->index_bits_mask |= 1 << (i + index_bits_shift);
    }

    /// TAG MASK
    for (i = tag_bits_shift; i < utils_t::get_power_of_two((uint64_t)INT64_MAX+1); i++) {
        this->tag_bits_mask |= 1 << i;
    }
	
	this->sets = new cacheSet_t[this->n_sets]();
    for (size_t i = 0; i < this->n_sets; i++) {
		this->sets[i].lines = new line_t[this->associativity]();
		this->sets[i].n_lines = this->associativity;
        for (uint32_t j = 0; j < this->sets[i].n_lines; j++) {
            this->sets[i].lines[j].allocate(POINTER_LEVELS);
			this->sets[i].lines[j].clean_line();
            for (uint32_t k = 0; k < NUMBER_OF_PROCESSORS; k++) {
                for (uint32_t l = 0; l < POINTER_LEVELS; l++) {
                    this->sets[i].lines[j].line_ptr_caches[k][l] = NULL;
                }
			}
			//this->sets[i].lines[j].print_line();
        }
    }
    this->set_cache_access(0);
    this->set_cache_hit(0);
    this->set_cache_miss(0);
	this->set_cache_eviction(0);
    this->set_cache_read(0);
    this->set_cache_write(0);
    this->set_cache_writeback(0);
	this->set_cache_install(0);
	this->set_concurrent_cache_accesses(0);
	this->set_max_reached(0);
	this->set_mshr_occupied_entries(0);
	this->set_mshr_stall(false);
	this->set_max_MSHR_reached(0);
	this->set_num_mshr_stall(0);

	this->cache_hit_per_type = new uint64_t[MEMORY_OPERATION_LAST]();
	this->cache_miss_per_type = new uint64_t[MEMORY_OPERATION_LAST]();
	this->cache_count_per_type = new uint64_t[MEMORY_OPERATION_LAST]();
	
	this->total_per_type = new int64_t[MEMORY_OPERATION_LAST]();
	this->min_per_type = new int64_t[MEMORY_OPERATION_LAST]();
	this->max_per_type = new int64_t[MEMORY_OPERATION_LAST]();
	for (i = 0; i < MEMORY_OPERATION_LAST; i++){
		this->min_per_type[i] = INT64_MAX;
		this->max_per_type[i] = 0;
	}
}

// Return address index in cache
inline void cache_t::tagIdxSetCalculation(uint64_t address, uint64_t *idx, uint64_t *tag) {
	*tag = addr_tag(address);
	*idx = addr_idx(address);
}

// void cache_t::printTagIdx (uint64_t address){
// 	uint32_t get_bits = (this->n_sets) - 1;
// 	uint64_t tag = (address >> this->offset);
// 	uint32_t idx = tag & get_bits;
// 	// tag >>= utils_t::get_power_of_two(this->n_sets);
// }

// Verifica se um endereço está na cache sem atualizar LRU ou estatísticas
bool cache_t::peek(uint64_t address) {
    uint64_t idx;
    uint64_t tag;

    // 1. Calcula o Índice (Set) e a Tag usando a mesma lógica de hardware da leitura
    this->tagIdxSetCalculation(address, &idx, &tag);

    // 2. Itera sobre todas as linhas (vias) do conjunto
    for (size_t i = 0; i < this->sets[idx].n_lines; i++) {
        // Verifica se a linha é válida E se a tag corresponde
        if (this->sets[idx].lines[i].valid == 1 && this->line_tag(&this->sets[idx].lines[i]) == tag) {
            return true; // Está na cache (HIT)
        }
    }

    return false; // Não está na cache (MISS)
}

// Reads a cache, updates cycles and return HIT or MISS status
uint32_t cache_t::read(uint64_t address, uint32_t &ttc){
    uint64_t idx;
    uint64_t tag;
	this->tagIdxSetCalculation(address, &idx, &tag);
	//ERROR_ASSERT_PRINTF (this->get_count() < this->mshr_size, "REQUEST # > MSHR_SIZE")
	this->add_concurrent_cache_accesses();
	this->add_cache_access();
	if (this->get_concurrent_cache_accesses() > this->get_max_reached()) this->set_max_reached(this->get_concurrent_cache_accesses());
	for (size_t i = 0; i < this->sets->n_lines; i++) {
		//printf("tag: %u\n", this->sets[idx].lines[i].dirty);
		if (this->sets[idx].lines[i].valid && this->line_tag(&this->sets[idx].lines[i]) == tag) {
			// Se ready Cycle for menor que o ciclo atual, a latencia é apenas da leitura, sendo um hit.
			if (this->sets[idx].lines[i].ready_at <= orcs_engine.get_global_cycle()){
				if (PREFETCHER_ACTIVE){
					if (this->sets[idx].lines[i].prefetched == 1){
						orcs_engine.cacheManager->prefetcher->add_usefulPrefetches();
						this->sets[idx].lines[i].prefetched =0;
					}
				}

				this->sets[idx].lines[i].lru = orcs_engine.get_global_cycle();
				ttc += this->latency;
				// if (this->id == DATA) {
				// 	if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
				// 		//ORCS_PRINTF("     Cache level %u Ready At %lu\n", this->level, this->sets[idx].lines[i].ready_at)
				// 	}
				// }
				this->add_cache_hit();
				return HIT;
			}
			// Se ready Cycle for maior que o atual, a latencia é dada pela demora a chegar
			else{
				if (PREFETCHER_ACTIVE){
					if (this->sets[idx].lines[i].prefetched == 1){
						orcs_engine.cacheManager->prefetcher->add_latePrefetches();
						orcs_engine.cacheManager->prefetcher->add_usefulPrefetches();
						uint32_t latePrefetcher = orcs_engine.cacheManager->prefetcher->get_totalCycleLate()+
						(this->sets[idx].lines[i].ready_at - orcs_engine.get_global_cycle());
						orcs_engine.cacheManager->prefetcher->set_totalCycleLate(latePrefetcher);
						this->sets[idx].lines[i].prefetched =0;
					}
				}
				ttc+=(this->sets[idx].lines[i].ready_at - orcs_engine.get_global_cycle());
				this->sets[idx].lines[i].lru = ttc;
				return HIT;
			}
		}
	}
	// Allocate MSHR entry
	if (this->get_mshr_occupied_entries() < this->mshr_size) {

		// ******************************************************************************************
		// Note that, if the request reaches a memory level, it is in the MSHR of the superior ones
		// ******************************************************************************************
		this->add_mshr_occupied_entries();

		if (this->get_mshr_occupied_entries() > this->get_max_MSHR_reached()) this->set_max_MSHR_reached(this->get_mshr_occupied_entries());

	} else {
		// Do not add miss, because will be added only once
		this->set_mshr_stall(true);
		this->add_num_mshr_stall();
		this->concurrent_cache_accesses--;
		this->cache_access--;
		return MSHR_STALL;
	}


	ttc += this->latency;
	this->add_cache_miss();
	return MISS;
}

// Returns the minor LRU line
inline uint32_t cache_t::searchLru(cacheSet_t *set) {
	uint32_t index = 0;
	for (uint32_t i = 1; i < set->n_lines; i++)	{
		index = (set->lines[index].lru <= set->lines[i].lru)? index : i;
	}

	return index;
}

// Remove every pointer to and from a line (any core, any level).
void cache_t::unlink_line(line_t *line, uint32_t line_level) {
	for (uint32_t p = 0; p < line->get_NUMBER_OF_PROCESSORS(); p++) {
		for (uint32_t k = 0; k < POINTER_LEVELS; k++) {
			line_t *other = line->line_ptr_caches[p][k];
			if (other == NULL) continue;
			if (other->line_ptr_caches[p][line_level] == line) {
				other->line_ptr_caches[p][line_level] = NULL;
			}
			line->line_ptr_caches[p][k] = NULL;
		}
	}
}

// Send a dirty line to DRAM.
void cache_t::send_writeback(uint64_t memory_address, uint32_t processor_id) {
	memory_package_t* request = new memory_package_t();
	request->processor_id = processor_id;
	request->memory_address = memory_address;
	request->memory_operation = MEMORY_OPERATION_WRITE;
	request->memory_size = this->LINE_SIZE;
	request->is_hive = false;
	request->is_vima = false;
	request->status = PACKAGE_STATE_UNTREATED;
	request->readyAt = orcs_engine.get_global_cycle();
	request->born_cycle = orcs_engine.get_global_cycle();
	request->sent_to_ram = false;
	request->type = DATA;
	request->op_count[request->memory_operation]++;
	orcs_engine.memory_controller->requestDRAM(request);
}

// Evict a valid line, clean or dirty, before its slot is reused.
//  1. Upper levels (every core): an inclusive cache removes their copies,
//     taking their dirty data. Every private level is inclusive; the LLC
//     follows INCLUSION. A non-inclusive LLC leaves them in place.
//  2. Dirty data goes to the next level's copy, or to DRAM from the LLC
//     (or when the next level no longer holds the line).
//  3. Every pointer to and from the victim is cleared.
void cache_t::evict(line_t *victim, uint32_t processor_id) {
	const uint32_t L = this->level;
	const bool is_llc = (L == DATA_LEVELS - 1);
	const bool remove_upper = !is_llc || this->inclusive;
	const uint64_t victim_address = victim->address_line;

	if (remove_upper) {
		for (uint32_t p = 0; p < victim->get_NUMBER_OF_PROCESSORS(); p++) {
			/// Upper copies of this core, nearest level first. A level the
			/// victim does not point to is found through the level below it.
			line_t *upper[MAX_NUM_CACHE_LEVELS] = {NULL};
			for (int32_t u = (int32_t) L - 1; u >= 0; u--) {
				line_t *copy = victim->line_ptr_caches[p][u];
				for (uint32_t k = u + 1; copy == NULL && k < L; k++) {
					if (upper[k] != NULL) copy = upper[k]->line_ptr_caches[p][u];
				}
				upper[u] = copy;
			}
			for (uint32_t u = 0; u < L; u++) {
				line_t *copy = upper[u];
				if (copy == NULL || !copy->valid || copy->address_line != victim->address_line) continue;
				if (copy->dirty) victim->dirty = 1;   /// newer data than the victim
				this->unlink_line(copy, u);
				copy->clean_line();
				this->add_cache_back_invalidation();
			}
		}
	}

	if (victim->dirty) {
		this->add_cache_writeback();
		bool written = false;
		if (!is_llc) {
			for (uint32_t p = 0; p < victim->get_NUMBER_OF_PROCESSORS() && !written; p++) {
				line_t *next = victim->line_ptr_caches[p][L + 1];
				if (next != NULL && next->valid && next->address_line == victim->address_line) {
					next->dirty = 1;
					written = true;
				}
			}
		}
		if (!written) {
			this->send_writeback(victim_address, processor_id);
		}
	}

	this->unlink_line(victim, L);
	victim->clean_line();
}

// Searches for a cache line to write data
line_t* cache_t::installLine(memory_package_t* request, uint32_t latency, uint64_t &idx, uint64_t &line) {
	// printf("installLine %lu in processor %u\n", request->memory_address, request->processor_id);
	line = POSITION_FAIL;
    uint64_t tag;
    
	// -----------------------------------------------------------------------------------------
    // Obtém o índice onde será instalada e a tag, com base no endereço da requisição
    // -----------------------------------------------------------------------------------------
    this->tagIdxSetCalculation(request->memory_address, &idx, &tag);

	// -----------------------------------------------------------------------------------------
    // Procura uma entra vazia no conjunto
    // -----------------------------------------------------------------------------------------
	for (size_t i = 0; i < this->sets[idx].n_lines; i++) {
		if (this->sets[idx].lines[i].valid == 0) {
			line = i;
            break;
		}
	}

	// -----------------------------------------------------------------------------------------
    // Se não encontrar, obtém uma pelo LRU
    // -----------------------------------------------------------------------------------------
	if ((int)line == POSITION_FAIL) {
		line = this->searchLru(&this->sets[idx]);
		this->add_cache_eviction();
		this->evict(&this->sets[idx].lines[line], request->processor_id);
	}

	// -----------------------------------------------------------------------------------------
    // Insere o novo dado
    // -----------------------------------------------------------------------------------------
	/// The line holds this address; its tag/index at any level come from it
	this->sets[idx].lines[line].address_line = request->memory_address & ~((uint64_t)this->LINE_SIZE - 1);
	this->sets[idx].lines[line].lru = orcs_engine.get_global_cycle() + latency;
	this->sets[idx].lines[line].valid = 1;
	this->sets[idx].lines[line].dirty = 0;
	this->sets[idx].lines[line].prefetched = 0;
	this->sets[idx].lines[line].ready_at = orcs_engine.get_global_cycle() + latency;
	this->add_cache_install();
	return &this->sets[idx].lines[line];
}

// Selects a cache line to install an address and points this memory address with the other cache pointers
void cache_t::returnLine(memory_package_t* request, cache_t *cache) {
	//printf("returnLine %lu in processor %u\n", request->memory_address, request->processor_id);
	uint64_t tag, idx, idx_padding, line_padding;
    this->tagIdxSetCalculation(request->memory_address, &idx, &tag);
	int32_t line = POSITION_FAIL;
	// Selects a line in this cache
	for (size_t i = 0; i < this->sets->n_lines; i++) {
		if (this->sets[idx].lines[i].valid && this->line_tag(&this->sets[idx].lines[i]) == tag) {
			this->sets[idx].lines[i].lru = orcs_engine.get_global_cycle();
			line = i;
			break;
		}
	}
    ERROR_ASSERT_PRINTF(line != POSITION_FAIL, "Error, line não encontrada para retorno")
	if (this->level > 0) {
		line_t *line_return = NULL;

		line_return = cache->installLine(request, this->latency, idx_padding, line_padding);

		this->sets[idx].lines[line].line_ptr_caches[request->processor_id][cache->level] = line_return;
		for (uint32_t i = this->level + 1; i < POINTER_LEVELS; i++) {
			line_return->line_ptr_caches[request->processor_id][i] = this->sets[idx].lines[line].line_ptr_caches[request->processor_id][i];
			/// Lower levels also point to the new copy, so an eviction finds it
			if (line_return->line_ptr_caches[request->processor_id][i] != NULL) {
				line_return->line_ptr_caches[request->processor_id][i]->line_ptr_caches[request->processor_id][cache->level] = line_return;
			}
		}
		line_return->line_ptr_caches[request->processor_id][this->level] = &this->sets[idx].lines[line];
		// Copy information
		/// The lower level keeps the dirty data: the new copy starts clean, and
		/// becomes dirty only if this core writes it (cache_t::write)
		line_return->dirty = 0;
		line_return->lru = line_return->line_ptr_caches[request->processor_id][this->level]->lru;
		line_return->prefetched = line_return->line_ptr_caches[request->processor_id][this->level]->prefetched;
		line_return->ready_at = orcs_engine.get_global_cycle();
	}
}


// Mark a line dirty without counting an access (merged writes).
// Returns false if the line is not in this cache.
bool cache_t::mark_dirty(uint64_t address) {
	uint64_t idx, tag;
	this->tagIdxSetCalculation(address, &idx, &tag);
	for (size_t i = 0; i < this->sets[idx].n_lines; i++) {
		line_t *l = &this->sets[idx].lines[i];
		if (l->valid && this->line_tag(l) == tag) {
			l->dirty = 1;
			l->lru = (l->ready_at <= orcs_engine.get_global_cycle()) ? orcs_engine.get_global_cycle() : l->ready_at + this->latency;
			return true;
		}
	}
	return false;
}

// write address
uint32_t cache_t::write(memory_package_t* request){
	// printf("write %lu in processor %u\n", request->memory_address, request->processor_id);
    uint64_t idx;
    uint64_t tag;
    this->tagIdxSetCalculation(request->memory_address, &idx, &tag);
	int32_t line = POSITION_FAIL;
	this->add_cache_write();
    for (size_t i = 0; i < this->sets->n_lines; i++) {
		if (this->sets[idx].lines[i].valid && this->line_tag(&this->sets[idx].lines[i]) == tag) {
			line = i;
			break;
		}
	}
    if (line == POSITION_FAIL) {
		printf("Nunca deveria entrar aqui!!! (além disso essa parte do código está errada [Javan])\n");
		assert(false);
        line = this->searchLru(&this->sets[idx]);
        this->add_cache_eviction();
        this->evict(&this->sets[idx].lines[line], request->processor_id);
    }
	ERROR_ASSERT_PRINTF(line != POSITION_FAIL, "Error, line didn't find to be written.")
    this->sets[idx].lines[line].dirty = 1;
	if(this->sets[idx].lines[line].ready_at <= orcs_engine.get_global_cycle()){
		this->sets[idx].lines[line].lru = orcs_engine.get_global_cycle();
	} else {
		this->sets[idx].lines[line].lru = this->sets[idx].lines[line].ready_at + this->latency;
	}
	return OK;
}

// statistics of a level of cache
void cache_t::statistics(FILE *output) {
	
	if (output != NULL){
		fprintf(output,"#========================================================================#\n");
        fprintf(output,"#L%d %s Cache\n", this->level+1, get_enum_cache_type_char ((cacheId_t) this->id));
        fprintf(output,"#========================================================================#\n");
		fprintf(output, "%d_Cache_Access:       %lu\n", this->level, this->get_cache_access());
		fprintf(output, "%d_Cache_Hits:         %lu\n", this->level, this->get_cache_hit());
		fprintf(output, "%d_Cache_Miss:         %lu\n", this->level, this->get_cache_miss());
		fprintf(output, "%d_Cache_Install:      %lu\n", this->level, this->get_cache_install());
		fprintf(output, "%d_Cache_Eviction:     %lu\n", this->level, this->get_cache_eviction());
		fprintf(output, "%d_Cache_Back_Invalidation: %lu\n", this->level, this->get_cache_back_invalidation());
		if (this->get_cache_write_merged() != 0) fprintf(output, "%d_Cache_Write_Merged: %lu\n", this->level, this->get_cache_write_merged());
		if (this->get_cache_merged_miss() != 0) fprintf(output, "%d_Cache_Merged_Miss:  %lu\n", this->level, this->get_cache_merged_miss());
		if (this->get_cache_write_merged_missing() != 0) fprintf(output, "%d_Cache_Write_Merged_Missing: %lu\n", this->level, this->get_cache_write_merged_missing());
		fprintf(output, "%d_Cache_Read:         %lu\n", this->level, this->cache_count_per_type[MEMORY_OPERATION_READ]);
		if (this->cache_hit_per_type[MEMORY_OPERATION_READ] != 0) fprintf(output, "%d_Cache_Read_Hit:     %lu\n", this->level, this->cache_hit_per_type[MEMORY_OPERATION_READ]);
		if (this->cache_miss_per_type[MEMORY_OPERATION_READ] != 0) fprintf(output, "%d_Cache_Read_Miss:    %lu\n", this->level, this->cache_miss_per_type[MEMORY_OPERATION_READ]);
		fprintf(output, "%d_Cache_Write:        %lu\n", this->level, this->cache_count_per_type[MEMORY_OPERATION_WRITE]);
		if (this->cache_hit_per_type[MEMORY_OPERATION_WRITE] != 0) fprintf(output, "%d_Cache_Write_Hit:    %lu\n", this->level, this->cache_hit_per_type[MEMORY_OPERATION_WRITE]);
		if (this->cache_miss_per_type[MEMORY_OPERATION_WRITE] != 0)	fprintf(output, "%d_Cache_Write_Miss:   %lu\n", this->level, this->cache_miss_per_type[MEMORY_OPERATION_WRITE]);
		fprintf(output, "%d_Cache_Inst:         %lu\n", this->level, this->cache_count_per_type[MEMORY_OPERATION_INST]);
		if (this->cache_hit_per_type[MEMORY_OPERATION_INST] != 0) fprintf(output, "%d_Cache_Inst_Hit:     %lu\n", this->level, this->cache_hit_per_type[MEMORY_OPERATION_INST]);
		if (this->cache_miss_per_type[MEMORY_OPERATION_INST] != 0) fprintf(output, "%d_Cache_Inst_Miss:    %lu\n", this->level, this->cache_miss_per_type[MEMORY_OPERATION_INST]);
		for (int32_t i = 0; i < MEMORY_OPERATION_LAST; i++){
			if (this->cache_count_per_type[i] > 0){
				//fprintf(output, "%d_Total_%s_Latency: %lu\n", this->level, get_enum_memory_operation_char((memory_operation_t) i), this->total_per_type[i]);
				fprintf(output, "%d_Avg._%s_Latency: %lu\n", this->level, get_enum_memory_operation_char((memory_operation_t) i), this->total_per_type[i]/this->cache_count_per_type[i]);
				fprintf(output, "%d_Min._%s_Latency: %lu\n", this->level, get_enum_memory_operation_char((memory_operation_t) i), this->min_per_type[i]);
				fprintf(output, "%d_Max._%s_Latency: %lu\n", this->level, get_enum_memory_operation_char((memory_operation_t) i), this->max_per_type[i]);
			}
		}

		if(this->get_cache_writeback()!=0) 
		fprintf(output, "%d_Cache_WriteBack:    %lu\n", this->level, this->get_cache_writeback());
		fprintf(output, "%d_Concurrent_Requests_Max_Reached:   %u\n", this->level, this->get_max_reached());
		fprintf(output, "%d_MSHR_Max_Reached:   %u\n", this->level, this->get_max_MSHR_reached());
		fprintf(output, "%d_Num_MSHR_Stall:   %u\n", this->level, this->get_num_mshr_stall());
	}

}

void cache_t::reset_statistics() {
	this->set_cache_access(0);
	this->set_cache_hit(0);
	this->set_cache_miss(0);
	this->set_cache_eviction(0);
	this->set_cache_install(0);
	this->set_cache_back_invalidation(0);
	this->set_cache_write_merged(0);
	this->set_cache_write_merged_missing(0);
	this->set_cache_merged_miss(0);
	for (int32_t i = 0; i < MEMORY_OPERATION_LAST; i++){
		this->cache_count_per_type[i] = 0;
		this->cache_hit_per_type[i] = 0;
		this->cache_miss_per_type[i] = 0;
		this->total_per_type[i] = 0;
		this->min_per_type[i] = 0;
		this->max_per_type[i] = 0;
	}

	this->set_cache_writeback(0);
	this->set_max_reached(0);
}
