#pragma once

#include "structs.h"
#include "node.h"
#include "sysclock.h"
#include "common-functions.h"
#include "io-functions.h"
#include "log-recorder.h"
#include "job.h"
#include <stdarg.h>
#include <queue>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>

class scheduler
{
protected:
    uint64_t total_jobs_queued = 0;
    uint64_t backfill_frames = 0;
    std::vector<node *> * nodes;
    std::vector<load_on_node *> load_in_nodes;
    
    uint64_t * expected_CPU_load;
    uint64_t * expected_MEM_load;
    uint64_t modulus = 0;

public:
    //scheduler(std::vector<node *> * nodes);    
    virtual void queue_job(job * j, uint64_t t) = 0;
    virtual void deploy_jobs(uint64_t t) = 0;
    virtual double compute_priority(job * j, uint64_t t) = 0;
    virtual void manage_nodes_state() = 0;
    virtual uint64_t get_queued_jobs_size() = 0;
    virtual job * get_next_job() = 0;
    virtual void pop_next_job() = 0;
    bool job_fits_in_node(job * j, node * n, uint64_t t);
    void assign_grain_to_backfill(uint64_t frames);
    void set_nodes_list(std::vector<node *> * nodes);
};



class scheduler_FIFO : public scheduler
{
private:
    std::vector<job *> * jobs_queue;
public:
    scheduler_FIFO();
    void queue_job(job * j, uint64_t t);
    void deploy_jobs(uint64_t t);
    double compute_priority(job * j, uint64_t t);
    void manage_nodes_state();
    uint64_t get_queued_jobs_size(){ return this->jobs_queue->size(); }
    job * get_next_job();
    void pop_next_job();
};

// Needed to encapsulate the compare function
struct class_comp_short_jobs
{
public:
    bool operator() (const job * j1, const job * j2) const { return (j1->wall_time_clocks < j2->wall_time_clocks); }
};

class scheduler_SHORT : public scheduler
{
private:    
    std::multiset<job *, class_comp_short_jobs> * jobs_set;  // function pointer as Compare
public:
    scheduler_SHORT();
    void queue_job(job * j, uint64_t t);
    void deploy_jobs(uint64_t t);
    double compute_priority(job * j, uint64_t t);
    void manage_nodes_state();
    uint64_t get_queued_jobs_size(){ return jobs_set->size(); }
    job * get_next_job();
    void pop_next_job();
};

// Needed to encapsulate the compare function
struct class_comp_priority_jobs
{
public:
    bool operator() (const job * j1, const job * j2) const { return (j1->priority > j2->priority); } // Higher priority first
};

class scheduler_PRIORITY : public scheduler
{
private:    
    std::multiset<job *, class_comp_priority_jobs> * jobs_set;  // function pointer as Compare
    double w, q, e, c, m;
public:
    scheduler_PRIORITY(double w, double q, double e, double c, double m);
    void queue_job(job * j, uint64_t t);
    void deploy_jobs(uint64_t t);
    double compute_priority(job * j, uint64_t t);
    void recompute_priorities_queue(uint64_t t);
    void manage_nodes_state();
    uint64_t get_queued_jobs_size(){ return jobs_set->size(); }
    job * get_next_job();
    void pop_next_job();
};
