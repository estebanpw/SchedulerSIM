#include "job.h"

job::job(uint64_t job_id, double CPU_min_use, double CPU_average_use, double CPU_max_use, double CPU_requested,
    double MEM_min_use, double MEM_average_use, double MEM_max_use, double MEM_requested, uint64_t run_time_seconds,
    uint64_t wall_time_seconds, uint64_t submit_time_seconds, uint64_t start_time_seconds, uint64_t end_time_seconds,
    char * account_name, char * job_name, int exit_code){
    
    this->job_id = job_id;
    this->CPU_min_use = CPU_min_use;
    this->CPU_average_use = CPU_average_use;
    this->CPU_max_use = CPU_max_use;
    this->CPU_requested = CPU_requested;
    this->MEM_min_use = MEM_min_use;
    this->MEM_average_use = MEM_average_use;
    this->MEM_max_use = MEM_max_use;
    this->MEM_requested = MEM_requested;
    this->run_time_seconds = run_time_seconds;
    this->wall_time_seconds = wall_time_seconds;
    this->submit_time_seconds = submit_time_seconds;
    this->start_time_seconds = start_time_seconds;
    this->end_time_seconds = end_time_seconds;
    strcpy(this->account_name, account_name);
    strcpy(this->job_name, job_name);
    this->exit_code = exit_code;
    this->remaining_quantums = this->run_time_seconds * QUANTUMS_PER_SEC;
}