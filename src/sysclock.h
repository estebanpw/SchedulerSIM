#pragma once

#include "structs.h"

class sysclock
{
private:
    uint64_t t;
    uint64_t time;

public:
    sysclock();
    void next_clock();
    uint64_t get_clock(){ return this->t; }
    uint64_t get_time(){ return this->time; }
};