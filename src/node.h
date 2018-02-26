#pragma once

#include "structs.h"
#include "core.h"
#include "common-functions.h"
#include <vector>
#include <iostream>

class node
{
private:
    uint64_t id_node;
    char node_name[DATA_LEN];
    uint64_t n_cores;
    double total_memory;
    double used_memory;
    uint64_t penalty_boot;
    uint64_t penalty_shutdown;
    uint64_t cost_per_second;
    std::vector<core *> cores;

    bool node_state;
    uint64_t delay_time;

public:
    node(uint64_t id_node, char * node_name, uint64_t n_cores, double memory, uint64_t penalty_boot, uint64_t penalty_shutdown, uint64_t cost_per_second);
    void compute(uint64_t quantum, uint64_t time);
    bool get_state(){ return this->node_state; }
    uint64_t is_system_busy(){ return this->delay_time; }
    void turn_on(){ this->node_state = true; this->delay_time = this->penalty_boot; }
    void turn_off(){ this->node_state = false; this->delay_time = this->penalty_shutdown; } // KILL signals should be distributed

    void print_node(){ std::cout << "Node " << id_node << ":" << node_name << " [" << n_cores << "CPUs " << total_memory << "GB" << "]" << std::endl; }
    char * get_name(){ return node_name; }
};