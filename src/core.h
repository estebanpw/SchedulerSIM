#pragma once

#include "structs.h"
#include "job.h"
#include <queue>

class core
{
private:
    std::queue<job *> jobs;

public:
    core();
    void compute(uint64_t quantum, uint64_t time);
    void insert_job(job * j);
    
    
    uint64_t get_core_load(){ return this->jobs.size(); }
};