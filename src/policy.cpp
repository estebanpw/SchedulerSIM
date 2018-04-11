#include "policy.h"

// Basic Policy: All On

bool policy_always_on::compare_two_node_loads(load_on_node * a, load_on_node * b){
    return (a->at_node->efficient_get_node_CPU_load() > b->at_node->efficient_get_node_CPU_load());
}

void policy_always_on::manage_node_state(node * n){
    want_node_on(n);
}


// Basic Policy: All On
bool policy_off_if_not_busy::compare_two_node_loads(load_on_node * a, load_on_node * b){
    if 
    return (a->at_node->efficient_get_node_CPU_load() > b->at_node->efficient_get_node_CPU_load());
}

void policy_off_if_not_busy::manage_node_state(node * n){
    if((*n)->efficient_get_node_CPU_load == 0 && (*n)->get_node_MEM_load == 0){
        want_node_off(n);
    } else {
        want_node_on(n);
     }
}