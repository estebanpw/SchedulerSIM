#include "node.h"

node::node(uint64_t id_node, char * node_name, uint64_t n_cores, double memory, uint64_t penalty_boot, uint64_t penalty_shutdown, uint64_t cost_per_second)
{
	this->id_node = id_node;
    strcpy(this->node_name, node_name);
    this->n_cores = n_cores;

    for(uint64_t i=0; i<this->n_cores; i++){
        this->cores.push_back(new core());
        this->load_in_cores.push_back(new jobs_per_core(this->cores.back()));
    }
    //printf(" total cores in node %s is %" PRIu64" and %" PRIu64 ",\n", this->node_name, this->n_cores, this->cores.size());

    this->total_memory = memory;
    this->used_memory = 0.0;

    this->penalty_boot = penalty_boot;
    this->penalty_shutdown = penalty_shutdown;

    this->cost_per_second = cost_per_second;
    this->node_state = false;

    this->delay_clocks = 0;
    this->efficient_t_jobs = 0;

    this->how_the_scheduler_wants_it = false;
    this->time_online = 0;
    this->time_offline = 0;
}

std::queue<job *> * node::compute(uint64_t t){
    
    //uint64_t prev_n_threads = n_threads;

    std::queue<job *> * finished_jobs = new std::queue<job *>; 
    if(t == delay_clocks){
        
        if(this->node_state){ 
            LOG->record(4, NODE_ON, t/QUANTUMS_PER_SEC, "Node ", this->node_name, " is on. Hooray!"); 
            this->time_online = 0; 
            this->time_offline = 0;
        } 
        if(!this->node_state){ 
            LOG->record(4, NODE_ON, t/QUANTUMS_PER_SEC, "Node ", this->node_name, " is off. See you!");
            this->time_online = 0; 
            this->time_offline = 0;
        }     
    }else if(t > delay_clocks && this->node_state){ // System must be on and not busy

        this->time_online++;
        // Perform computations on each core
        if(this->efficient_get_node_CPU_load() > (double) 0){

            
            for(std::vector<core *>::iterator it = this->cores.begin() ; it != this->cores.end(); ++it){
                job * job_state = (*it)->compute(t);
                
                if(job_state != NULL){
                    
                    (*it)->decrease_load();
                    finished_jobs->push(job_state);
                    this->efficient_t_jobs--;
                }
            }
            
        }
           
    }else{
        this->time_offline++;
    }
    //n_threads = prev_n_threads;
    return finished_jobs;
}

void * node::compute_pthreaded(void * a){
    Computing_threads_data * data_in_thread = (Computing_threads_data *) a;
    
    for(uint64_t i=data_in_thread->from; i<data_in_thread->to; i++){
        job * job_state = (data_in_thread->cores->at(i))->compute(data_in_thread->t);
        
        if(job_state != NULL){
            
            (data_in_thread->cores->at(i))->decrease_load();
            data_in_thread->finished_jobs.push(job_state);
        }
    }
    return NULL;
}

uint64_t node::get_available_cores(){
    if(MULTITHREADING){
        // Not implemented yet TODO
        if(this->efficient_t_jobs < this->n_cores*MAX_THREADS_PER_CPU) return ((this->n_cores*MAX_THREADS_PER_CPU) - this->efficient_t_jobs); else return 0;
    }else{
        // 
        if(this->efficient_t_jobs < this->n_cores) return (this->n_cores - this->efficient_t_jobs); else return 0;
    }
}

double node::get_node_CPU_load(){
    double load = 0.0;
    for(std::vector<core *>::iterator it = this->cores.begin() ; it != this->cores.end(); ++it){
            load += ((*it)->get_core_load() > 0) ? (1.0) : (0.0) ;
    }
    return 100.0 * (load / (double) this->n_cores);
}

double node::efficient_get_node_CPU_load(){
    
    return 100.0 * (this->efficient_t_jobs / (double) this->n_cores);
}

double node::get_node_MEM_load(){
    return 100.0 * (this->used_memory / this->total_memory);
}

void node::insert_job(job * j){
    // assumed job fits
    std::sort(this->load_in_cores.begin(), this->load_in_cores.end(), core::compare_two_core_loads);
    this->used_memory += j->MEM_requested;
    
    if(MULTITHREADING == false){
        for(uint64_t i=0; i<(uint64_t)j->CPU_requested; i++){

            if(this->load_in_cores.at(i) == NULL){
                terror("Bad allocation of resources");
            }else{
                this->load_in_cores.at(i)->at_core->insert_job(j);
                //printf("insert %" PRIu64 " cores used: %" PRIu64 "\n", j->job_internal_identifier, i);
                this->load_in_cores.at(i)->at_core->increase_load();
                this->efficient_t_jobs++;
            }
        }
    }else{
        uint64_t mod_cpu;
        for(uint64_t i=0; i<(uint64_t)j->CPU_requested; i++){

            mod_cpu = i % this->n_cores;

            if(this->load_in_cores.at(mod_cpu) == NULL){
                terror("Bad allocation of resources");
            }else{
                this->load_in_cores.at(mod_cpu)->at_core->insert_job(j);
                //printf("insert %" PRIu64 " cores used: %" PRIu64 "\n", j->job_internal_identifier, i);
                this->load_in_cores.at(mod_cpu)->at_core->increase_load();
                this->efficient_t_jobs++;
            }
        }
    }

}