#pragma once

#include "structs.h"
#include "node.h"
#include "log-recorder.h"
#include "common-functions.h"
#include <pthread.h>
#include <vector>
#include <queue>
#include <algorithm>
#include <iostream>

class policy;

// Node comparison struct
/* struct load_on_node{
    node * at_node;
    load_on_node(node * n): at_node(n) {}
};
*/
// Abstract Policy
class policy
{
public:
    //policy(){ global_policy = this) };
    virtual bool compare_node_load(load_on_node * a, load_on_node * b) = 0;
    virtual void manage_node_state(node * n) = 0;
    void want_node_off(node * n) { n->how_the_scheduler_wants_it = false; }
    void want_node_on(node * n) { n->how_the_scheduler_wants_it = true; }

    //static bool compare_two_node_loads(load_on_node * a, load_on_node * b) { return global_policy->compare_node_load(a,b); }
};

// Basic Policy: All On
class policy_ALWAYS_ON : public policy
{
public:
    //policy_ALWAYS_ON();
    bool compare_node_load(load_on_node * a, load_on_node * b);
    void manage_node_state(node * n);
};

// Basic Policy: All On
class policy_ON_WHEN_BUSY : public policy
{
public:
    //policy_ON_WHEN_BUSY();
    bool compare_node_load(load_on_node * a, load_on_node * b);
    void manage_node_state(node * n);
};

// Needed to encapsulate the compare function
class class_comp_node_load
{
private:
    policy * current_policy = NULL;
public:
    class_comp_node_load(policy * p): current_policy(p) {}
    bool operator() (load_on_node * n1, load_on_node * n2) const { return (this->current_policy->compare_node_load(n1,n2)); }
};
