#include "node.h"

node::node(uint64_t id_node, char * node_name, uint64_t n_cores, double memory, uint64_t penalty_boot, uint64_t penalty_shutdown, uint64_t cost_per_second)
{
	this->id_node = id_node;
    strcpy(this->node_name, node_name);
    this->n_cores = n_cores;

    for(uint64_t i=0; i<this->n_cores; i++){
        this->cores.push_back(new core());
    }

    this->total_memory = memory;
    this->used_memory = 0.0;

    this->penalty_boot = penalty_boot;
    this->penalty_shutdown = penalty_shutdown;

    this->cost_per_second = cost_per_second;
    this->node_state = false;

    this->delay_time = 0;
}

void node::compute(uint64_t quantum, uint64_t time){
    if(delay_time > 0){
        delay_time--;
        if(delay_time == 0 && this->node_state) generic_broadcast(3, "Node ", this->node_name, " is on. Hooray!");
        if(delay_time == 0 && !this->node_state) generic_broadcast(3, "Node ", this->node_name, " is off. See you!");
    }else{
        // Perform computations on each core
        for(std::vector<core *>::iterator it = this->cores.begin() ; it != this->cores.end(); ++it){
            (*it)->compute(quantum, time);
        }   
    }
}