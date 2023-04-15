#include "common.h"
#include "stdint.h"
#include "timer_functions.h"

void __core_0_code("tablefill")(fill_schedule_core_0)(table_entry_t table[])
{
    /*
        All time elements, if used, are interpreted in us
    */
    uint64_t scheduled_wait_time_array[TABLE_SIZE_CORE_0] = {6000000, 6000000, 6000000, 6000000}; //500 ms, 600 ms
    uint64_t r_to_e_wait_time_array[TABLE_SIZE_CORE_0] = {100, 100, 100, 100}; 
    uint64_t e_to_w_wait_time_array[TABLE_SIZE_CORE_0] = {100, 100, 100, 100};
    void (* task_array[TABLE_SIZE_CORE_0])(subschedule_t) = {task1, task2, task3, task4};


    extern char __core_0_code_end__[];
    for (int i=0; i<TABLE_SIZE_CORE_0; i++)
    {
        table[i].scheduled_wait_time = scheduled_wait_time_array[i];

        subschedule_t subschedule =
        {
            .cpu_id = 0,
            .r_to_e_wait_time = r_to_e_wait_time_array[i],
            .e_to_w_wait_time = e_to_w_wait_time_array[i],
            .exec_copy_func_dst = __core_0_code_end__,
            .sleep_func = sleep_ms_core_0,
            .timestamp_func = timestamp_core_0
        };

        table[i].subschedule = subschedule;

        table[i].task = task_array[i];
   } 
}

void __core_1_code("tablefill")(fill_schedule_core_1)(table_entry_t table[])
{
    /*
        All time elements, if used, are interpreted in us
    */
    uint64_t scheduled_wait_time_array[TABLE_SIZE_CORE_1] = {5000000, 7000000}; //500 ms, 600 ms
    uint64_t r_to_e_wait_time_array[TABLE_SIZE_CORE_1] = {200, 200}; //10 and 30 ms
    uint64_t e_to_w_wait_time_array[TABLE_SIZE_CORE_1] = {200, 200}; //20 and 40 ms
    void (* task_array[TABLE_SIZE_CORE_1])(subschedule_t) = {task1, task3};


    extern char __core_1_code_end__[];

   for (int i=0; i<TABLE_SIZE_CORE_1; i++)
   {
    table[i].scheduled_wait_time = scheduled_wait_time_array[i];

    subschedule_t subschedule =
    {
        .cpu_id = 1,
        .r_to_e_wait_time = r_to_e_wait_time_array[i],
        .e_to_w_wait_time = e_to_w_wait_time_array[i],
        .exec_copy_func_dst = __core_1_code_end__,
        .sleep_func = sleep_ms_core_1,
        .timestamp_func = timestamp_core_1
    };

    table[i].subschedule = subschedule;

    table[i].task = task_array[i];

   }
    
}