#pragma once

#include "structs.h"
#include "job.h"
#include "log-recorder.h"
#include <queue>

class core;

struct jobs_per_core{
    core * at_core;
    jobs_per_core(core * c2): at_core(c2) {}
};

class core
{
private:
    std::queue<job *> jobs;
    uint64_t current_load;

public:
    core();
    job * compute(uint64_t t);
    void insert_job(job * j);
    uint64_t get_max_clock_remaining();
    
    uint64_t get_core_load(){ return this->jobs.size(); }
    void increase_load(){ this->current_load++; }
    uint64_t get_current_load(){ return this->current_load; }
    void decrease_load(){ if(this->current_load > 0) this->current_load--; }
    static bool compare_two_core_loads(jobs_per_core * a, jobs_per_core * b){ return (a->at_core->get_current_load() < b->at_core->get_current_load()); }
};

