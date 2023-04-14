/* TASK 2
    Purely based on a TaclE benchmark - insertsort
*/

#include <string.h>
#include <stdio.h>
// subschedule and function prototype location
#include "common.h" 
#include "pico/platform.h"


/*
    Additional type declarations
*/

/*
    Define data in /data out data types, structs can also be 0.
*/
typedef struct
{
    unsigned int a[ 11 ];
} data_in_t;

typedef struct 
{
    unsigned int a[ 11 ];   
    int return_result;
} data_out_t;

/*
    Local data type definition.
    DO NOT MODIFY!
*/
typedef struct 
{
    data_in_t local_data_in;
    data_out_t local_data_out;
} local_data_t;


#define __task2_runtime_copy(group) __attribute__((used, section(".task2_runtime_copy." group)))

/* Initialize global variables, initialization and a !!const!! modifier MANDATORY!
    If the conditions are met, the variable is placed in .rodata section
    .rodata will be placed in the flash, regardless of the accessing core.
*/
static __scratch_y("task3") data_in_t data_in = {.a = {0, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2}};

static __scratch_y("task3") data_out_t data_out;

void __task2_runtime_copy("task2")(task2_E)(local_data_t * local_data)
{
    unsigned int insertsort_a[ 11 ];
    int insertsort_iters_i, insertsort_min_i, insertsort_max_i;
    int insertsort_iters_a, insertsort_min_a, insertsort_max_a;

    insertsort_iters_i = 0;
    insertsort_min_i = 100000;
    insertsort_max_i = 0;
    insertsort_iters_a = 0;
    insertsort_min_a = 100000;
    insertsort_max_a = 0;

    /* Initialize routine */
    register volatile int k;
    _Pragma( "loopbound min 11 max 11" )
    for ( k = 0; k < 11; k++ )
        insertsort_a[ k ] = local_data->local_data_in.a[ k ];

    /* Main entrypoint */
    int  i, j, temp;
    i = 2;

    insertsort_iters_i = 0;

    _Pragma( "loopbound min 9 max 9" )
    while ( i <= 10 ) {

        insertsort_iters_i++;

        j = i;

        insertsort_iters_a = 0;

        _Pragma( "loopbound min 1 max 9" )
        while ( insertsort_a[ j ] < insertsort_a[ j - 1 ] ) {
        insertsort_iters_a++;

        temp = insertsort_a[ j ];
        insertsort_a[ j ] = insertsort_a[ j - 1 ];
        insertsort_a[ j - 1 ] = temp;
        j--;
        }

        if ( insertsort_iters_a < insertsort_min_a )
        insertsort_min_a = insertsort_iters_a;
        if ( insertsort_iters_a > insertsort_max_a )
        insertsort_max_a = insertsort_iters_a;

        i++;
    }

    if ( insertsort_iters_i < insertsort_min_i )
        insertsort_min_i = insertsort_iters_i;
    if ( insertsort_iters_i > insertsort_max_i )
        insertsort_max_i = insertsort_iters_i;

    /* Return routine */
    int in, returnValue = 0;

    _Pragma( "loopbound min 11 max 11" )
    for ( in = 0; in < 11; in++ )
    {
        returnValue += insertsort_a[ in ];
        local_data->local_data_out.a[in] = insertsort_a[in];
    }

    local_data->local_data_out.return_result = ( returnValue + ( -65 ) ) != 0;

}

/*
    This will execute from the flash, except for the task_exec function
*/
void task2(subschedule_t subschedule) //add relative waiting times as a parameter here
{
    #ifdef DEBUG
        printf("Task 2 entered \n");
        uint64_t timestamp_before_read_phase = subschedule.timestamp_func();
    #endif

    #ifdef TIMESTAMP
        uint64_t timestamp_READ = subschedule.timestamp_func();
    #endif

    /* Init + read routine (FLASH) */
    local_data_t local_data;
    void (*exec_copy_func)(local_data_t * local_data);

    extern char __task2_runtime_copy_start__[],  __task2_runtime_copy_end__[];

    #ifdef DEBUG
        printf("Task2 Runtime copy start: %p\n", __task2_runtime_copy_start__);
        printf("Task2 Runtime copy end: %p\n", __task2_runtime_copy_end__);
        printf("Task2 Core end used: %p\n", subschedule.exec_copy_func_dst);
        printf("Task2 Data in (should be SRAM_5) address: %p\n", &data_in);
        printf("Task2 Local data IN address: %p\n", &local_data.local_data_in);
        printf("Task2 Local data OUT address: %p\n", &local_data.local_data_out);
    #endif

    /* Perform memcpy on data and code */
    memcpy(&local_data.local_data_in, &data_in, sizeof(data_in));

    int func_size = (int) (__task2_runtime_copy_end__) - (int)(__task2_runtime_copy_start__);

    #ifdef TIMESTAMP
        uint64_t timestamp_EXECUTE = subschedule.timestamp_func();
    #endif

    /* If multiple functions are called, multiple can be copied but destination location for following functions must be adjusted according to the size of the previous function */
    exec_copy_func = (memcpy(subschedule.exec_copy_func_dst, __task2_runtime_copy_start__, func_size) + 1); // note the +1 because the return address is even but the function must execue from an odd address (little endian)

    #ifdef DEBUG
        uint64_t timestamp_after_read_phase = subschedule.timestamp_func();
        printf("Task2 copied func pointer: %p of size: %x\n", exec_copy_func, func_size);
    #endif

    /* End of Init + read routine */

    /* DELAY between task_read and task_exec functionality */
    subschedule.sleep_func(subschedule.r_to_e_wait_time);

    #ifdef DEBUG
        uint64_t timestamp_before_exec_phase = subschedule.timestamp_func();
    #endif
    /* Exec routine (RAM)*/
    exec_copy_func(&local_data);

    #ifdef DEBUG
        uint64_t timestamp_after_exec_phase = subschedule.timestamp_func();
    #endif

    /* End of Exec routine */

    /* DELAY between task_exec and task_write functionality */
    subschedule.sleep_func(subschedule.e_to_w_wait_time);

    /* Write routine*/
    #ifdef TIMESTAMP
        uint64_t timestamp_WRITE = subschedule.timestamp_func();
    #endif


    memcpy(&data_out, &local_data.local_data_out, sizeof(data_out));


   /* Note that the next time the task runs, the same (initial) input data is going to be used.
    If that should not be the case, assign corresponding out data to input data too.
   */

    #ifdef TIMESTAMP
        printf("Return value: %d\n", data_out.return_result);
        uint64_t timestamp_PASS = subschedule.timestamp_func();
        printf("\n\nCORE %d, T2\nRead: %lli, execute: %lli, write: %lli, pass: %lli \n", subschedule.cpu_id, timestamp_EXECUTE, timestamp_WRITE, timestamp_PASS);
    #endif

    /* End of Write routine and end of task job, return to the scheduler */
}