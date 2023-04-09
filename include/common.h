#ifndef COMMON
#define COMMON

#include "stdint.h"

typedef struct
{   
    char cpu_id;
    char * exec_copy_func_dst;
    uint64_t (* timestamp_func)();
    void (* sleep_func)(uint64_t ms);
    int64_t r_to_e_wait_time;
    int64_t e_to_w_wait_time;
} subschedule_t;

/* All task prototypes */
void task1(subschedule_t subschedule);

void task2(subschedule_t subschedule);


#endif /*COMMON*/