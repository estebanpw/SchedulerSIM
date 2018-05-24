#include "policy.h"
/*
fptr policy::get_compare_func(){
    return policy_ALWAYS_ON::compare_node_load;
}
*/
// Basic Policy: All On

bool policy_ALWAYS_ON::compare_node_load(load_on_node * a, load_on_node * b){
    return (a->at_node->efficient_get_node_CPU_load() > b->at_node->efficient_get_node_CPU_load());
}

void policy_ALWAYS_ON::manage_node_state(node * n, uint64_t t){
    if(n->is_system_busy() <= t)
        want_node_on(n);
}

bool policy_ALWAYS_ON::empty_queue_manager(std::vector<node *> * nodes, node * current_n){
    // Return always false (to maintain on)
    return false;
}

fptr policy_ALWAYS_ON::get_compare_func(){
    return policy_ALWAYS_ON::compare_node_load;
}

bool policy_ALWAYS_ON::turn_off_check(node * n, uint64_t curr_clock){
    // Never turn off nodes
    return false;
}

// Basic Policy: All On
bool policy_ON_WHEN_BUSY::compare_node_load(load_on_node * a, load_on_node * b){
    if (a->at_node->efficient_get_node_CPU_load() == b->at_node->efficient_get_node_CPU_load()){
        return (a->at_node->get_free_memory() > b->at_node->get_free_memory());
    } else {
        return (a->at_node->efficient_get_node_CPU_load() > b->at_node->efficient_get_node_CPU_load());
    }
}

void policy_ON_WHEN_BUSY::manage_node_state(node * n, uint64_t t){
    if(!(n->is_system_busy() <= t) && n->efficient_get_node_CPU_load() == 0 && n->get_node_MEM_load() == 0 && n->get_efficient_t_jobs() == 0){
        want_node_off(n);
    } else {
        want_node_on(n);
    }
}

bool policy_ON_WHEN_BUSY::empty_queue_manager(std::vector<node *> * nodes, node * current_n){
    // Return always true (to turn off when not busy)
    return true;
}

fptr policy_ON_WHEN_BUSY::get_compare_func(){
    return policy_ON_WHEN_BUSY::compare_node_load;
}

bool policy_ON_WHEN_BUSY::turn_off_check(node * n, uint64_t curr_clock){
    return (n->can_I_use_it(curr_clock) && n->get_state() == true && n->how_the_scheduler_wants_it == false && n->get_efficient_t_jobs() == 0);
}

// Policy: Auto-Scaling (Static and Dynamic Nodes)

policy_AUTO_SCALING::policy_AUTO_SCALING(int n_static, const char * m_conf){    
    std::vector<load_on_node *> nodes;
    Machine_conf mc;

    int read = 0; char devnull[1024];
    FILE * f = fopen(m_conf, "rt");
    if(f == NULL) terror("Could not open machine configuration file");
    if(NULL == fgets(devnull, 1024, f)) terror("Could not read from machine configuration file");

    while(!feof(f)){
        read=fscanf(f, "%" PRIu64 "\t%s\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 , &mc.id, mc.name, &mc.n_cores, &mc.mem, &mc.penalty_boot, &mc.penalty_shutdown, &mc.cost_per_second);
        if(read == MACHINE_CONF_FIELDS){
            node * n = new node(mc.id, mc.name, mc.n_cores, mc.mem, mc.penalty_boot, mc.penalty_shutdown, mc.cost_per_second);
            nodes.push_back(new load_on_node(n));
        }
    }

    std::sort(nodes.begin(), nodes.end(), policy_AUTO_SCALING::compare_node_load);

    int counter = 0;
    for(std::vector<load_on_node *>::iterator it = nodes.begin() ; it != nodes.end(); ++it){
        if(counter < n_static){
            static_list.push_back((*it)->at_node->get_id()); counter++;
            //std::cout << "ADDING TO STATIC | NODE ID " << (*it)->at_node->get_id() << "\n";
        }
    }
}

int policy_AUTO_SCALING::is_node_static(node * n){
    int pos = std::find(this->static_list.begin(), this->static_list.end(),n->get_id()) - this->static_list.begin();
    //std::cout << "TEST IS_STATIC | NODE ID: " << n->get_id() << " | pos " << pos << "\n";
    if(pos >= static_cast<int>(this->static_list.size())) 
        return -1; 
    else
     return pos;
}

bool policy_AUTO_SCALING::compare_node_load(load_on_node * a, load_on_node * b){
    if (a->at_node->efficient_get_node_CPU_load() == b->at_node->efficient_get_node_CPU_load()){
        return (a->at_node->get_free_memory() > b->at_node->get_free_memory());
    } else {
        return (a->at_node->efficient_get_node_CPU_load() > b->at_node->efficient_get_node_CPU_load());
    }
}

void policy_AUTO_SCALING::manage_node_state(node * n, uint64_t t){
    //std::cout << "TEST CURRENT NODE | ID " << n->get_id() << "\n";
    if(is_node_static(n) == -1 && !(n->is_system_busy() <= t) && n->efficient_get_node_CPU_load() == 0 && n->get_node_MEM_load() == 0 && n->get_efficient_t_jobs() == 0){
        // If node_id is dynamic and not busy -> turn off
        want_node_off(n);
    } else {
        want_node_on(n);
    }
}

bool policy_AUTO_SCALING::empty_queue_manager(std::vector<node *> * nodes, node * current_n){
    // DOWNSCALING | If current empty node is Static...
    int static_pos = is_node_static(current_n);
    if(static_pos != -1){
        uint64_t most_working_node = 10000;
        uint64_t most_jobs_in_node = 0; 

        //std::cout << "TEST EMPTY QUEUE | NODE " << current_n->get_id() << "\n";
        // Get Dynamic Node ON with Highest CPU load (Will become Static)
        for(std::vector<node *>::iterator it = (*nodes).begin() ; it != (*nodes).end(); ++it){
            //(*it)->print_node();
            if((*it)!=current_n && (*it)->get_efficient_t_jobs() > most_jobs_in_node){
                most_jobs_in_node = (*it)->get_efficient_t_jobs();
                most_working_node = (*it)->get_id();      
                //std::cout << "ITERATE - EMPTY QUEUE | CURRENT NODE FOUND: " << most_working_node << " - JOB LOAD: " << most_jobs_in_node << "\n";
            }
        }
        //std::cout << "TEST EMPTY QUEUE | TOP NODE FOUND: " << most_working_node << " - JOB LOAD: " << most_jobs_in_node << "\n";
        // If found Dynamic, switch status and turn off
        if(most_jobs_in_node > 0){
            this->static_list.erase(this->static_list.begin() + static_pos);
            this->static_list.push_back(most_working_node);
            //std::cout << "SWAP STATIC NODE (ID:" << current_n->get_id() << " - JOB LOAD: " << current_n->get_efficient_t_jobs() << ") FOR DYNAMIC NODE (ID: " << most_working_node << " - JOB LOAD: " << most_jobs_in_node << ")\n";
            return true;
        }
        else // Keep on
            return false;
    } else
        return false;
}

fptr policy_AUTO_SCALING::get_compare_func(){
    return policy_AUTO_SCALING::compare_node_load;
}

bool policy_AUTO_SCALING::turn_off_check(node * n, uint64_t curr_clock){
    return (this->is_node_static(n) == -1 && n->get_state() == true && n->how_the_scheduler_wants_it == false && n->get_efficient_t_jobs() == 0);
}
