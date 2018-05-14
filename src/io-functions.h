#pragma once

#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include <string>
#include "structs.h"



uint64_t normalize_date(const char * d1);

uint64_t walltime_to_seconds(char * wtime);

int picasso_read_row(FILE * f_in, Picasso_row * pr);

std::string seconds_to_date_char(uint64_t seconds);