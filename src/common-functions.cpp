#include "common-functions.h"

void terror(const char * msg){
    fprintf(stdout, "%s\n", msg);
    exit(-1);
}

void generic_broadcast(int count, ...){
    va_list ap;
    va_start(ap, count);
    std::cout << "[STATUS] ";
    for(int i=0; i<count; i++){
        std::cout << va_arg(ap, char *);
    }
    va_end(ap);
    std::cout << std::endl;
}

/*
  Function to read char by char buffered from a FILE
*/
char buffered_fgetc(char *buffer, uint64_t *pos, uint64_t *read, FILE *f) {
    if (*pos >= READ_BUFFER) {
        *pos = 0;
        memset(buffer, 0, READ_BUFFER);
        *read = fread(buffer, 1, READ_BUFFER, f);
    }
    *pos = *pos + 1;
    return buffer[*pos-1];
}