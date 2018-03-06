#include "core.h"

core::core(){
    this->current_load = 0;
}

job * core::compute(uint64_t t){
    job * j = NULL;
    if(this->jobs.size() > 0){
        j = this->jobs.front();
        this->jobs.pop();


        if(j != NULL){
            // Perform computation
            if(j->get_remaining_quantums() > 0){
                /*
                if(j->get_remaining_quantums() % QUANTUMS_IN_DAY == 0){
                    printf("Job %" PRIu64 " has %" PRIu64" quantums remaining which are %" PRIu64 " days\n", j->job_internal_identifier, j->get_remaining_quantums(), j->get_remaining_quantums() / (uint64_t) QUANTUMS_IN_DAY);
                    getchar();
                }
                */
                j->compute();

                this->jobs.push(j); // Append to end of queue
            }else{
                return j;
            }
        }  
    }
    return NULL;
}

void core::insert_job(job * j){
    this->jobs.push(j);
}

