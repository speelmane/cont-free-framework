/* TASK 5
    Purely based on a TaclE benchmark - binary_search
*/

#include <string.h>
#include <stdio.h>
#include "common.h" 
#include "pico/platform.h"


/* 
    custom defines (e.g. from the benchmark) and type declarations
*/

struct binarysearch_DATA {
  int key;
  int value;
};

/*
    Define data in /data out data types, structs can also be 0.
*/
typedef struct
{
    int keys[15];
    int values[15];
    int x;
} data_in_t;

typedef struct 
{
   int binary_search_result;
} data_out_t;

/*
    Local data type definition.
    DO NOT MODIFY!
*/
typedef struct 
{
    data_in_t local_data_in;
    data_out_t local_data_out;
    char volatile * write_release_flag;
} local_data_t;


#define __task4_runtime_copy(group) __attribute__((used, section(".task4_runtime_copy." group)))

/* 
    Initialize data input to necessary values (independent tasks)
    If none used, keep a:
    static const data_in_t data_in = {};
*/
static __scratch_y("task4") data_in_t data_in = {
    .x = 8,
    .keys = {81, 2753, 1056, 7178, 4326, 3338, 3711, 4283, 3641, 4588, 7516, 1003, 586, 6913, 3746},
    .values = {2759, 1955, 2914, 7640, 694, 6905, 7944, 3070, 6729, 3160, 4024, 3960, 5164, 4775, 4504}
    };
static __scratch_y("task4") data_out_t data_out;

/*
    Place the attribute to note that this is an exec function
*/
void __task4_runtime_copy("task4")(task4_E)(local_data_t * local_data)
{
    struct binarysearch_DATA binarysearch_data[ 15 ];

    /* Init section */
    int i;
    
    _Pragma( "loopbound min 15 max 15" )
    for ( i = 0; i < 15; ++i ) {
        binarysearch_data[ i ].key = local_data->local_data_in.keys[ i ];
        binarysearch_data[ i ].value = local_data->local_data_in.values[ i ];
    }
    
    /* End of init section */

    int fvalue, mid, up, low;

    low = 0;
    up = 14;
    fvalue = -1;

    _Pragma( "loopbound min 1 max 4" )
    while ( low <= up ) {
        mid = ( low + up ) >> 1;

        if ( binarysearch_data[ mid ].key == local_data->local_data_in.x) {
        /* Item found */
        up = low - 1;
        fvalue = binarysearch_data[ mid ].value;
        } else

        if ( binarysearch_data[ mid ].key > local_data->local_data_in.x )
            /* Item not found */
            up = mid - 1;
        else
            low = mid + 1;
    }

    local_data->local_data_out.binary_search_result = (fvalue - ( -1 ) != 0);

    //MANDATORY ROUTINE IN ALL TASKS 
    while(!(*(local_data->write_release_flag))); // wait for the write flag to be released
    *(local_data->write_release_flag) = 0; // reset the write flag
}

/* This will execute from the flash, except for the task_exec function
*  Future implementation: subschedule the phases - 
*  DELAY between task_read and task_exec functionality 
*  subschedule.sleep_func(subschedule.r_to_e_wait_time)
*/
void task4(subschedule_t subschedule) //add relative waiting times as a parameter here
{
    #ifdef DEBUG
        printf("Task 5 entered \n");
        uint64_t timestamp_before_read_phase = subschedule.timestamp_func();
    #endif

    #ifdef TIMESTAMP
        uint64_t timestamp_READ = subschedule.timestamp_func();
    #endif

    /* Init + read routine (FLASH) */
    local_data_t local_data;
    void (*exec_copy_func)(local_data_t * local_data);

    extern char __task4_runtime_copy_start__[],  __task4_runtime_copy_end__[];

    /* Perform memcpy on data and code */
    memcpy(&local_data.local_data_in, &data_in, sizeof(data_in));
    local_data.write_release_flag = subschedule.write_release_flag;

    int func_size = (int) (__task4_runtime_copy_end__) - (int)(__task4_runtime_copy_start__);

    /* If multiple functions are called, multiple can be copied but destination location for following functions must be adjusted according to the size of the previous function */
    exec_copy_func = (memcpy(subschedule.exec_copy_func_dst, __task4_runtime_copy_start__, func_size) + 1); // note the +1 because the return address is even but the function must execue from an odd address (little endian)

    #ifdef DEBUG
        printf("Task 5 copied func pointer: %p of size: %x\n", exec_copy_func, func_size);
                printf("Task 5 Runtime copy start: %p\n", __task4_runtime_copy_start__);
        printf("Task 5 Runtime copy end: %p\n", __task4_runtime_copy_end__);
        printf("Task 5 Core end used: %p\n", subschedule.exec_copy_func_dst);
        printf("Task 5 Data in (should be SRAM_5) address, size: %d: %p\n", &data_in, sizeof(data_in));
        printf("Task 5 Data out (should be SRAM_5) address, size: %d: %p\n", &data_in, sizeof(data_out));
        printf("Task 5 Local data IN address: %p\n", &local_data.local_data_in);
        printf("Task 5 Local data OUT address: %p\n", &local_data.local_data_out);
    #endif

    /* End of Init + read routine */


    #ifdef TIMESTAMP
        uint64_t timestamp_EXECUTE = subschedule.timestamp_func();
    #endif

    /* Exec routine (RAM)*/
    exec_copy_func(&local_data);
    /* End of Exec routine */

    #ifdef TIMESTAMP
        uint64_t timestamp_WRITE = subschedule.timestamp_func();
    #endif

    /* Write routine*/
    memcpy(&data_out, &local_data.local_data_out, sizeof(data_out));
    /* End of Write routine*/


    #ifdef TIMESTAMP
        uint64_t timestamp_PASS = subschedule.timestamp_func();
        printf("\n\nCORE %d, T4\nRead: %lli, execute: %lli, write: %lli, pass: %lli\n", subschedule.cpu_id, timestamp_READ, timestamp_EXECUTE, timestamp_WRITE, timestamp_PASS);
        printf("Return value: %d\n", data_out.binary_search_result);
    #endif

    /* End of Write routine and end of task job, return to the scheduler */
}