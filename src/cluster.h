#pragma once

#include "structs.h"
#include "node.h"
#include "sysclock.h"
#include "common-functions.h"
#include "io-functions.h"
#include "log.h"
#include "scheduler.h"
#include <stdarg.h>
#include <queue>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

class cluster
{
private:
    std::vector<job *> input_jobs;
    std::vector<node *> nodes;
    scheduler * sch;
    Jobs_completition * table_of_jobs_completition;
    uint64_t quantum;
    uint64_t nodes_online;
    sysclock * syscl;

public:
    cluster(FILE * f_input_jobs, scheduler * sch);
    void compute();
    void add_node(node * n);
    void add_nodes_from_file(FILE * f);
    bool boot_node(node * n);
    void boot_all_nodes();
    bool shutdown_node(node * n);
    void shutdown_all_nodes();
    std::vector<node *> * get_nodes_list(){ return &this->nodes; }
    const char * print_cluster_usage();
    void broadcast(int count, ...);
    void insert_job_waiting_signal(job * j){ this->table_of_jobs_completition[j->job_internal_identifier].n_cores_launched = j->CPU_requested; this->table_of_jobs_completition[j->job_internal_identifier].n_cores_finished = 0; }
    bool add_finished_core_and_check(job * j);
    ~cluster();
};