#pragma once

#include "structs.h"
#include "core.h"
#include "log-recorder.h"
#include "common-functions.h"
#include <pthread.h>
#include <vector>
#include <queue>
#include <algorithm>
#include <iostream>

typedef struct{
    std::queue<job *> finished_jobs;
    uint64_t from;
    uint64_t to;
    std::vector<core *> * cores;
    uint64_t t;
} Computing_threads_data;

class node;

struct load_on_node{
    node * at_node;
    load_on_node(node * n): at_node(n) {}
};

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
    uint64_t efficient_t_jobs;
    std::vector<core *> cores;
    std::vector<jobs_per_core *> load_in_cores;
    

    bool node_state;
    uint64_t delay_clocks;

public:
    node(uint64_t id_node, char * node_name, uint64_t n_cores, double memory, uint64_t penalty_boot, uint64_t penalty_shutdown, uint64_t cost_per_second);
    std::queue<job *> * compute(uint64_t t);
    bool get_state(){ return this->node_state; }
    uint64_t is_system_busy(){ return this->delay_clocks; }
    void turn_on(uint64_t t){ this->node_state = true; this->delay_clocks = this->penalty_boot*QUANTUMS_PER_SEC + t; }
    void turn_off(uint64_t t){ this->node_state = false; this->delay_clocks = this->penalty_shutdown*QUANTUMS_PER_SEC + t; } // KILL signals should be distributed

    void print_node(){ std::cout << "Node " << id_node << ":" << node_name << " [" << n_cores << "CPUs " << total_memory << "GB" << "]" << std::endl; }
    double get_node_CPU_load();
    double efficient_get_node_CPU_load();
    double get_node_MEM_load();

    uint64_t get_number_of_cores(){ return this->n_cores; }
    uint64_t get_available_cores();
    double get_free_memory(){ return (this->total_memory - this->used_memory); }
    void free_memory_from_process(double memory){ this->used_memory -= memory; }
    uint64_t get_cost(){ return this->cost_per_second; }
    char * get_name(){ return node_name; }

    void insert_job(job * j);

    // For descending sort (more busy nodes first)
    static bool compare_two_node_loads(load_on_node * a, load_on_node * b){ return (a->at_node->efficient_get_node_CPU_load() > b->at_node->efficient_get_node_CPU_load()); }
    static void * compute_pthreaded(void * a);
};
