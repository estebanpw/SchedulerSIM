#include <stdio.h>
#include <signal.h>
#include "io-functions.h"
#include "cluster.h"
#include "sysclock.h"
#include "common-functions.h"
#include "structs.h"

static volatile int keep_running = 1;

void signal_handler(int dummy) {
    keep_running = 0;
    fprintf(stdout, "Detected Ctrl+C signal. Exiting.\n");
}

void open_files(const char * m_conf, const char * workload, const char * log_out, const char * quantum_time, FILE ** f_m_conf, FILE ** f_workload, FILE ** f_log_out, uint64_t * quantum_time_cpu){
    char devnull[1024];
    *f_m_conf = fopen(m_conf, "rt");
    if(*f_m_conf == NULL) terror("Could not open machine configuration file");
    if(NULL == fgets(devnull, 1024, *f_m_conf)) terror("Could not read from machine configuration file"); // Read header

    *f_workload = fopen(workload, "rt");
    if(*f_workload == NULL) terror("Could not open workload file");
    if(NULL == fgets(devnull, 1024, *f_workload)) terror("Could not read from workload file"); // Read header

    *f_log_out = fopen(log_out, "wt");
    if(*f_log_out == NULL) terror("Could not open output log file");

    *quantum_time_cpu = (uint64_t) atoi(quantum_time);
}

int main(int argc, char ** av){

    if(argc != 5) terror("Error, please use: ./schedulerSIM machine-conf.csv workload.csv out.log quantum_time");

    FILE * f_machine_conf = NULL, * f_workload = NULL, * f_log_out = NULL;
    uint64_t quantum_time;
    open_files(av[1], av[2], av[3], av[4], &f_machine_conf, &f_workload, &f_log_out, &quantum_time);

    cluster * system_cluster = new cluster(quantum_time, f_workload);
    system_cluster->add_nodes_from_file(f_machine_conf);
    system_cluster->boot_all_nodes();

    signal(SIGINT, signal_handler);
    while(keep_running){
        system_cluster->compute();
    }

    

    system_cluster->shutdown_all_nodes();

    fclose(f_machine_conf);
    fclose(f_workload);
    fclose(f_log_out);
    
    return 0;
}