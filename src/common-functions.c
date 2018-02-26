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