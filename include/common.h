#ifndef COMMON
#define COMMON

#include "stdint.h"

typedef struct
{   
    char cpu_id;
    int32_t r_to_e_wait_time;
    int32_t e_to_w_wait_time;
} subschedule_t;

/* All task prototypes */
void task1(subschedule_t subschedule);

#endif /*COMMON*/