#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "structs.h"
#include "common-functions.h"

FILE* fopen64(const char *filename, const char *type);
uint64_t get_time_unit(char time_unit_symbol);
void init_args(int argc, char **argv, FILE **input, char *output_namefile, char *time_unit_symbol);
void scaled_throughput(uint64_t_array job_finish_times, uint64_t makespan, uint64_t time_unit, char *namefile);
void scaled_stat_per_time(uint64_t_array stat_object, uint64_t_array time_object, uint64_t makespan, uint64_t time_unit, char *namefile);
void fill_scaled_stat_per_time(uint64_t_array stat_object, uint64_t_array time_object, uint64_t makespan, uint64_t time_unit, char *namefile);

int main(int argc, char ** argv){
	FILE *input_file = NULL;
	FILE *output = NULL;

	char time_unit_symbol;
	char *line_buffer = NULL;
	char *header_buffer = NULL;
	char *job_launched_aux_string;
	char *output_namefile;
	char *aux;

	uint64_t time_unit 	= 1;
	uint64_t i 			= 0;
	uint64_t maxnodes 	= 0;
	uint64_t submit 	= 0;
	uint64_t start 		= 0;
	uint64_t makespan 	= 0;
	uint64_t_array job_finish_times;
	uint64_t_array launched_jobs;
	uint64_t_array queued_jobs;
	uint64_t_array finished_jobs;
	uint64_t_array aborted_jobs;
	uint64_t_array jobs_times;
	uint64_t_array cost_per_second;
	// In the original R version, cpu_usage and mem_usage values were stored as floating point values. 
	// Here they are considered uint64_t
	uint64_t_array cpu_usage;
	uint64_t_array mem_usage;
	uint64_t_array nodes_usage;
	uint64_t_array cost_per_minute;
	double_array diff_submit_start_jobs;

	job_finish_times.array 			= (uint64_t*) malloc(ARRAY_SIZE * sizeof(uint64_t));
	launched_jobs.array 			= (uint64_t*) malloc(ARRAY_SIZE * sizeof(uint64_t));
	queued_jobs.array 				= (uint64_t*) malloc(ARRAY_SIZE * sizeof(uint64_t));
	finished_jobs.array 			= (uint64_t*) malloc(ARRAY_SIZE * sizeof(uint64_t));
	aborted_jobs.array 				= (uint64_t*) malloc(ARRAY_SIZE * sizeof(uint64_t));
	jobs_times.array 				= (uint64_t*) malloc(ARRAY_SIZE * sizeof(uint64_t));
	cost_per_second.array 			= (uint64_t*) malloc(ARRAY_SIZE * sizeof(uint64_t));
	cpu_usage.array 				= (uint64_t*) malloc(ARRAY_SIZE * sizeof(uint64_t));
	mem_usage.array 				= (uint64_t*) malloc(ARRAY_SIZE * sizeof(uint64_t));
	nodes_usage.array 				= (uint64_t*) malloc(ARRAY_SIZE * sizeof(uint64_t));
	daily_time.array				= (uint64_t*) malloc(ARRAY_SIZE * sizeof(uint64_t));
	cost_per_minute.array			= (uint64_t*) malloc(ARRAY_SIZE * sizeof(uint64_t));
	diff_submit_start_jobs.array 	= (double*) malloc(ARRAY_SIZE * sizeof(double));

	job_finish_times.i = 0;
	launched_jobs.i = 0;
	queued_jobs.i = 0;
	finished_jobs.i = 0;
	aborted_jobs.i = 0;
	jobs_times.i = 0;
	cost_per_second.i = 0;
	cpu_usage.i = 0;
	mem_usage.i = 0;
	nodes_usage.i = 0;
	diff_submit_start_jobs.i = 0;
	daily_time.i = 0;
	cost_per_minute.i = 0;

	job_finish_times.number_of_reallocs = 1;
	diff_submit_start_jobs.number_of_reallocs = 1;
	jobs_times.number_of_reallocs = 1;

	if (job_finish_times.array == NULL) {
		terror("Could not allocate memory for job_finish_times array");
	}
	if (launched_jobs.array == NULL) {
		terror("Could not allocate memory for launched_jobs array");
	}
	if (finished_jobs.array == NULL) {
		terror("Could not allocate memory for finished_jobs array");
	}
	if (aborted_jobs.array == NULL) {
		terror("Could not allocate memory for aborted_jobs array");
	}
	if (jobs_times.array == NULL) {
		terror("Could not allocate memory for jobs_times array");
	}
	if (cost_per_second.array == NULL) {
		terror("Could not allocate memory for cost_per_second array");
	}
	if (cpu_usage.array == NULL) {
		terror("Could not allocate memory for cpu_usage array");
	}
	if (mem_usage.array == NULL) {
		terror("Could not allocate memory for mem_usage array");
	}
	if (nodes_usage.array == NULL) {
		terror("Could not allocate memory for nodes_usage array");
	}
	if (daily_time.array == NULL) {
		terror("Could not allocate memory for daily_time array");
	}
	if (cost_per_minute.array == NULL) {
		terror("Could not allocate memory for cost_per_minute array");
	}
	if (diff_submit_start_jobs.array == NULL) {
		terror("Could not allocate memory for diff_submit_start_jobs array");
	}
	if ((line_buffer = (char*) calloc(LINE_BUFFER, sizeof(char))) == NULL) {
		terror("Could not allocate memory for line buffer");
	}
	if ((header_buffer = (char*) calloc(LINE_BUFFER, sizeof(char))) == NULL) {
		terror("Could not allocate memory for header buffer");
	}
	if ((job_launched_aux_string = (char*) calloc(LINE_BUFFER, sizeof(char))) == NULL) {
		terror("Could not allocate memory for job_launched_aux_string");
	}
	if ((output_namefile = (char*) calloc(LINE_BUFFER, sizeof(char))) == NULL) {
		terror("Could not allocate memory for output_namefile");
	}
	if ((aux = (char*) calloc(LINE_BUFFER, sizeof(char))) == NULL) {
		terror("Could not allocate memory for output_namefile");
	}

	init_args(argc, argv, &input_file, output_namefile, &time_unit_symbol);

	time_unit = get_time_unit(time_unit_symbol);

	while (fgets(line_buffer, LINE_BUFFER, input_file)) {
		strncpy(header_buffer, line_buffer, JOB_FINISHED_LENGTH);
		if (!strcmp(header_buffer, "[JOB FINISHED]")) {
			if (job_finish_times.number_of_reallocs * ARRAY_SIZE == job_finish_times.i) {
				job_finish_times.number_of_reallocs++;
				job_finish_times.array = (uint64_t*) realloc(job_finish_times.array, 
					job_finish_times.number_of_reallocs * ARRAY_SIZE * sizeof(uint64_t));
				if (job_finish_times.array == NULL) terror("Could not reallocate job_finish_times array");
			}
			sscanf(line_buffer, "%*[^$]$%" PRIu64 "$%*[^\n]\n", &job_finish_times.array[job_finish_times.i++]);
		} else if (!strcmp(header_buffer, "[JOB LAUNCHED]")) {
			if (diff_submit_start_jobs.number_of_reallocs * ARRAY_SIZE == diff_submit_start_jobs.i) {
				diff_submit_start_jobs.number_of_reallocs++;
				diff_submit_start_jobs.array = (double*) realloc(diff_submit_start_jobs.array, 
					diff_submit_start_jobs.number_of_reallocs * ARRAY_SIZE * sizeof(double));
				if (diff_submit_start_jobs.array == NULL) terror("Could not reallocate diff_submit_start_jobs array");
			}
			job_launched_aux_string = strstr(line_buffer, "SUBMIT");
			sscanf(job_launched_aux_string, "%*[^:]:%" PRIu64 "%*[^:]:%" PRIu64 "%*[^\n]\n", &submit, &start);
			diff_submit_start_jobs.array[diff_submit_start_jobs.i++] = (double)(start-submit)/60;
		} else {
			header_buffer[SYSTEM_OFF_LENGTH] = '\0';
			if (!strcmp(header_buffer, "[SYSTEM OFF]")) {
				sscanf(line_buffer, "%*[^$]$%" PRIu64 "$%*[^\n]\n", &makespan);
			} else {
				header_buffer[SYS_LOAD_LENGTH] = '\0';
				if (!strcmp(header_buffer, "[SYS LOAD]")){
					if (jobs_times.number_of_reallocs * ARRAY_SIZE == jobs_times.i) {
					jobs_times.number_of_reallocs++;
					jobs_times.array = (uint64_t*) realloc(jobs_times.array, jobs_times.number_of_reallocs * ARRAY_SIZE * sizeof(uint64_t));
					if (jobs_times.array == NULL) terror("Could not reallocate jobs_times array");
					queued_jobs.array = (uint64_t*) realloc(queued_jobs.array, jobs_times.number_of_reallocs * ARRAY_SIZE * sizeof(uint64_t));
					if (queued_jobs.array == NULL) terror("Could not reallocate queued_jobs array");
					launched_jobs.array= (uint64_t*) realloc(launched_jobs.array, jobs_times.number_of_reallocs * ARRAY_SIZE * sizeof(uint64_t));
					if (launched_jobs.array == NULL) terror("Could not reallocate launched_jobs array");
					finished_jobs.array= (uint64_t*) realloc(finished_jobs.array, jobs_times.number_of_reallocs * ARRAY_SIZE * sizeof(uint64_t));
					if (finished_jobs.array == NULL) terror("Could not reallocate finished_jobs array");
					aborted_jobs.array = (uint64_t*) realloc(aborted_jobs.array, jobs_times.number_of_reallocs * ARRAY_SIZE * sizeof(uint64_t));
					if (aborted_jobs.array == NULL) terror("Could not reallocate aborted_jobs array");
					nodes_usage.array = (uint64_t*) realloc(nodes_usage.array, jobs_times.number_of_reallocs * ARRAY_SIZE * sizeof(uint64_t));
					if (nodes_usage.array == NULL) terror("Could not reallocate nodes_usage array");
					cpu_usage.array = (uint64_t*) realloc(cpu_usage.array, jobs_times.number_of_reallocs * ARRAY_SIZE * sizeof(uint64_t));
					if (cpu_usage.array == NULL) terror("Could not reallocate cpu_usage array");
					mem_usage.array = (uint64_t*) realloc(mem_usage.array, jobs_times.number_of_reallocs * ARRAY_SIZE * sizeof(uint64_t));
					if (mem_usage.array == NULL) terror("Could not reallocate mem_usage array");
					cost_per_second.array = (uint64_t*) realloc(cost_per_second.array, jobs_times.number_of_reallocs * ARRAY_SIZE * sizeof(uint64_t));
					if (cost_per_second.array == NULL) terror("Could not reallocate cost_per_second array");
				
					sscanf(line_buffer, "%*[^$]$%" PRIu64 "$%*[^$]$%" PRIu64 "$%*[^$]$%" PRIu64 "$%*[^$]$%" PRIu64 
						"$%*[^$]$%"	PRIu64 "$%*[^s]s %" PRIu64 "/%" PRIu64 "%*[^d]d %" PRIu64 ".%*[^d]d %" PRIu64 
						".%*[^/]/s %" PRIu64 "\n", 
						&jobs_times.array[jobs_times.i++], &queued_jobs.array[queued_jobs.i++], 
						&launched_jobs.array[launched_jobs.i++], &finished_jobs.array[finished_jobs.i++],
						&aborted_jobs.array[aborted_jobs.i++], &nodes_usage.array[nodes_usage.i++],
						&maxnodes, &cpu_usage.array[cpu_usage.i++], &mem_usage.array[mem_usage.i++],
						&cost_per_second.array[cost_per_second.i++]);
					}
				} else {
					header_buffer[DISPLAY_COST_LENGTH] = '\0';
					if(!strcmp(header_buffer, "[COST]")){
						if (cost_per_minute.number_of_reallocs * ARRAY_SIZE == cost_per_minute.i) {
							cost_per_minute.number_of_reallocs++;
							cost_per_minute.array = (uint64_t*) realloc(cost_per_minute.array, 
								ost_per_minute.number_of_reallocs * cost_per_minute * sizeof(uint64_t));
							if (cost_per_minute.array == NULL) terror("Could not reallocate cost_per_minute array");
							daily_time.array = (uint64_t*) realloc(daily_time.array, 
								daily_time.number_of_reallocs * daily_time * sizeof(uint64_t));
							if (daily_time.array == NULL) terror("Could not reallocate daily_time array");
						}

						sscanf(line_buffer, "%*[^$]$%" PRIu64 "$%*[^$]$%" PRIu64 "$\n",
							&daily_time.array[daily_time.i++], &cost_per_minute.array[cost_per_minute.i++]);
					}
				}
			}
		}
	}

	output = fopen64(strcat(strcpy(aux, output_namefile), "_m_general_info"), "wt");
	if(output==NULL) terror("Could not open output file");
	fprintf(output, "%" PRIu64 ",", makespan);
	fprintf(output, "%" PRIu64 ",", time_unit);
	fprintf(output, "%c,", time_unit_symbol);
	fprintf(output, "%" PRIu64 ",", jo1_finish_times.i);
	fprintf(output, "%" PRIu64 "\n", maxnodes);
	fclose(output);

	output = fopen64(strcat(strcpy(aux, output_namefile), "_m_queued_jobs"), "wt");
	if(output==NULL) terror("Could not open output file");
	for (i = 0; i < queued_jobs.i; i++) {
		fprintf(output, "%" PRIu64, queued_jobs.array[i]);
		if (i < queued_jobs.i - 1) {
			fprintf(output, ",");
		}
	}
	fprintf(output, "\n");
	fclose(output);

	output = fopen64(strcat(strcpy(aux, output_namefile), "_m_diff_submit_start_jobs"), "wt");
	if(output==NULL) terror("Could not open output file");
	for (i = 0; i < diff_submit_start_jobs.i; i++) {
		fprintf(output, "%lf", diff_submit_start_jobs.array[i]);
		if (i < diff_submit_start_jobs.i - 1) {
			fprintf(output, ",");
		}
	}
	fprintf(output, "\n");
	fclose(output);

	fill_scaled_stat_per_time(cost_per_minute, daily_time, makespan, time_unit, strcat(strcpy(aux, output_namefile), "_m_cost"));
	free(cost_per_minute.array);
	free(daily_time.array);
	free(cost_per_second.array);
	scaled_stat_per_time(queued_jobs, jobs_times, makespan, time_unit, strcat(strcpy(aux, output_namefile), "_m_queued"));
	free(queued_jobs.array);
	scaled_stat_per_time(launched_jobs, jobs_times, makespan, time_unit, strcat(strcpy(aux, output_namefile), "_m_launched"));
	free(launched_jobs.array);
	scaled_stat_per_time(finished_jobs, jobs_times, makespan, time_unit, strcat(strcpy(aux, output_namefile), "_m_finished"));
	free(finished_jobs.array);
	scaled_stat_per_time(aborted_jobs, jobs_times, makespan, time_unit, strcat(strcpy(aux, output_namefile), "_m_aborted"));
	free(aborted_jobs.array);
	scaled_throughput(job_finish_times, makespan, time_unit, strcat(strcpy(aux, output_namefile), "_m_job_finish_times"));
	free(job_finish_times.array);
	fill_scaled_stat_per_time(cpu_usage, jobs_times, makespan, time_unit, strcat(strcpy(aux, output_namefile), "_m_cpu_usage"));
	free(cpu_usage.array);
	fill_scaled_stat_per_time(mem_usage, jobs_times, makespan, time_unit, strcat(strcpy(aux, output_namefile), "_m_mem_usage"));
	free(mem_usage.array);
	fill_scaled_stat_per_time(nodes_usage, jobs_times, makespan, time_unit, strcat(strcpy(aux, output_namefile), "_m_nodes_usage"));
	free(nodes_usage.array);

	fclose(input_file);
	free(jobs_times.array);
	free(diff_submit_start_jobs.array);
}

