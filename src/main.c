#include <stdio.h>
#include <signal.h>
#include "io-functions.h"
#include "cluster.h"
#include "sysclock.h"
#include "log.h"
#include "common-functions.h"
#include "structs.h"

static volatile int keep_running = 1;
log * LOG;
bool MULTITHREADING = false;
uint64_t n_threads = 4;

void signal_handler(int dummy) {
    keep_running = 0;
    fprintf(stdout, "    Detected Ctrl+C signal. Exiting.\n");
}

void open_files(const char * m_conf, const char * workload, const char * log_out, FILE ** f_m_conf, FILE ** f_workload, FILE ** f_log_out){
    char devnull[1024];
    *f_m_conf = fopen(m_conf, "rt");
    if(*f_m_conf == NULL) terror("Could not open machine configuration file");
    if(NULL == fgets(devnull, 1024, *f_m_conf)) terror("Could not read from machine configuration file"); // Read header

    *f_workload = fopen(workload, "rt");
    if(*f_workload == NULL) terror("Could not open workload file");
    if(NULL == fgets(devnull, 1024, *f_workload)) terror("Could not read from workload file"); // Read header

    if(strcmp(log_out, "stdout") != 0){
        *f_log_out = fopen(log_out, "wt");
        if(*f_log_out == NULL) terror("Could not open output log file");    
    }else{
        *f_log_out = stdout;
    }

}

int main(int argc, char ** av){

    if(argc != 5) terror("Error, please use: ./schedulerSIM machine-conf.csv workload.csv out.log threads");

    FILE * f_machine_conf = NULL, * f_workload = NULL, * f_log_out = NULL;
    open_files(av[1], av[2], av[3], &f_machine_conf, &f_workload, &f_log_out);
    LOG = new log(f_log_out);
    n_threads = (uint64_t) atoi(av[4]);

    scheduler_FIFO * sch_FIFO = new scheduler_FIFO();
    cluster * system_cluster = new cluster(f_workload, sch_FIFO);
    system_cluster->add_nodes_from_file(f_machine_conf);
    system_cluster->boot_all_nodes();

    signal(SIGINT, signal_handler);
    int done = 0;
    while(keep_running && done == 0){
        done = system_cluster->compute();
    }

    

    system_cluster->shutdown_all_nodes();
    delete LOG;

    fclose(f_machine_conf);
    fclose(f_workload);
    if(f_log_out != NULL) fclose(f_log_out);
    
    return 0;
}