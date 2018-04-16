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

typedef bool (*fptr)(load_on_node *, load_on_node *);

// Abstract Policy
class policy
{
public:
    //policy(){ global_policy = this) };
    //virtual bool compare_node_load(load_on_node * a, load_on_node * b);
    virtual void manage_node_state(node * n) = 0;
    fptr get_compare_func();
    void want_node_off(node * n) { n->how_the_scheduler_wants_it = false; }
    void want_node_on(node * n) { n->how_the_scheduler_wants_it = true; }

    //static bool compare_two_node_loads(load_on_node * a, load_on_node * b) { return global_policy->compare_node_load(a,b); }
};

// Basic Policy: All On
class policy_ALWAYS_ON : public policy
{
public:
    // policy_ALWAYS_ON():
    static bool compare_node_load(load_on_node * a, load_on_node * b);
    void manage_node_state(node * n);
};

// Basic Policy: All On
class policy_ON_WHEN_BUSY : public policy
{
public:
    //policy_ON_WHEN_BUSY();
    static bool compare_node_load(load_on_node * a, load_on_node * b);
    void manage_node_state(node * n);
};