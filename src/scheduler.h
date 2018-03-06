#pragma once

#include "structs.h"
#include "node.h"
#include "sysclock.h"
#include "common-functions.h"
#include "io-functions.h"
#include "log.h"
#include "job.h"
#include <stdarg.h>
#include <queue>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

class scheduler
{
protected:
    uint64_t total_jobs_queued;
    std::vector<job *> jobs_queue;
    std::vector<node *> * nodes;
    std::vector<load_on_node *> load_in_nodes;

public:
    //scheduler(std::vector<node *> * nodes);    
    virtual void queue_job(job * j) = 0;
    virtual void deploy_jobs(uint64_t t) = 0;
    virtual double compute_priority(job * j) = 0;
    virtual bool job_fits_in_node(job * j, node * n, uint64_t t) = 0;
    void set_nodes_list(std::vector<node *> * nodes);
    job * get_next_job();
    void pop_next_job();
};

class scheduler_FIFO : public scheduler
{
public:
    scheduler_FIFO();
    void queue_job(job * j);
    void deploy_jobs(uint64_t t);
    double compute_priority(job * j);
    bool job_fits_in_node(job * j, node * n, uint64_t t);
};