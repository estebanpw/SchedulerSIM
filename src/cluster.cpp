#include "cluster.h"

cluster::cluster(FILE * f_input_jobs, scheduler * sch){
    this->syscl = new sysclock();
    this->sch = sch;
    this->sch->set_nodes_list(this->get_nodes_list());
    this->nodes_online = 0;
    this->t_total = 0;
    this->t_finished = 0;
    this->t_aborted = 0;


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
    
    this->t_jobs = this->input_jobs.size();
    this->table_of_jobs_completition = (Jobs_completition *) std::calloc(this->input_jobs.size(), sizeof(Jobs_completition));
    if(this->table_of_jobs_completition == NULL) terror("Could not allocate table of jobs completition");


}

cluster::~cluster(){
    std::free(this->table_of_jobs_completition);
}

int cluster::compute(){
    // Perform actions
    
    // Check if there is a job waiting
    job * current_job = NULL;
    uint64_t curr_clock = this->syscl->get_clock();
    uint64_t curr_time = this->syscl->get_time();

    if(curr_clock % (QUANTUMS_IN_DAY) == 0 && curr_time > 0){
        LOG->record(7, SYS_USE, curr_time, this->sch->get_queued_jobs_size(), this->t_total, this->t_finished, this->t_aborted, this->print_cluster_usage().c_str());
        LOG->record(2, DISPLAY_DATE, curr_time);
        
        uint64_t counter = 0;
        for(uint64_t i=0; i<this->t_jobs; i++){
            if(this->table_of_jobs_completition[i].j != NULL && this->table_of_jobs_completition[i].n_cores_launched != this->table_of_jobs_completition[i].n_cores_finished){
                if(counter == 0){ LOG->record(2, QUEUE_STATUS, curr_time); }
                LOG->record(2, QUEUE_ROW, this->table_of_jobs_completition[i].j->to_string().c_str());
                ++counter;
            }
            if(counter == MAX_JOBS_TO_SHOW){ LOG->record(1, QUEUE_ETC); break; }
            
        }
        if(counter > 0) LOG->record(1, QUEUE_END);
    } 

    if(this->input_jobs.size() > 0){
        current_job = this->input_jobs.front();
        // If a job enters the system, submit it to the waiting queue
        while(current_job != NULL && current_job->get_submit_time() <= this->syscl->get_time() && this->input_jobs.size() > 0){
            // Schedule it insert to job queue
            //this->broadcast(2, "job enter ", current_job->to_string().c_str());
            this->sch->queue_job(current_job, curr_clock);
            this->t_total++;            
            LOG->record(4, JOB_ENTER, this->syscl->get_time(), this->sch->get_queued_jobs_size(), current_job->to_string().c_str());
            LOG->record(7, SYS_USE, this->syscl->get_time(), this->sch->get_queued_jobs_size(), this->t_total, this->t_finished, this->t_aborted, this->print_cluster_usage().c_str());
            this->insert_job_waiting_signal(current_job);
            current_job->real_submit_clocks = curr_clock;
            
            // Remove this job from input first
            this->input_jobs.erase(this->input_jobs.begin());

            current_job = this->input_jobs.front();
            
            
        }
    }
    // Only attempt to deploy jobs once every K second
    if(curr_clock % (LOGIN_NODE_INTERVAL*QUANTUMS_PER_SEC) == 0 && this->sch->get_queued_jobs_size() > 0){
        this->sch->deploy_jobs(curr_clock);
        this->sch->manage_nodes_state(curr_clock);
    }

    // Compute quantums
    for(std::vector<node *>::iterator it = this->nodes.begin() ; it != this->nodes.end(); ++it){
        // Power off/on nodes as requested 
        //if((*it)->can_I_use_it(curr_clock) && (*it)->get_state() == true && (*it)->how_the_scheduler_wants_it == false && (*it)->get_efficient_t_jobs() == 0){
        if(this->sch->get_policy()->turn_off_check(*it, curr_clock)){ 
            // Turn off 
            (*it)->turn_off(curr_clock); //
            this->nodes_online--;// std::cout << "TURN OFF NODE - " << (*it)->get_id() << "\n";
        }else if(!(*it)->can_I_use_it(curr_clock) && (*it)->get_state() == false && (*it)->how_the_scheduler_wants_it == true){
            (*it)->turn_on(curr_clock);
            this->nodes_online++;// std::cout << "TURN ON NODE - " << (*it)->get_id() << "\n";
        }

        // Actual computation
        std::queue<job *> * finished_jobs = (*it)->compute(curr_clock);
        
        while(finished_jobs->size() > 0){
            
            job * j = finished_jobs->front();
            finished_jobs->pop();

            JOBSTATE js = this->add_finished_core_and_check(j);

            if(js == JOB_DONE){
                (*it)->free_memory_from_process(j->MEM_requested);
                j->real_end_clocks = curr_clock;
                this->t_finished++;
                LOG->record(8, JOB_FINISH, curr_time, this->sch->get_queued_jobs_size(), j->real_submit_clocks, j->real_start_clocks, j->real_end_clocks,
                j->to_string().c_str(), seconds_to_date_char((j->real_end_clocks - j->real_start_clocks) / QUANTUMS_PER_SEC).c_str());
                LOG->record(7, SYS_USE, curr_time, this->sch->get_queued_jobs_size(), this->t_total, this->t_finished, this->t_aborted, this->print_cluster_usage().c_str());
                if((*it)->get_efficient_t_jobs() == 0 && this->sch->get_policy()->empty_queue_manager(&this->nodes, *it)){
                    this->sch->get_policy()->want_node_off((*it));// std::cout << "EMPTY - TURN OFF NODE - " << (*it)->get_id() << "\n";//(*it)->turn_off(curr_clock);this->nodes_online--; std::cout << "TURN OFF NODE - " << (*it)->get_id() << "\n"; //this->sch->get_policy()->want_node_off((*it));
                }
            }else if(js == JOB_ABORT){
                (*it)->free_memory_from_process(j->MEM_requested);
                j->real_end_clocks = curr_clock;
                this->t_aborted++;
                LOG->record(8, JOB_ABORTED, curr_time, this->sch->get_queued_jobs_size(), j->real_submit_clocks, j->real_start_clocks, j->real_end_clocks,
                j->to_string().c_str(), seconds_to_date_char((j->real_end_clocks - j->real_start_clocks) / QUANTUMS_PER_SEC).c_str());
                LOG->record(7, SYS_USE, curr_time, this->sch->get_queued_jobs_size(), this->t_total, this->t_finished, this->t_aborted, this->print_cluster_usage().c_str());
                if((*it)->get_efficient_t_jobs() == 0 && this->sch->get_policy()->empty_queue_manager(&this->nodes, *it)){
                    this->sch->get_policy()->want_node_off((*it));// std::cout << "EMPTY - TURN OFF NODE - " << (*it)->get_id() << "\n";//this->sch->get_policy()->want_node_off((*it));this->nodes_online--; std::cout << "TURN OFF NODE - " << (*it)->get_id() << "\n";
                }
            }
        }
        
        delete finished_jobs;
    }
    
    this->syscl->next_clock();
    if( (t_finished+t_aborted) == t_total && this->sch->get_queued_jobs_size() == 0 && this->input_jobs.size() == 0){
        LOG->record(7, SYS_USE, this->syscl->get_time(), this->sch->get_queued_jobs_size(), this->t_total, this->t_finished, this->t_aborted, this->print_cluster_usage().c_str());
        LOG->record(3, SHUTDOWN, this->syscl->get_time(), this->syscl->get_clock());
        return 1;
    }
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
        //this->broadcast(2, "Attempting to boot ", (*it)->get_name());
        LOG->record(4, NODE_ON, this->syscl->get_time(), "Attempting to boot ", (*it)->get_name());
        this->boot_node(*it);
    }
}

