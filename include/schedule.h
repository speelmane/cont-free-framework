#ifndef SCHEDULE
#define SCHEDULE

#include "stdint.h"
#include "common.h"

#define __core_0_code(group) __attribute__((section(".core_0_code." group)))
#define __core_1_code(group) __attribute__((section(".core_1_code." group)))
#define __core_0_data(group) __attribute__((section(".core_0_data." group)))
#define __core_1_data(group) __attribute__((section(".core_1_data." group)))

#define __inline_external __attribute__((always_inline))

#define TABLE_SIZE_CORE_0 8
#define TABLE_SIZE_CORE_1 3

// Keep it to a second (1 million us) to ensure perfect synchronisation
#define SCHEDULE_OFFSET 1000000
#define START_OFFSET_CORE_0 150
#define START_OFFSET_CORE_1 0


typedef struct
{
    int64_t scheduled_wait_time;
    void (* task)(subschedule_t subschedule);
    subschedule_t subschedule;
} table_entry_t;


void fill_schedule_core_0(table_entry_t table[]);

void fill_schedule_core_1(table_entry_t table[]);


#endif /*SCHEDULE*/