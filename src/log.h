#pragma once

#include "structs.h"
#include "common-functions.h"
#include "io-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>

class log
{
private:
    FILE * out_log;

public:
    log(FILE * out);
    void record(int count, ...);
};

extern log * LOG;