#include "scheduler.h"

void scheduler::set_nodes_list(std::vector<node *> * nodes){
    this->nodes = nodes;
    for(std::vector<node *>::iterator it = this->nodes->begin() ; it != this->nodes->end(); ++it){
        this->load_in_nodes.push_back(new load_on_node((*it)));
    }
}

bool scheduler::job_fits_in_node(job * j, node * n, uint64_t t){
    if(!n->get_state() || n->is_system_busy() > t){ 
        //printf("node state: %d sys_busy %" PRIu64 " t: %" PRIu64 "\n", n->get_state(), n->is_system_busy(), t);
        return false; 
    }

    if(j->CPU_requested <= n->get_available_cores() && (double) j->MEM_requested <= n->get_free_memory()){
        // Job fits in this node
        //printf("job %" PRIu64 " asks for %le cores and fits in node %s\n", j->job_internal_identifier, j->CPU_requested, n->get_name());
        //getchar();
        return true;
    }
    return false;
}

void scheduler::assign_grain_to_backfill(uint64_t frames){
    this->backfill_frames = frames;
    this->expected_CPU_load = (uint64_t *) std::calloc(frames, sizeof(uint64_t));
    if(this->expected_CPU_load == NULL) terror("Could not allocate backfill CPU windows");
    this->expected_MEM_load = (uint64_t *) std::calloc(frames, sizeof(uint64_t));
    if(this->expected_MEM_load == NULL) terror("Could not allocate backfill MEM windows");
}

void add_job_to_expected_load(job * j){
    for(uint64_t i=0; i<j->wall_time_clocks/QUANTUMS_IN_FRAME && i<this->backfill_frames ; i++){
        this->expected_CPU_load[(this->modulus + i) % this->backfill_frames] += j->CPU_requested:
        this->expected_MEM_load[(this->modulus + i) % this->backfill_frames] += j->MEM_requested;
    }
}


// ****** Start Scheduler FIFO nodes online 24/7 ****************************************************************************
scheduler_FIFO::scheduler_FIFO(){
    this->total_jobs_queued = 0;
    this->jobs_queue = new std::vector<job *>();
}

job * scheduler_FIFO::get_next_job(){
    return this->jobs_queue->front();
}

void scheduler_FIFO::pop_next_job(){
    this->jobs_queue->erase(this->jobs_queue->begin());
}

double scheduler_FIFO::compute_priority(job * j, uint64_t t){
    return this->jobs_queue->size();
}

void scheduler_FIFO::manage_nodes_state(){
    
    for(std::vector<node *>::iterator it = this->nodes->begin() ; it != this->nodes->end(); ++it){
        // Policies for power 
        
        // Basic policy: let them all on
        (*it)->how_the_scheduler_wants_it = true;
    }
}

void scheduler_FIFO::queue_job(job * j, uint64_t t){
    // For FIFO it is just priority based 
    j->priority = this->compute_priority(j, t);

    this->jobs_queue->push_back(j);
}


void scheduler_FIFO::deploy_jobs(uint64_t t){
    // Sort nodes according to CPU load
    std::sort(this->load_in_nodes.begin(), this->load_in_nodes.end(), node::compare_two_node_loads);
    
    if(BACKFILL == true){
        std::vector<uint64_t> remove_jobs;
        uint64_t kill_id = 0;
        
        for(std::vector<job *>::iterator jobit = this->jobs_queue->begin() ; jobit != this->jobs_queue->end(); ++jobit){
            for(std::vector<node *>::iterator it = this->nodes->begin() ; it != this->nodes->end(); ++it){
                if(job_fits_in_node(*jobit, *it, t)){
                    (*jobit)->state = 'R';
                    (*it)->insert_job(*jobit);
                    remove_jobs.push_back(kill_id);
                    //printf("job was assigned to node! j=%" PRIu64 " to %s\n", (*jobit)->job_internal_identifier, (*it)->get_name());
                    (*jobit)->real_start_clocks = t;
                    LOG->record(4, JOB_START, t * QUANTUMS_PER_SEC, this->get_queued_jobs_size() - remove_jobs.size(), (*jobit)->to_string().c_str());
                    break;
                }else{
                    //printf("does not fit requires %le and %le %" PRIu64 "\n", (*jobit)->CPU_requested, (*jobit)->MEM_requested, (*jobit)->job_internal_identifier);
                }
            }
            ++kill_id;
        }
        uint64_t amount_removed = 0;
        for(std::vector<uint64_t>::iterator rem_job = remove_jobs.begin() ; rem_job != remove_jobs.end(); ++rem_job){
            this->jobs_queue->erase(this->jobs_queue->begin()+(*rem_job) - amount_removed);
            amount_removed++;
        }   
    }else{
        // No backfill
                
        job * jobit = this->jobs_queue->front();
        for(std::vector<node *>::iterator it = this->nodes->begin() ; it != this->nodes->end(); ++it){
            if(job_fits_in_node(jobit, *it, t)){
                jobit->state = 'R';
                (*it)->insert_job(jobit);
                jobit->real_start_clocks = t;
                this->jobs_queue->erase(this->jobs_queue->begin());
                LOG->record(4, JOB_START, t * QUANTUMS_PER_SEC, this->get_queued_jobs_size(), (jobit)->to_string().c_str());
                break;
            }
        }
    }
}



