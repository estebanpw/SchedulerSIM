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
struct load_on_node{
    node * at_node;
    load_on_node(node * n): at_node(n) {}
};

// Abstract Policy
class policy
{
public:
    virtual bool compare_node_load(load_on_node * a, load_on_node * b) = 0;
    virtual void manage_node_state(node * n) = 0;
    void want_node_off(node * n) { (*n)->how_the_scheduler_wants_it = false; }
    void want_node_on(node * n) { (*n)->how_the_scheduler_wants_it = true; }
}

// Basic Policy: All On
class policy_always_on : public policy
{
public:
    bool compare_node_load(load_on_node * a, load_on_node * b);
    void manage_node_state(node * n);
}

// Basic Policy: All On
class policy_off_if_not_busy : public policy
{
public:
    bool compare_node_load(load_on_node * a, load_on_node * b);
    void manage_node_state(node * n)
}