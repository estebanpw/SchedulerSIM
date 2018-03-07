#include "log-recorder.h"

log_recorder::log_recorder(FILE * out){
    
    this->out_log = out;
}

void log_recorder::record(int count, ...){

    va_list ap;
    va_start(ap, count);

    switch(va_arg(ap, int)){
        case NODE_FOUND: {
            fprintf(this->out_log, "[NODE DETECTED] (t=%" PRIu64 ") I can see ",  va_arg(ap, uint64_t));
            for(int i=2; i<count; i++) fprintf(this->out_log, "%s", va_arg(ap, char *));
        }
        case NODE_ON: {
            fprintf(this->out_log, "[NODE ON] (t=%" PRIu64 ") ",  va_arg(ap, uint64_t));
            for(int i=2; i<count; i++) fprintf(this->out_log, "%s", va_arg(ap, char *));
        }
        break;
        case NODE_OFF: {
            fprintf(this->out_log, "[NODE OFF] (t=%" PRIu64 ") ",  va_arg(ap, uint64_t));
            for(int i=2; i<count; i++) fprintf(this->out_log, "%s", va_arg(ap, char *));
        }
        break;
        case JOB_ENTER: {
            fprintf(this->out_log, "[JOB QUEUED] (t=%" PRIu64 ")", va_arg(ap, uint64_t));
            fprintf(this->out_log, " (in-queue=%" PRIu64 ") ", va_arg(ap, uint64_t));
            for(int i=3; i<count; i++) fprintf(this->out_log, "%s", va_arg(ap, char *));
        }
        break;
        case JOB_START: {
            fprintf(this->out_log, "[JOB LAUNCHED] (t=%" PRIu64 ")", va_arg(ap, uint64_t));
            fprintf(this->out_log, " (in-queue=%" PRIu64 ") ", va_arg(ap, uint64_t));
            for(int i=3; i<count; i++) fprintf(this->out_log, "%s", va_arg(ap, char *));
        }
        break;
        case JOB_FINISH: {
            fprintf(this->out_log, "[JOB FINISHED] (t=%" PRIu64 ") ",  va_arg(ap, uint64_t));
            fprintf(this->out_log, " (in-queue=%" PRIu64 ") ", va_arg(ap, uint64_t));
            for(int i=3; i<count; i++) fprintf(this->out_log, "%s", va_arg(ap, char *));
        }
        break;
        case SYS_USE: {
            fprintf(this->out_log, "[SYS LOAD] (t=%" PRIu64 ") ",  va_arg(ap, uint64_t));
            fprintf(this->out_log, " (in-queue=%" PRIu64 ") ", va_arg(ap, uint64_t));
            for(int i=3; i<count; i++) fprintf(this->out_log, "%s", va_arg(ap, char *));
        }
        break;
        case SHUTDOWN: {
            fprintf(this->out_log, "[SYSTEM OFF] (t=%" PRIu64 ") ",  va_arg(ap, uint64_t));
            for(int i=2; i<count; i++) fprintf(this->out_log, "%s", va_arg(ap, char *));
        }
        break;
    }

    va_end(ap);
    fprintf(this->out_log, "\n");
    fflush(stdout);
}

