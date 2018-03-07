#include "io-functions.h"
#include "common-functions.h"


uint64_t normalize_date(const char * d1, struct tm * zero){

    char aux[DATA_LEN];
    struct tm newdate;

    strncpy(&aux[0], &d1[0], 4); aux[4] = '\0'; // Copy year
    newdate.tm_year = atoi(aux);

    strncpy(&aux[0], &d1[6], 2); aux[2] = '\0'; // Copy month
    newdate.tm_mon = atoi(aux);

    strncpy(&aux[0], &d1[8], 2); aux[2] = '\0'; // Copy day
    newdate.tm_mday = atoi(aux);

    strncpy(&aux[0], &d1[11], 2); aux[2] = '\0'; // Copy hour
    newdate.tm_hour = atoi(aux);

    strncpy(&aux[0], &d1[14], 2); aux[2] = '\0'; // Copy minute
    newdate.tm_min = atoi(aux);
    
    strncpy(&aux[0], &d1[17], 2); aux[2] = '\0'; // Copy second
    newdate.tm_sec = atoi(aux);

    return (uint64_t) difftime(mktime(&newdate), mktime(zero));
}

uint64_t walltime_to_seconds(char * wtime){
    // 7-00:00:00
    // e.g. last character is strlen(wtime) - 1
    uint64_t acum = 0;
    char aux[16];
    aux[0] = '\0';

    // Seconds
    strncpy(&aux[0], &wtime[strlen(wtime) - 2], 2);
    aux[3] = '\0';
    acum += (uint64_t) atoi(aux);

    // Minutes
    strncpy(&aux[0], &wtime[strlen(wtime) - 5], 2);
    aux[3] = '\0';
    acum += 60 * (uint64_t) atoi(aux);

    // Hours
    strncpy(&aux[0], &wtime[strlen(wtime) - 8], 2);
    aux[3] = '\0';
    acum += 3600 * (uint64_t) atoi(aux);

    char * pos = strchr(wtime,'-');
    //printf("%s\n", wtime);
    if(pos != NULL){
        // There is a day also attached
        // Days
        
        strncpy(&aux[0], &wtime[0], pos-wtime);
        aux[pos-wtime] = '\0';
        acum += 24 * 3600 * (uint64_t) atoi(aux);
    }
    
    //printf("converted %s to %" PRIu64 "\n", wtime, acum);
    //getchar();


    return acum;
}

int picasso_read_row(FILE * f_in, Picasso_row * pr){
    char runtime_aux[DATA_LEN], maxtime_aux[DATA_LEN], submittime_aux[DATA_LEN], starttime_aux[DATA_LEN], endtime_aux[DATA_LEN];
    int read = 0;
    struct tm zero;
    zero.tm_sec = INIT_SECOND; zero.tm_min = INIT_MINUTE; zero.tm_hour = INIT_HOUR;
    zero.tm_mon = INIT_MONTH; zero.tm_year = INIT_YEAR; zero.tm_mday = INIT_DAY;

    read = fscanf(f_in, "%" PRIu64 "\t%le\t%le\t%le\t%le\t%le\t%le\t%le\t%le\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d:0", 
    &pr->job_id, &pr->CPU_min_use, &pr->CPU_average_use, &pr->CPU_max_use, &pr->CPU_requested,
    &pr->MEM_min_use, &pr->MEM_average_use, &pr->MEM_max_use, &pr->MEM_requested,
    runtime_aux, maxtime_aux, submittime_aux, starttime_aux, endtime_aux,
    pr->account_name, pr->job_name, &pr->exit_code);

    if(17 != read && !feof(f_in)) return -1;


    
    pr->wall_time_seconds = walltime_to_seconds(maxtime_aux);

    pr->submit_time_seconds = normalize_date(submittime_aux, &zero);
    pr->start_time_seconds = normalize_date(starttime_aux, &zero);
    pr->end_time_seconds = normalize_date(endtime_aux, &zero);
    pr->run_time_seconds = pr->end_time_seconds - pr->start_time_seconds;

    return read;
}