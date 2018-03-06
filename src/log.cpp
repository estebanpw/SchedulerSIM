#include "log.h"

log::log(FILE * out){
    
    this->out_log = out;
}

void log::record(int count, ...){

    va_list ap;
    va_start(ap, count);

    switch(va_arg(ap, int)){
        case NODE_FOUND: {
            fprintf(this->out_log, "[NODE DETECTED] (t=%" PRIu64 ") I can see ",  va_arg(ap, uint64_t));
        }
        case NODE_ON: {
            fprintf(this->out_log, "[NODE ON] (t=%" PRIu64 ") ",  va_arg(ap, uint64_t));
        }
        break;
        case NODE_OFF: {
            fprintf(this->out_log, "[NODE OFF] (t=%" PRIu64 ") ",  va_arg(ap, uint64_t));
        }
        break;
        case JOB_ENTER: {
            fprintf(this->out_log, "[JOB QUEUED] (t=%" PRIu64 ") ",  va_arg(ap, uint64_t));
        }
        break;
        case JOB_START: {
            fprintf(this->out_log, "[JOB STARTED] (t=%" PRIu64 ") ",  va_arg(ap, uint64_t));
        }
        break;
        case JOB_FINISH: {
            fprintf(this->out_log, "[JOB FINISHED] (t=%" PRIu64 ") ",  va_arg(ap, uint64_t));
        }
        break;
        case SYS_USE: {
            fprintf(this->out_log, "[SYS LOAD] (t=%" PRIu64 ") ",  va_arg(ap, uint64_t));
        }
        break;
    }
    
    for(int i=1; i<count; i++){
        fprintf(this->out_log, "%s", va_arg(ap, char *));
    }
    va_end(ap);
    fprintf(this->out_log, "\n");
    fflush(stdout);
}

