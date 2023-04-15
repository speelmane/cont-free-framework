#ifndef COMMON
#define COMMON

#include "stdint.h"

#define __core_0_code(group) __attribute__((section(".core_0_code." group)))
#define __core_1_code(group) __attribute__((section(".core_1_code." group)))
#define __core_0_data(group) __attribute__((section(".core_0_data." group)))
#define __core_1_data(group) __attribute__((section(".core_1_data." group)))

#define __inline_external __attribute__((always_inline))

// #define DEBUG 0
// #define TIMESTAMP 0

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

void task3(subschedule_t subschedule);

void task4(subschedule_t subschedule);

// void task5(subschedule_t subschedule);


#endif /*COMMON*/