#include "log-recorder.h"

log_recorder::log_recorder(FILE * out){
    
    this->out_log = out;
}

void log_recorder::record(int count, ...){

    va_list ap;
    va_start(ap, count);

    switch(va_arg(ap, int)){
        case NODE_FOUND: {
            fprintf(this->out_log, "[NODE DETECTED] (t=$%" PRIu64 "$) I can see ",  va_arg(ap, uint64_t));
            for(int i=2; i<count; i++) fprintf(this->out_log, "%s ", va_arg(ap, char *));
        }
        case NODE_ON: {
            fprintf(this->out_log, "[NODE ON] (t=$%" PRIu64 "$) ",  va_arg(ap, uint64_t));
            for(int i=2; i<count; i++) fprintf(this->out_log, "%s ", va_arg(ap, char *));
        }
        break;
        case NODE_OFF: {
            fprintf(this->out_log, "[NODE OFF] (t=$%" PRIu64 "$) ",  va_arg(ap, uint64_t));
            for(int i=2; i<count; i++) fprintf(this->out_log, "%s ", va_arg(ap, char *));
        }
        break;
        case JOB_ENTER: {
            fprintf(this->out_log, "[JOB QUEUED] (t=$%" PRIu64 "$)", va_arg(ap, uint64_t));
            fprintf(this->out_log, " (in-queue=$%" PRIu64 "$) ", va_arg(ap, uint64_t));
            for(int i=3; i<count; i++) fprintf(this->out_log, "%s ", va_arg(ap, char *));
        }
        break;
        case JOB_START: {
            fprintf(this->out_log, "[JOB LAUNCHED] (t=$%" PRIu64 "$)", va_arg(ap, uint64_t));
            fprintf(this->out_log, " (in-queue=$%" PRIu64 "$) ", va_arg(ap, uint64_t));
            for(int i=3; i<count; i++) fprintf(this->out_log, "%s ", va_arg(ap, char *));
        }
        break;
        case JOB_FINISH: {
            fprintf(this->out_log, "[JOB FINISHED] (t=$%" PRIu64 "$) ",  va_arg(ap, uint64_t));
            fprintf(this->out_log, " (in-queue=$%" PRIu64 "$) ", va_arg(ap, uint64_t));
            fprintf(this->out_log, " (CL-submi=$%" PRIu64 "$) ", va_arg(ap, uint64_t));
            fprintf(this->out_log, " (CL-start=$%" PRIu64 "$) ", va_arg(ap, uint64_t));
            fprintf(this->out_log, " (CL-end  =$%" PRIu64 "$) ", va_arg(ap, uint64_t));

            for(int i=6; i<count; i++) fprintf(this->out_log, "%s ", va_arg(ap, char *));
        }
        break;
        case JOB_ABORTED: {
            fprintf(this->out_log, "[JOB ABORTED] (t=$%" PRIu64 "$) ",  va_arg(ap, uint64_t));
            fprintf(this->out_log, " (in-queue=$%" PRIu64 "$) ", va_arg(ap, uint64_t));
            fprintf(this->out_log, " (CL-submi=$%" PRIu64 "$) ", va_arg(ap, uint64_t));
            fprintf(this->out_log, " (CL-start=$%" PRIu64 "$) ", va_arg(ap, uint64_t));
            fprintf(this->out_log, " (CL-end  =$%" PRIu64 "$) ", va_arg(ap, uint64_t));

            for(int i=6; i<count; i++) fprintf(this->out_log, "%s ", va_arg(ap, char *));
        }
        break;
        case SYS_USE: {
            fprintf(this->out_log, "[SYS LOAD] (t=$%" PRIu64 "$) ",  va_arg(ap, uint64_t));
            fprintf(this->out_log, " (in-queue=$%" PRIu64 "$) ", va_arg(ap, uint64_t));
            fprintf(this->out_log, " (t-launch=$%" PRIu64 "$) ", va_arg(ap, uint64_t));
            fprintf(this->out_log, " (t-finish=$%" PRIu64 "$) ", va_arg(ap, uint64_t));
            fprintf(this->out_log, " (t-abort =$%" PRIu64 "$) ", va_arg(ap, uint64_t));
            for(int i=6; i<count; i++) fprintf(this->out_log, "%s ", va_arg(ap, char *));
        }
        break;
        case SHUTDOWN: {
            uint64_t t = va_arg(ap, uint64_t);
            fprintf(this->out_log, "[SYSTEM OFF] (t=$%" PRIu64 "$) ", t);
            fprintf(this->out_log, "(CL=$%" PRIu64 "$) ", va_arg(ap, uint64_t));
            std::string s = seconds_to_date_char(t);
            fprintf(this->out_log, "System was online for %s ", s.c_str());
        }
        break;
        case DISPLAY_DATE: {
            uint64_t t = va_arg(ap, uint64_t);
            fprintf(this->out_log, "[DATE] (t=$%" PRIu64 "$) ", t);
            std::string s = seconds_to_date_char(t);
            fprintf(this->out_log, "System was online for %s ", s.c_str());
        }
        break;
        case QUEUE_STATUS: {
            fprintf(this->out_log, "[JOBs STATUS] (t=%" PRIu64 ") ----------------",  va_arg(ap, uint64_t));
        }
        break;
        case QUEUE_ROW: {
            fprintf(this->out_log, "| %s", va_arg(ap, char *));
        }
        break;
        case QUEUE_END: {
            fprintf(this->out_log, "|---------------------------------------------\n");
        }
        break;
        case QUEUE_ETC: {
            fprintf(this->out_log, "|                    .....                    \n");
        }
        break;
    }

    va_end(ap);
    fprintf(this->out_log, "\n");
    fflush(stdout);
}

