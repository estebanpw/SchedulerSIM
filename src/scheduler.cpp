#include "scheduler.h"

void scheduler::set_nodes_list(std::vector<node *> * nodes){
    this->nodes = nodes;
}

job * scheduler::get_next_job(){
    return this->jobs_queue.front();
}

void scheduler::pop_next_job(){
    this->jobs_queue.erase(this->jobs_queue.begin());
}




// ****** Start Scheduler FIFO ****************************************************************************
scheduler_FIFO::scheduler_FIFO(){
    this->total_jobs_queued = 0;
}

double scheduler_FIFO::compute_priority(job * j){
    return 0.0;
}

bool scheduler_FIFO::job_fits_in_node(job * j, node * n){
    if(!n->get_state() || n->is_system_busy() > 0) return false;

    if(j->CPU_requested <= n->get_available_cores() && (double) j->MEM_requested <= n->get_free_memory()){
        // Job fits in this node
        return true;
    }
    return false;
}

void scheduler_FIFO::queue_job(job * j){
    // For FIFO it is just priority based 
    j->priority = this->compute_priority(j);

    this->jobs_queue.push_back(j);
}

int scheduler_FIFO::deploy_jobs(){
    for(std::vector<node *>::iterator it = this->nodes->begin() ; it != this->nodes->end(); ++it){
        if(job_fits_in_node(this->jobs_queue.front(), *it)){

            return 1;
        }
    }
    return 0;
}

// ****** End Scheduler FIFO ****************************************************************************