// ****** End Scheduler FIFO nodes online 24/7 ****************************************************************************



// ****** Start Scheduler Short-first nodes online 24/7 ****************************************************************************

scheduler_SHORT::scheduler_SHORT(){
    this->total_jobs_queued = 0;
    this->jobs_set = new std::multiset<job *, class_comp_short_jobs>();
}

job * scheduler_SHORT::get_next_job(){
    return (*this->jobs_set->begin());
}

void scheduler_SHORT::pop_next_job(){
    this->jobs_set->erase(this->jobs_set->begin());
}

double scheduler_SHORT::compute_priority(job * j, uint64_t t){
    return j->wall_time_clocks;
}

void scheduler_SHORT::manage_nodes_state(){
    
    for(std::vector<node *>::iterator it = this->nodes->begin() ; it != this->nodes->end(); ++it){
        // Policies for power 
        
        // Basic policy: let them all on
        (*it)->how_the_scheduler_wants_it = true;
    }
}

void scheduler_SHORT::queue_job(job * j, uint64_t t){
    // For FIFO it is just priority based 
    j->priority = this->compute_priority(j, t);

    this->jobs_set->insert(j);
}

void scheduler_SHORT::deploy_jobs(uint64_t t){
    std::sort(this->load_in_nodes.begin(), this->load_in_nodes.end(), node::compare_two_node_loads);
    
    if(BACKFILL == true){
        
        std::vector<uint64_t> remove_jobs;
        uint64_t kill_id = 0;
        
        for(std::multiset<job *>::iterator jobit = this->jobs_set->begin() ; jobit != this->jobs_set->end(); ++jobit){
            for(std::vector<node *>::iterator it = this->nodes->begin() ; it != this->nodes->end(); ++it){
                if(job_fits_in_node(*jobit, *it, t)){
                    (*jobit)->state = 'R';
                    (*it)->insert_job(*jobit);
                    remove_jobs.push_back(kill_id);
                    //printf("job was assigned to node! j=%" PRIu64 " to %s\n", (*jobit)->job_internal_identifier, (*it)->get_name());
                    (*jobit)->real_start_clocks = t;
                    LOG->record(4, JOB_START, t * QUANTUMS_PER_SEC, this->get_queued_jobs_size() - remove_jobs.size(), (*jobit)->to_string().c_str());
                    break;
                }else{
                    //printf("does not fit requires %le and %le %" PRIu64 "\n", (*jobit)->CPU_requested, (*jobit)->MEM_requested, (*jobit)->job_internal_identifier);
                }
            }
            ++kill_id;
        }
        uint64_t amount_removed = 0;
        std::multiset<job *>:: iterator it;
        for(std::vector<uint64_t>::iterator rem_job = remove_jobs.begin() ; rem_job != remove_jobs.end(); ++rem_job){   
            it = this->jobs_set->begin();
            std::advance(it, (*rem_job) - amount_removed);
            this->jobs_set->erase(it);
            amount_removed++;
        }
        
    }else{
        // No backfill
                
        job * jobit = (*this->jobs_set->begin());
        for(std::vector<node *>::iterator it = this->nodes->begin() ; it != this->nodes->end(); ++it){
            if(job_fits_in_node(jobit, *it, t)){
                jobit->state = 'R';
                (*it)->insert_job(jobit);
                jobit->real_start_clocks = t;
                this->jobs_set->erase(this->jobs_set->begin());
                LOG->record(4, JOB_START, t * QUANTUMS_PER_SEC, this->get_queued_jobs_size(), (jobit)->to_string().c_str());
                break;
            }
        }
    }
}


// ****** End scheduler_SHORT nodes online 24/7 ****************************************************************************


// ****** Start Scheduler Priority nodes online 24/7 ****************************************************************************

