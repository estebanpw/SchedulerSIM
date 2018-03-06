#include "node.h"

node::node(uint64_t id_node, char * node_name, uint64_t n_cores, double memory, uint64_t penalty_boot, uint64_t penalty_shutdown, uint64_t cost_per_second)
{
	this->id_node = id_node;
    strcpy(this->node_name, node_name);
    this->n_cores = n_cores;

    for(uint64_t i=0; i<this->n_cores; i++){
        this->cores.push_back(new core());
        this->load_in_cores.push_back(new jobs_per_core(this->cores.back()));
    }
    //printf(" total cores in node %s is %" PRIu64" and %" PRIu64 ",\n", this->node_name, this->n_cores, this->cores.size());

    this->total_memory = memory;
    this->used_memory = 0.0;

    this->penalty_boot = penalty_boot;
    this->penalty_shutdown = penalty_shutdown;

    this->cost_per_second = cost_per_second;
    this->node_state = false;

    this->delay_clocks = 0;
}

std::queue<job *> * node::compute(uint64_t t){
    
    std::queue<job *> * finished_jobs = new std::queue<job *>; 
    if(t == delay_clocks){
        
        if(this->node_state) LOG->record(4, NODE_ON, t/QUANTUMS_PER_SEC, "Node ", this->node_name, " is on. Hooray!");
        if(!this->node_state) LOG->record(4, NODE_ON, t/QUANTUMS_PER_SEC, "Node ", this->node_name, " is off. See you!");
    }else if(t > delay_clocks){
        // Perform computations on each core
        for(std::vector<core *>::iterator it = this->cores.begin() ; it != this->cores.end(); ++it){
            job * job_state = (*it)->compute(t);
            
            if(job_state != NULL){
                
                (*it)->decrease_load();
                finished_jobs->push(job_state);
            }
        }   
    }
    return finished_jobs;
}

uint64_t node::get_available_cores(){
    if(MULTITHREADING){
        // Not implemented yet TODO
        return 0;
    }else{
        // 
        uint64_t n_free_cores = 0;
        for(std::vector<core *>::iterator it = this->cores.begin() ; it != this->cores.end(); ++it){
            n_free_cores += ((*it)->get_core_load() == 0) ? (1) : (0) ;
        }
        return n_free_cores;
    }
}

double node::get_node_CPU_load(){
    double load = 0.0;
    for(std::vector<core *>::iterator it = this->cores.begin() ; it != this->cores.end(); ++it){
            load += ((*it)->get_core_load() > 0) ? (1.0) : (0.0) ;
    }
    return 100.0 * (load / (double) this->n_cores);
}

double node::get_node_MEM_load(){
    return 100.0 * (this->used_memory / this->total_memory);
}

void node::insert_job(job * j){
    // assumed job fits
    std::sort(this->load_in_cores.begin(), this->load_in_cores.end(), core::compare_two_core_loads);
    this->used_memory += j->MEM_requested;
    for(uint64_t i=0; i<(uint64_t)j->CPU_requested; i++){

        if(this->load_in_cores.at(i) == NULL) 
            terror("Bad allocation of resources");
        else
            this->load_in_cores.at(i)->at_core->insert_job(j);
            //printf("insert %" PRIu64 " cores used: %" PRIu64 "\n", j->job_internal_identifier, i);
            this->load_in_cores.at(i)->at_core->increase_load();
    }

}