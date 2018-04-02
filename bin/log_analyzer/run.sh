#!/bin/bash

# Version of metrics.R script. This generates temporal files with a C program and then metrics_plot.R generates a pdf with the plots

input_file=$1
output_file=$2
time_unit=$3
./bin/log_analyzer -in $input_file -out ___temp -t $time_unit
Rscript src/metrics_plot.R ___temp_m_cost ___temp_m_queued ___temp_m_launched ___temp_m_finished ___temp_m_aborted ___temp_m_job_finish_times ___temp_m_cpu_usage ___temp_m_mem_usage ___temp_m_nodes_usage ___temp_m_general_info ___temp_m_queued_jobs ___temp_m_diff_submit_start_jobs $output_file.pdf
rm ___temp*