void scaled_throughput(uint64_t_array job_finish_times, uint64_t makespan, uint64_t time_unit, char *namefile) {
	FILE *output = fopen64(namefile, "wt");
	if(output==NULL) terror("Could not open output file");

	uint64_t **m = (uint64_t **) malloc((makespan/time_unit) * sizeof(uint64_t *));
	uint64_t i;
	uint64_t j;

	for (i = 0; i < makespan/time_unit; i++) {
        m[i] = (uint64_t *) malloc(COLUMNS_SCALED_THROUGHPUT * sizeof(uint64_t));
    }

	for (i = 0; i < makespan/time_unit; i++) {
		m[i][0] = i+1;
		m[i][1] = 0;
	}

	for (i = 0; i < job_finish_times.i; i++) {
		j = job_finish_times.array[i]/time_unit - 1;
		m[j][1] = m[j][1] + 1;
	}

	fprintf(output, "\"V1\",\"V2\"\n");
	for (j = 0; j < makespan/time_unit; j++) {
		fprintf(output, "\"%" PRIu64 "\",%" PRIu64 ",%" PRIu64 "\n", m[j][0], m[j][0], m[j][1]);
	}

	for (i = 0; i < makespan/time_unit; i++) {
		free(m[i]);
	}

	free(m);
	fclose(output);
}

