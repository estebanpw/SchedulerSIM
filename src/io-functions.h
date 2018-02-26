#pragma once

#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include "structs.h"



uint64_t normalize_date(const char * d1);

int picasso_read_row(FILE * f_in, Picasso_row * pr);