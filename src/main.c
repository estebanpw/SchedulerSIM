#include <stdio.h>
#include <signal.h>
#include "io-functions.h"
#include "cluster.h"
#include "sysclock.h"
#include "log-recorder.h"
#include "common-functions.h"
#include "structs.h"

static volatile int keep_running = 1;
log_recorder * LOG;

bool MULTITHREADING = false;
bool BACKFILL = false;


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

    if(argc < 7) terror("Error, use: ./schedulerSIM <machine-conf.csv> <workload.csv> <out.log> <backfill=TRUE/FALSE> <multithreading=TRUE/FALSE> <scheduler-type>");

    FILE * f_machine_conf = NULL, * f_workload = NULL, * f_log_out = NULL;
    open_files(av[1], av[2], av[3], &f_machine_conf, &f_workload, &f_log_out);
    LOG = new log_recorder(f_log_out);
    if(strcmp(av[4], "TRUE") == 0) BACKFILL = true;
    if(strcmp(av[5], "TRUE") == 0) MULTITHREADING = true;

    scheduler * sch = NULL;

    if(strcmp(av[6], "FIFO") == 0) sch = new scheduler_FIFO();
    if(strcmp(av[6], "SHORT") == 0) sch = new scheduler_SHORT();
    if(strcmp(av[6], "PRIO") == 0){
        if(argc != 12) terror("   # For PRIOrity type use: ./schedulerSIM <machine-conf.csv> <workload.csv> <out.log> <backfill=TRUE/FALSE> <multithreading=TRUE/FALSE> <scheduler-type> <w> <q> <e> <c> <m>");
        sch = new scheduler_PRIORITY(atof(av[7]), atof(av[8]), atof(av[9]), atof(av[10]), atof(av[11])); // w q e c m
    }
    

    cluster * system_cluster = new cluster(f_workload, sch);
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