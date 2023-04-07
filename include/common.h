#ifndef COMMON
#define COMMON
#include "pico/stdlib.h"

typedef struct
{   
    char cpu_id;
    int32_t r_to_e_wait_time;
    int32_t e_to_w_wait_time;
} subschedule_t;

#endif /*COMMON*/