void cluster::shutdown_all_nodes(){
    for(std::vector<node *>::iterator it = this->nodes.begin() ; it != this->nodes.end(); ++it){
        //this->broadcast(2, "Attempting to shutdown ", (*it)->get_name());
        LOG->record(4, NODE_OFF, this->syscl->get_time(), "Attempting to shutdown ", (*it)->get_name());
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
    if(!n->get_state() && n->is_system_busy() <= this->syscl->get_clock()){
        n->turn_on(this->syscl->get_clock());
        this->nodes_online++;
        return true;
    }
    return false;
}

bool cluster::shutdown_node(node * n){
    if(n->get_state() && n->is_system_busy() <= this->syscl->get_clock()){
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
            core_load += (*it)->efficient_get_node_CPU_load();
            mem_load += (*it)->get_node_MEM_load();
        }
        if((*it)->get_state() || (*it)->is_system_busy() > this->syscl->get_clock()){
            cost_per_sec += (*it)->get_cost();
        }
    }

    s += " coreload " + std::to_string(core_load / this->nodes_online) + "% ";
    s += " memload " + std::to_string(mem_load / this->nodes_online) + "% "; 
    s += " cost/s " + std::to_string(cost_per_sec) + '\0';

    return s;
}


JOBSTATE cluster::add_finished_core_and_check(job * j){ 
    this->table_of_jobs_completition[j->job_internal_identifier].n_cores_finished++;


    if(this->table_of_jobs_completition[j->job_internal_identifier].n_cores_launched == this->table_of_jobs_completition[j->job_internal_identifier].n_cores_finished){
        if(j->real_exit_code == 0){
            return JOB_DONE;
        }else{
            return JOB_ABORT;
        }
    }
    
    return JOB_OK;
}