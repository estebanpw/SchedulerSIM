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

bool policy_ALWAYS_ON::empty_queue_manager(){
    // Return always false (to maintain on)
    return false;
}

fptr policy_ALWAYS_ON::get_compare_func(){
    return policy_ALWAYS_ON::compare_node_load;
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
    if(!(n->is_system_busy() <= t) && n->efficient_get_node_CPU_load() == 0 && n->get_node_MEM_load() == 0){
        want_node_off(n);
    } else {
        want_node_on(n);
    }
}

bool policy_ON_WHEN_BUSY::empty_queue_manager(){
    // Return always true (to turn off when not busy)
    return true;
}

fptr policy_ON_WHEN_BUSY::get_compare_func(){
    return policy_ON_WHEN_BUSY::compare_node_load;
}