void scaled_stat_per_time(uint64_t_array stat_object, uint64_t_array time_object, uint64_t makespan, uint64_t time_unit, char *namefile) {
	FILE *output = fopen64(namefile, "wt");
    if(output==NULL) terror("Could not open output file");

	uint64_t **m = (uint64_t **) malloc((makespan/time_unit) * sizeof(uint64_t *));
	uint64_t i;
	uint64_t j;

	for (i = 0; i < makespan/time_unit; i++) {
        m[i] = (uint64_t *) malloc(COLUMNS_SCALED_THROUGHPUT * sizeof(uint64_t));
    }

	for (i = 0; i < makespan/time_unit; i++) {
		m[i][0] = i+1;
		m[i][1] = 0;
	}

	for (i = 0; i < stat_object.i; i++) {
		j = time_object.array[i]/time_unit - 1;
		m[j][1] = stat_object.array[i];
	}

	fprintf(output, "\"V1\",\"V2\"\n");
	for (j = 0; j < makespan/time_unit; j++) {
		fprintf(output, "\"%" PRIu64 "\",%" PRIu64 ",%" PRIu64 "\n", m[j][0], m[j][0], m[j][1]);
	}

	for (i = 0; i < makespan/time_unit; i++) {
		free(m[i]);
	}

	free(m);
	fclose(output);
}

