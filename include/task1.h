#ifndef TASK1
#define TASK1

#include "common.h"
#include "pico/stdlib.h"

// define local and memory variables here
typedef struct 
{
    int32_t sram_memory;
    int32_t local_data_in[8];
    int32_t local_data_out[8];
} local_data_t;

void task1(subschedule_t subschedule);

#endif /*TASK1*/