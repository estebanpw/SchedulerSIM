#pragma once

#include "structs.h"
#include <vector>

class job
{
private:

    uint64_t    job_id;
    double      CPU_min_use;
    double      CPU_average_use;
    double      CPU_max_use;
    double      CPU_requested;
    double      MEM_min_use;
    double      MEM_average_use;
    double      MEM_max_use;
    double      MEM_requested;
    uint64_t    run_time_seconds;
    uint64_t    wall_time_seconds;
    uint64_t    submit_time_seconds;
    uint64_t    start_time_seconds;
    uint64_t    end_time_seconds;
    uint64_t    remaining_quantums;
    char        account_name[DATA_LEN];
    char        job_name[DATA_LEN];
    int         exit_code;

public:
    job(uint64_t job_id, double CPU_min_use, double CPU_average_use, double CPU_max_use, double CPU_requested,
    double MEM_min_use, double MEM_average_use, double MEM_max_use, double MEM_requested, uint64_t run_time_seconds,
    uint64_t wall_time_seconds, uint64_t submit_time_seconds, uint64_t start_time_seconds, uint64_t end_time_seconds,
    char * account_name, char * job_name, int exit_code);

    uint64_t get_remaining_quantums(){ return this->remaining_quantums; }
    void compute(uint64_t quantum){ this->remaining_quantums -= quantum; }

};