scheduler_PRIORITY::scheduler_PRIORITY(double w, double q, double e, double c, double m){
    this->total_jobs_queued = 0;
    this->w = w;
    this->q = q;
    this->e = e;
    this->c = c;
    this->m = m;
    this->jobs_set = new std::multiset<job *, class_comp_priority_jobs>();
}

job * scheduler_PRIORITY::get_next_job(){
    return (*this->jobs_set->begin());
}

void scheduler_PRIORITY::pop_next_job(){
    this->jobs_set->erase(this->jobs_set->begin());
}

double scheduler_PRIORITY::compute_priority(job * j, uint64_t t){
    
    return (this->w * j->wall_time_clocks + this->q * (t - j->real_submit_clocks) + this->e * ((t - j->real_submit_clocks)/j->wall_time_clocks) + this->c * j->CPU_requested + this->m * j->MEM_requested);    
}

void scheduler_PRIORITY::manage_nodes_state(){
    
    for(std::vector<node *>::iterator it = this->nodes->begin() ; it != this->nodes->end(); ++it){
        // Policies for power 
        
        // Basic policy: let them all on
        (*it)->how_the_scheduler_wants_it = true;
    }
}

void scheduler_PRIORITY::queue_job(job * j, uint64_t t){
    // For FIFO it is just priority based 
    j->priority = this->compute_priority(j, t);

    // Every time a job is inserted, priorities are recomputed
    this->jobs_set->insert(j);

    this->recompute_priorities_queue(t);

}

void scheduler_PRIORITY::recompute_priorities_queue(uint64_t t){
    std::multiset<job *, class_comp_priority_jobs> * aux_jobs_set = new std::multiset<job *, class_comp_priority_jobs>();
    for(std::multiset<job *>::iterator jobit = this->jobs_set->begin() ; jobit != this->jobs_set->end(); ++jobit){
        this->compute_priority((*jobit), t);
        aux_jobs_set->insert(*jobit);
    }
    this->jobs_set->swap(*aux_jobs_set);
    delete aux_jobs_set;
    
}

void scheduler_PRIORITY::deploy_jobs(uint64_t t){
    std::sort(this->load_in_nodes.begin(), this->load_in_nodes.end(), node::compare_two_node_loads);
    
    if(BACKFILL == true){
        
        std::vector<uint64_t> remove_jobs;
        uint64_t kill_id = 0;
        
        for(std::multiset<job *>::iterator jobit = this->jobs_set->begin() ; jobit != this->jobs_set->end(); ++jobit){
            for(std::vector<node *>::iterator it = this->nodes->begin() ; it != this->nodes->end(); ++it){
                if(job_fits_in_node(*jobit, *it, t)){
                    (*jobit)->state = 'R';
                    (*it)->insert_job(*jobit);
                    remove_jobs.push_back(kill_id);
                    //printf("job was assigned to node! j=%" PRIu64 " to %s\n", (*jobit)->job_internal_identifier, (*it)->get_name());
                    (*jobit)->real_start_clocks = t;
                    LOG->record(4, JOB_START, t * QUANTUMS_PER_SEC, this->get_queued_jobs_size() - remove_jobs.size(), (*jobit)->to_string().c_str());
                    break;
                }else{
                    //printf("does not fit requires %le and %le %" PRIu64 "\n", (*jobit)->CPU_requested, (*jobit)->MEM_requested, (*jobit)->job_internal_identifier);
                }
            }
            ++kill_id;
        }
        uint64_t amount_removed = 0;
        std::multiset<job *>:: iterator it;
        for(std::vector<uint64_t>::iterator rem_job = remove_jobs.begin() ; rem_job != remove_jobs.end(); ++rem_job){   
            it = this->jobs_set->begin();
            std::advance(it, (*rem_job) - amount_removed);
            this->jobs_set->erase(it);
            amount_removed++;
        }
        
    }else{
        // No backfill
                
        job * jobit = (*this->jobs_set->begin());
        for(std::vector<node *>::iterator it = this->nodes->begin() ; it != this->nodes->end(); ++it){
            if(job_fits_in_node(jobit, *it, t)){
                jobit->state = 'R';
                (*it)->insert_job(jobit);
                jobit->real_start_clocks = t;
                this->jobs_set->erase(this->jobs_set->begin());
                LOG->record(4, JOB_START, t * QUANTUMS_PER_SEC, this->get_queued_jobs_size(), (jobit)->to_string().c_str());
                break;
            }
        }
    }
}


// ****** End scheduler_SHORT nodes online 24/7 ****************************************************************************