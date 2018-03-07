#include "cluster.h"

cluster::cluster(FILE * f_input_jobs, scheduler * sch){
    this->syscl = new sysclock();
    this->sch = sch;
    this->sch->set_nodes_list(this->get_nodes_list());
    this->nodes_online = 0;


    Picasso_row pr;
    this->broadcast(1, "Receiving input job list");
    uint64_t internal_job_id = 0;
    uint64_t last_job = 0xFFFFFFFFFFFFFFFF;
    
    while(!feof(f_input_jobs)){
        if(0 < picasso_read_row(f_input_jobs, &pr)){
            
            job * j = new job(pr.job_id, internal_job_id++, pr.CPU_min_use, pr.CPU_average_use, pr.CPU_max_use, pr.CPU_requested, 
            pr.MEM_min_use, pr.MEM_average_use, pr.MEM_max_use, pr.MEM_requested, pr.run_time_seconds, pr.wall_time_seconds,
            pr.submit_time_seconds, pr.start_time_seconds, pr.end_time_seconds, pr.account_name, pr.job_name, pr.exit_code);
            if(last_job != pr.job_id) this->input_jobs.push_back(j);
            last_job = pr.job_id;
        }
    }
    std::sort(this->input_jobs.begin(), this->input_jobs.end(), job::compare_jobs_order);
    

    this->table_of_jobs_completition = (Jobs_completition *) std::malloc(this->input_jobs.size() * sizeof(uint64_t) * 2);
    if(this->table_of_jobs_completition == NULL) terror("Could not allocate table of jobs completition");


}

cluster::~cluster(){
    std::free(this->table_of_jobs_completition);
}

int cluster::compute(){
    // Perform actions
    if(this->syscl->get_clock() % (QUANTUMS_IN_DAY) == 0 && this->syscl->get_time() > 0){
        LOG->record(4, SYS_USE, this->syscl->get_time(), this->sch->get_queued_jobs_size(), this->print_cluster_usage().c_str());
        this->broadcast(1, "A day goes by... ");
    } 
    
    // Check if there is a job waiting

    job * current_job = NULL;
    uint64_t t_finished = 0, t_total = 0;

    if(this->input_jobs.size() > 0){
        current_job = this->input_jobs.front();
        // If a job enters the system, submit it to the waiting queue
        while(current_job != NULL && current_job->get_submit_time() <= this->syscl->get_time() && this->input_jobs.size() > 0){

            
            
            //printf("input job list now %" PRIu64 "\n", this->input_jobs.size());
            
            /*
            for(std::vector<job *>::iterator it = this->input_jobs.begin() ; it != this->input_jobs.end(); ++it){
                printf("%" PRIu64 "\n", (*it)->job_internal_identifier);
            }
            */

            // Schedule it insert to job queue
            //this->broadcast(2, "job enter ", current_job->to_string().c_str());
            // Current mode is just FIFO
            LOG->record(4, SYS_USE, this->syscl->get_time(), this->sch->get_queued_jobs_size(), this->print_cluster_usage().c_str());
            this->sch->queue_job(current_job);
            ++t_total;
            LOG->record(4, JOB_ENTER, this->syscl->get_time(), this->sch->get_queued_jobs_size(), current_job->to_string().c_str());
            this->insert_job_waiting_signal(current_job);
            
            // Remove this job from input first
            this->input_jobs.erase(this->input_jobs.begin());

            current_job = this->input_jobs.front();
            //this->broadcast(2, "next job taken ", current_job->to_string().c_str());
            
        }
    }
    // Only attempt to deploy jobs once every K second
    if(this->syscl->get_clock() % (LOGIN_NODE_INTERVAL*QUANTUMS_PER_SEC) == 0 && this->sch->get_queued_jobs_size() > 0) this->sch->deploy_jobs(this->syscl->get_clock());

    // Compute quantums

    for(std::vector<node *>::iterator it = this->nodes.begin() ; it != this->nodes.end(); ++it){
        //printf("I think this takes a lot...%" PRIu64 "\n", this->syscl->get_clock());
        std::queue<job *> * finished_jobs = (*it)->compute(this->syscl->get_clock());
        //printf("correct?\n");
        
        // Delete jobs and broadcast finish time
        while(finished_jobs->size() > 0){
            
            job * j = finished_jobs->front();
            finished_jobs->pop();

            if(this->add_finished_core_and_check(j)){
                (*it)->free_memory_from_process(j->MEM_requested);
                LOG->record(4, SYS_USE, this->syscl->get_time(), this->sch->get_queued_jobs_size(), this->print_cluster_usage().c_str());
                LOG->record(4, JOB_FINISH, this->syscl->get_time(), this->sch->get_queued_jobs_size(), j->to_string().c_str());
                ++t_finished;
            }
            
        }
        
        delete finished_jobs;
    }
    this->syscl->next_clock();
    if(t_finished == t_total && this->sch->get_queued_jobs_size() == 0 && this->input_jobs.size() == 0) return 1;
    return 0;
}