void fill_scaled_stat_per_time(uint64_t_array stat_object, uint64_t_array time_object, uint64_t makespan, uint64_t time_unit, char *namefile) {
	FILE *output = fopen64(namefile, "wt");
    if(output==NULL) terror("Could not open output file");

	uint64_t **m = (uint64_t **) malloc((makespan/time_unit) * sizeof(uint64_t *));
	uint64_t i;
	uint64_t j;
	uint64_t c;
	uint64_t prev = 0;

	for (i = 0; i < makespan/time_unit; i++) {
        m[i] = (uint64_t *) malloc(COLUMNS_SCALED_THROUGHPUT * sizeof(uint64_t));
    }

	for (i = 0; i < makespan/time_unit; i++) {
		m[i][0] = i+1;
		m[i][1] = 0;
	}

	for (i = 0; i < stat_object.i; i++) {
		c = time_object.array[i]/time_unit - 1;
		for (j = prev; j <= c; j++) {
			m[j][1] = stat_object.array[i];
		}
		prev = c;
	}

	fprintf(output, "\"V1\",\"V2\"\n");
	for (j = 0; j < makespan/time_unit; j++) {
		fprintf(output, "\"%" PRIu64 "\",%" PRIu64 ",%" PRIu64 "\n", m[j][0], m[j][0], m[j][1]);
	}

	for (i = 0; i < makespan/time_unit; i++) {
		free(m[i]);
	}

	free(m);
	fclose(output);
}

