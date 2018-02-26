#pragma once

#include "structs.h"
#include "node.h"
#include "sysclock.h"
#include "common-functions.h"
#include "io-functions.h"
#include <stdarg.h>
#include <queue>
#include <iostream>
#include <vector>
#include <algorithm>

class cluster
{
private:
    std::vector<job *> input_jobs;
    std::vector<job *> jobs_queue;
    std::vector<node *> nodes;
    uint64_t quantum;
    sysclock * syscl;

public:
    cluster(uint64_t quantum, FILE * f_input_jobs);
    void compute();
    void add_node(node * n);
    void add_nodes_from_file(FILE * f);
    bool boot_node(node * n);
    void boot_all_nodes();
    bool shutdown_node(node * n);
    void shutdown_all_nodes();
    void broadcast(int count, ...);
};