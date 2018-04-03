#include "structs.h"
#include <stdarg.h>
#include <iostream>

void terror(const char * msg);

void generic_broadcast(int count, ...);

char buffered_fgetc(char *buffer, uint64_t *pos, uint64_t *read, FILE *f);