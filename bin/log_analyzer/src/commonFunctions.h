#ifndef COMMON_FUNCTIONS_H
#define COMMON_FUNCTIONS_H
#include "structs.h"

void terror(char *s);
char buffered_fgetc(char *buffer, uint64_t *pos, uint64_t *read, FILE *f);

#endif /* COMMON_FUNCTIONS_H */