uint64_t get_time_unit(char time_unit_symbol) {
	if (time_unit_symbol == 's') return SECOND;
	if (time_unit_symbol == 'm') return MINUTE;
	if (time_unit_symbol == 'h') return HOUR;
	if (time_unit_symbol == 'd') return DAY;
	if (time_unit_symbol == 'w') return WEEK;
	if (time_unit_symbol == 'M') return MONTH;
	terror("Invalid time unit symbol");
	return 0;
}

void init_args(int argc, char ** argv, FILE **input, char *output_namefile, char *time_unit_symbol){
	uint64_t arg_index = 0;
	while(arg_index < (unsigned) argc) {
		if (!strcmp(argv[arg_index], "--help")) {
			fprintf(stdout, "USAGE:\n");
            fprintf(stdout, "\t./log_analyzer -in input_file -out output_file -t time_unit\n");
            exit(1);
		} else if (!strcmp(argv[arg_index], "-in")) {
			*input = fopen64(argv[arg_index+1], "rt");
        	if(input==NULL) terror("Could not open input file");
		} else if (!strcmp(argv[arg_index], "-out")) {
			strcpy(output_namefile, argv[arg_index+1]);
		} else if (!strcmp(argv[arg_index], "-t")) {
			*time_unit_symbol = argv[arg_index+1][0];
		}
		arg_index++;
	}
}
