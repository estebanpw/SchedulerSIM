#include "core.h"

core::core(){
    
}

void core::compute(uint64_t quantum, uint64_t time){
    job * j = NULL;
    if(this->jobs.size() > 0){
        j = this->jobs.front();
        this->jobs.pop();


        if(j != NULL){
            // Perform computation
            if(j->get_remaining_quantums() > quantum){
                j->compute(quantum);

                this->jobs.push(j); // Append to end of queue
            }else{
                delete j;
            }
        }  
    }
    
}

void core::insert_job(job * j){
    this->jobs.push(j);
}