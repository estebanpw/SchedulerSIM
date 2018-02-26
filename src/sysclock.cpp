#include "sysclock.h"

sysclock::sysclock()
{
	this->t = 0;
    this->time = 0;
}

void sysclock::next_clock(){
    this->t++;
    if(this->t % QUANTUMS_PER_SEC) this->time++;
}