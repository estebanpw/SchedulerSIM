#include "job.h"

job::job(uint64_t job_id, uint64_t job_internal_identifier, double CPU_min_use, double CPU_average_use, double CPU_max_use, double CPU_requested,
    double MEM_min_use, double MEM_average_use, double MEM_max_use, double MEM_requested, uint64_t run_time_seconds,
    uint64_t wall_time_seconds, uint64_t submit_time_seconds, uint64_t start_time_seconds, uint64_t end_time_seconds,
    char * account_name, char * job_name, int exit_code){
    
    this->job_id = job_id;
    this->job_internal_identifier = job_internal_identifier;
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
    this->wall_time_clocks = this->wall_time_seconds * QUANTUMS_PER_SEC;
    

    this->priority = 0.0;

    this->real_submit_clocks = 0;
    this->real_start_clocks = 0;
    this->real_end_clocks = 0;
    this->real_exit_code = 0;
    this->state = 'W';
}

std::string job::to_string(){
    std::string s = " ID:" + std::to_string(this->job_id);
    s += " STATE:";
    s += this->state;
    s += " SUBMIT:";
    s += std::to_string(this->real_submit_clocks);
    s += " START:";
    s += std::to_string(this->real_start_clocks);
    s += " NAME:";
    s += this->job_name;
    s += " CPUs:";
    s += std::to_string(this->CPU_requested);
    s += " MEM:";
    s += std::to_string(this->MEM_requested);
    s += " REM-QUA:";
    s += std::to_string(this->remaining_quantums);
    s += " PRIO:";
    s += std::to_string(this->priority);
    s += '\0';
    return s;
}