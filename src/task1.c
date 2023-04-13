#include <string.h>
#include <stdio.h>
#include "pico/platform.h"
#include "common.h"

/**
 * Create input and output typedefs, put all necessary values WITHIN the structs only.
**/
typedef struct 
{
    uint32_t coefficients[8];
} data_in_t;

typedef struct 
{
    uint32_t coefficients[8];
} data_out_t;

/**
 * Local data type definition, do not modify.
 * */
typedef struct 
{
    data_in_t local_data_in;
    data_out_t local_data_out;
} local_data_t;


#define __task1_runtime_copy(group) __attribute__((used, section(".task1_runtime_copy." group)))

// #define DEBUG 0
#define TIMESTAMP 0

/* Initialize data in with corresponding input values of your choice
*/
static __scratch_y("task1") data_in_t data_in = {.coefficients = {1,2,3,4,5,6,7,8}};
static __scratch_y("task1") data_out_t data_out;


/* Place an attribute to note that this is an exec function */
void __task1_runtime_copy("task1")(task1_E)(local_data_t * local_data)
{
    for(int i = 0; i < 8; i++)
    {
        local_data->local_data_out.coefficients[i] = local_data->local_data_in.coefficients[i] * 3;
    }
}

/* This will execute from the flash, except for the task_exec function*/
void task1(subschedule_t subschedule) //add relative waiting times as a parameter here
{
    #ifdef DEBUG
        printf("Task 1 entered \n");
    #endif

    #ifdef TIMESTAMP
        uint64_t timestamp_READ = subschedule.timestamp_func();
    #endif


    /* Init + read routine (FLASH) */
    local_data_t local_data;
    void (*exec_copy_func)(local_data_t * local_data);

    extern char __task1_runtime_copy_start__[],  __task1_runtime_copy_end__[];

    #ifdef DEBUG
        printf("Task1 Runtime copy start: %p\n", __task1_runtime_copy_start__);
        printf("Task1 Runtime copy end: %p\n", __task1_runtime_copy_end__);
        printf("Task1 Core end used: %p\n", subschedule.exec_copy_func_dst);
        printf("Task1 Data in (should be SRAM_5) address: %p\n", &data_in);
        printf("Task1 Local data IN address: %p\n", &local_data.local_data_in);
        printf("Task1 Local data OUT address: %p\n", &local_data.local_data_out);
    #endif

    /* Perform memcpy on data and code */
    local_data.local_data_in = data_in;

    int func_size = (int) (__task1_runtime_copy_end__) - (int)(__task1_runtime_copy_start__);

    /* If multiple functions are called, multiple can be copied but destination location for following functions must be adjusted according to the size of the previous function */
    exec_copy_func = (memcpy(subschedule.exec_copy_func_dst, __task1_runtime_copy_start__, func_size) + 1); // note the +1 because the return address is even but the function must execue from an odd address (little endian)

    #ifdef DEBUG
        printf("Task1 copied func pointer: %p of size: %x\n", exec_copy_func, func_size);
        uint64_t timestamp_after_read_phase = subschedule.timestamp_func();
    #endif

    /* End of Init + read routine */

    /* DELAY between task_read and task_exec functionality */
    subschedule.sleep_func(subschedule.r_to_e_wait_time);

    /* Exec routine (RAM)*/

    #ifdef TIMESTAMP
        uint64_t timestamp_EXECUTE = subschedule.timestamp_func();
    #endif

    exec_copy_func(&local_data);

    /* End of Exec routine */

    /* DELAY between task_exec and task_write functionality */
    subschedule.sleep_func(subschedule.e_to_w_wait_time);

    /* Write routine*/
    #ifdef TIMESTAMP
        uint64_t timestamp_WRITE = subschedule.timestamp_func();
    #endif


    data_out = local_data.local_data_out;


   /* Note that the next time the task runs, the same (initial) input data is going to be used.
    If that should not be the case, assign corresponding out data to input data too.
   */


    #ifdef TIMESTAMP
        uint64_t timestamp_PASS = subschedule.timestamp_func();
        printf("\n\nCORE %d, T1\nRead: %lli, execute: %lli, write: %lli, pass: %lli\n", subschedule.cpu_id, timestamp_EXECUTE, timestamp_WRITE, timestamp_PASS);
    #endif

    /* End of Write routine and end of task job, return to the scheduler */
}
