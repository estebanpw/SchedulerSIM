#pragma once

#include "structs.h"
#include <vector>
#include <string>

class job
{
public:

    uint64_t    job_id;
    uint64_t    job_internal_identifier;
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
    
    int         real_exit_code;
    uint64_t    real_submit_clocks;
    uint64_t    real_start_clocks;
    uint64_t    real_end_clocks;
    uint64_t    wall_time_clocks;
    char        state;

    double priority;


    job(uint64_t job_id, uint64_t job_internal_identifier, double CPU_min_use, double CPU_average_use, double CPU_max_use, double CPU_requested,
    double MEM_min_use, double MEM_average_use, double MEM_max_use, double MEM_requested, uint64_t run_time_seconds,
    uint64_t wall_time_seconds, uint64_t submit_time_seconds, uint64_t start_time_seconds, uint64_t end_time_seconds,
    char * account_name, char * job_name, int exit_code);

    uint64_t get_remaining_quantums(){ return this->remaining_quantums; }
    void compute(){ this->remaining_quantums--; this->wall_time_clocks--; }
    uint64_t get_submit_time(){ return this->submit_time_seconds; }
    char * get_name(){ return this->job_name; }
    uint64_t get_job_id(){ return this->job_id; }
    static bool compare_jobs_order(job * a, job * b){ return (a->submit_time_seconds < b->submit_time_seconds); }
    std::string to_string();

};

typedef struct jobs_completition{
    uint64_t n_cores_launched;
    uint64_t n_cores_finished;
    job * j;
    int exit_code;
} Jobs_completition;