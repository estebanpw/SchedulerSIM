#pragma once

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QUANTUMS_PER_SEC 5
#define QUANTUMS_IN_DAY 60*60*24*QUANTUMS_PER_SEC
#define LOGIN_NODE_INTERVAL 5
#define QUANTUMS_IN_FRAME QUANTUMS_PER_SEC*60*5 // Frames will have 5 minute duration

#define MAX_THREADS_PER_CPU 6
#define MAX_JOBS_TO_SHOW 10

#define DATA_LEN 100
#define MACHINE_CONF_FIELDS 7

#define INIT_YEAR 2016
#define INIT_MONTH 10
#define INIT_DAY 1
#define INIT_HOUR 0
#define INIT_MINUTE 0
#define INIT_SECOND 0

#define SECOND  1
#define MINUTE  60
#define HOUR    60*60
#define DAY     60*60*24
#define WEEK    60*60*24*7
#define MONTH   60*60*24*7*4

#define READ_BUFFER 5000000
#define ARRAY_SIZE  100000
#define LINE_BUFFER 500

#define COLUMNS_SCALED_THROUGHPUT   2
#define JOB_FINISHED_LENGTH         14
#define SYS_LOAD_LENGTH             10
#define SYSTEM_OFF_LENGTH           12

enum JOBSTATE { JOB_OK, JOB_DONE, JOB_ABORT };
enum LOGTYPE { NODE_FOUND, NODE_ON, NODE_OFF, JOB_ENTER, JOB_START, JOB_FINISH, JOB_ABORTED, SYS_USE, SHUTDOWN, DISPLAY_DATE, QUEUE_STATUS, QUEUE_ROW, QUEUE_END, QUEUE_ETC };

extern bool MULTITHREADING;
extern bool BACKFILL;



typedef struct machine_conf{
    uint64_t id;
    char name[DATA_LEN];
    uint64_t n_cores;
    uint64_t mem;
    uint64_t penalty_boot;
    uint64_t penalty_shutdown;
    uint64_t cost_per_second;
} Machine_conf;

typedef struct picasso_row{
    uint64_t    job_id;
    double      CPU_min_use;
    double      CPU_average_use;
    double      CPU_max_use;
    double      CPU_requested;
    double      MEM_min_use;
    double      MEM_average_use;
    double      MEM_max_use;
    double      MEM_requested;
    uint64_t    run_time_seconds;
    uint64_t    wall_time_seconds;
    uint64_t    submit_time_seconds;
    uint64_t    start_time_seconds;
    uint64_t    end_time_seconds;
    char        account_name[DATA_LEN];
    char        job_name[DATA_LEN];
    int         exit_code;

} Picasso_row;

typedef struct uint64_t_array {
    uint64_t *array;
    uint64_t i;
    uint64_t number_of_reallocs;
} uint64_t_array;