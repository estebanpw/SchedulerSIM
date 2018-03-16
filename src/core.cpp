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
            
            // First, make sure the job did not crash or was not aborted
            if(j->real_exit_code == 0){
                // Then, check that it still has quantums
                if(j->remaining_quantums > 0){
                    // Before computing further, check that we did not reach walltime
                    if(j->wall_time_clocks > 0){
                        // Compute
                        j->compute();
                        this->jobs.push(j); // Append to end of queue
                        return NULL;
                    }else{
                        // Abort the job
                        j->real_exit_code = -1;
                        return j;
                    }

                }else{
                    // If no more quantums, it is completed
                    return j;
                }

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

