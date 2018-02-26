#pragma once

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QUANTUMS_PER_SEC 100
#define DATA_LEN 100
#define MACHINE_CONF_FIELDS 7

#define INIT_YEAR 2014
#define INIT_MONTH 2
#define INIT_DAY 13
#define INIT_HOUR 8
#define INIT_MINUTE 25
#define INIT_SECOND 10

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