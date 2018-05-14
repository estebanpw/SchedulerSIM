#include "sysclock.h"

sysclock::sysclock()
{
	this->t = 0;
    this->time = 0;
}

void sysclock::next_clock(){
    this->t++;
    //if(this->t % 10000 == 0) printf("gone clocking %" PRIu64 " = %" PRIu64" (s)\n", this->t, this->time);
    if(this->t % QUANTUMS_PER_SEC == 0 && this->t > 0) this->time++;
}