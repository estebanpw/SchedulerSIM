#include "cluster.h"

cluster::cluster(uint64_t quantum, FILE * f_input_jobs){
    this->quantum = quantum;
    this->syscl = new sysclock();


    Picasso_row pr;
    this->broadcast(1, "Receiving input job list");
    
    while(!feof(f_input_jobs)){
        if(0 < picasso_read_row(f_input_jobs, &pr)){
            
            job * j = new job(pr.job_id, pr.CPU_min_use, pr.CPU_average_use, pr.CPU_max_use, pr.CPU_requested, 
            pr.MEM_min_use, pr.MEM_average_use, pr.MEM_max_use, pr.MEM_requested, pr.run_time_seconds, pr.wall_time_seconds,
            pr.submit_time_seconds, pr.start_time_seconds, pr.end_time_seconds, pr.account_name, pr.job_name, pr.exit_code);
            this->input_jobs.push_back(j);
        }
    }
}

void cluster::compute(){
    // Perform actions

    for(std::vector<node *>::iterator it = this->nodes.begin() ; it != this->nodes.end(); ++it){
        (*it)->compute(this->quantum, this->syscl->get_time());
    }
    this->syscl->next_clock();
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
        
        if(MACHINE_CONF_FIELDS != (read=fscanf(f, "%"PRIu64"\t%s\t%"PRIu64"\t%"PRIu64"\t%"PRIu64"\t%"PRIu64"\t%"PRIu64, &mc.id, mc.name, &mc.n_cores, &mc.mem, &mc.penalty_boot, &mc.penalty_shutdown, &mc.cost_per_second)) && !feof(f)){
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
        n->turn_on();
        return true;
    }
    return false;
}

bool cluster::shutdown_node(node * n){
    if(n->get_state() && n->is_system_busy() == 0){
        n->turn_off();
        return true;
    }
    return false;
}