void cluster::add_node(node * n){
    this->nodes.push_back(n);
}

void cluster::broadcast(int count, ...){
    std::cout<<"[INFO] [t=" << syscl->get_time() << "] ";
    va_list ap;
    va_start(ap, count);
    for(int i=0; i<count; i++){
        std::cout << va_arg(ap, char *);
    }
    va_end(ap);
    std::cout << std::endl;
}


void cluster::boot_all_nodes(){
    for(std::vector<node *>::iterator it = this->nodes.begin() ; it != this->nodes.end(); ++it){
        this->broadcast(2, "Attempting to boot ", (*it)->get_name());
        //LOG->record(3, NODE_FOUND, this->syscl->get_time(), (*it)->get_name());
        this->boot_node(*it);
    }
}

void cluster::shutdown_all_nodes(){
    for(std::vector<node *>::iterator it = this->nodes.begin() ; it != this->nodes.end(); ++it){
        this->broadcast(2, "Attempting to shutdown ", (*it)->get_name());
        this->shutdown_node(*it);
    }
}

void cluster::add_nodes_from_file(FILE * f){
    
    int read = 0;
    Machine_conf mc;
    
    while(!feof(f)){
        
        if(MACHINE_CONF_FIELDS != (read=fscanf(f, "%" PRIu64 "\t%s\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 , &mc.id, mc.name, &mc.n_cores, &mc.mem, &mc.penalty_boot, &mc.penalty_shutdown, &mc.cost_per_second)) && !feof(f)){
            this->broadcast(3, "Unknown node, read only  ", mc.name, " fields");
        }else if(read == MACHINE_CONF_FIELDS){
            node * n = new node(mc.id, mc.name, mc.n_cores, mc.mem, mc.penalty_boot, mc.penalty_shutdown, mc.cost_per_second);
            this->broadcast(2, "Detected node ", mc.name);
            this->add_node(n);
        }
    }    
}

bool cluster::boot_node(node * n){
    if(!n->get_state() && n->is_system_busy() == 0){
        n->turn_on(this->syscl->get_clock());
        this->nodes_online++;
        return true;
    }
    return false;
}

bool cluster::shutdown_node(node * n){
    if(n->get_state() && n->is_system_busy() == 0){
        n->turn_off(this->syscl->get_clock());
        this->nodes_online--;
        return true;
    }
    return false;
}

std::string cluster::print_cluster_usage(){

    std::string s = "nodes " + std::to_string(this->nodes_online) + "/" + std::to_string(this->nodes.size());

    double core_load = 0.0, mem_load = 0.0;
    uint64_t cost_per_sec = 0;
    for(std::vector<node *>::iterator it = this->nodes.begin() ; it != this->nodes.end(); ++it){
        if((*it)->get_state()){
            core_load += (*it)->get_node_CPU_load();
            mem_load += (*it)->get_node_MEM_load();
        }
        if((*it)->get_state() || (*it)->is_system_busy() > 0){
            cost_per_sec += (*it)->get_cost();
        }
    }

    s += " coreload " + std::to_string(core_load / this->nodes_online) + "% ";
    s += " memload " + std::to_string(mem_load / this->nodes_online) + "% "; 
    s += " cost/s " + std::to_string(cost_per_sec) + '\0';

    return s;
}


bool cluster::add_finished_core_and_check(job * j){ 
    this->table_of_jobs_completition[j->job_internal_identifier].n_cores_finished++;
    //printf("job %" PRIu64 " added up to %" PRIu64 " cores when req is %le\n", j->job_internal_identifier, this->table_of_jobs_completition[j->job_internal_identifier].n_cores_finished, j->CPU_requested);
    //getchar();
    return (this->table_of_jobs_completition[j->job_internal_identifier].n_cores_launched == this->table_of_jobs_completition[j->job_internal_identifier].n_cores_finished); 
}