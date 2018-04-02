#ifndef STRUCTS_H
#define STRUCTS_H

#include <inttypes.h>
#define SECOND 1
#define MINUTE 60
#define HOUR 3600
#define DAY 86400
#define WEEK 604800
#define MONTH 2419200
#define READ_BUFFER 5000000
#define ARRAY_SIZE 100000 // declarar con esto, contar elementos, comprobar reallocs
#define LINE_BUFFER 500
#define COLUMNS_SCALED_THROUGHPUT 2
#define JOB_FINISHED_LENGTH 14
#define SYS_LOAD_LENGTH 10
#define SYSTEM_OFF_LENGTH 12

typedef struct uint64_t_array {
	uint64_t *array;
	uint64_t i;
	uint64_t number_of_reallocs;
} uint64_t_array;

#endif
