/* TASK 2
    Purely based on a TaclE benchmark - binary_search
*/

#include <string.h>
#include <stdio.h>
// subschedule and function prototype location
#include "common.h" 
#include "pico/platform.h"


/*
    Additional type declarations
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
} local_data_t;


#define __task2_runtime_copy(group) __attribute__((used, section(".task2_runtime_copy." group)))
#define __inline_external(group) __attribute__((always_inline))
#define DEBUG 0

/* Initialize global variables, initialization and a !!const!! modifier MANDATORY!
    If the conditions are met, the variable is placed in .rodata section
    .rodata will be placed in the flash, regardless of the accessing core.
*/
static const data_in_t data_in = {.x = 8};

/*
    Place the attribute to note that this is an exec function
*/
void __task2_runtime_copy("task2")(task2_E)(local_data_t * local_data)
{
    struct binarysearch_DATA binarysearch_data[ 15 ];

    /* Init section */
    int i;

    int binarysearch_seed = 0;

    _Pragma( "loopbound min 15 max 15" )
    for ( i = 0; i < 15; ++i ) {
        binarysearch_seed = ( ( binarysearch_seed * 133 ) + 81 );

        binarysearch_data[ i ].key = binarysearch_seed;
        binarysearch_seed = ( ( binarysearch_seed * 133 ) + 81);
        binarysearch_data[ i ].value = binarysearch_seed;
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

    local_data->local_data_out.binary_search_result = fvalue;
}

/*
    This will execute from the flash, except for the task_exec function
*/
void task2(subschedule_t subschedule) //add relative waiting times as a parameter here
{
    #ifdef DEBUG
        printf("Task 2 entered \n");
    #endif
    uint64_t timestamp_before = subschedule.timestamp_func();

    /* Init + read routine (FLASH) */
    local_data_t local_data;
    void (*exec_copy_func)(local_data_t * local_data);

    #ifdef DEBUG
        printf("Task2 Data in (should be flash) address: %p\n", &data_in);
        printf("Task2 Local data IN address: %p\n", &local_data.local_data_in);
        printf("Task2 Local data OUT address: %p\n", &local_data.local_data_out);
    #endif

    extern char __task2_runtime_copy_start__[],  __task2_runtime_copy_end__[];

    #ifdef DEBUG
        printf("Task2 Runtime copy start: %p\n", __task2_runtime_copy_start__);
        printf("Task2 Runtime copy end: %p\n", __task2_runtime_copy_end__);
        printf("Task2 Core end used: %p\n", subschedule.exec_copy_func_dst);
    #endif

    /* Perform memcpy on data and code */
    memcpy(&local_data.local_data_in, &data_in, sizeof(data_in));

    int func_size = (int) (__task2_runtime_copy_end__) - (int)(__task2_runtime_copy_start__);

    #ifdef DEBUG
        printf("Task2 exec Size: %x\n", func_size);
    #endif


    /* If multiple functions are called, multiple can be copied but destination location for following functions must be adjusted according to the size of the previous function */
    exec_copy_func = (memcpy(subschedule.exec_copy_func_dst, __task2_runtime_copy_start__, func_size) + 1); // note the +1 because the return address is even but the function must execue from an odd address (little endian)

    #ifdef DEBUG
        printf("Task2 Copied func pointer: %p\n", exec_copy_func);
    #endif

    uint64_t timestamp_after = subschedule.timestamp_func();

    printf("Task2 Setup timestamp before: %lli, after: %lli\n", timestamp_before, timestamp_after);


    /* End of Init + read routine */


    /* DELAY between task_read and task_exec functionality */
    subschedule.sleep_func(subschedule.r_to_e_wait_time);


    /* Exec routine (RAM)*/
    exec_copy_func(&local_data);

    /* End of Exec routine */

    subschedule.sleep_func(subschedule.e_to_w_wait_time);
   
    #ifdef DEBUG
        printf("Task2 Flash routine here again\n");
    #endif

    /* Write routine (FLASH)*/

    /* TMP!! 
    * SOME PRINTS HERE TO CHECK VALUES ? */

    printf("Task2 Local data out is: %d\n", local_data.local_data_out);


    /* End of Write routine and end of task job, return to the scheduler */
}





















/**
 * static void task2_R(local_data_t * local_data)
{
    memcpy(&local_data->local_data_in, &global_coefficients, sizeof(global_coefficients));
    memcpy(local_data->sram_memory, task2_E, )
}

// put this in a pragma of a specific code segment
void task2_E(local_data_t * local_data)
{
    local_data->local_data_out[0] = 32;
    // do stuff to struct->task2.local_in and struct->task2.local_out
}
// end the pragma

static void task2_W(local_data_t * local_data)
{
    flash_write(local_data->)
}

    main sequence:

    get CPU id for sram reference
    void (*task2_E_RAM)(); //define a function pointer for the ram func

    Perform the 3-phases
    task2_E_RAM = task2_R(&local_data);

    wait first interval R to E
    sleep_ms(subschedule.r_to_e_wait_time);
    *(task2_E_RAM)(&local_data);

    wait second interval E to W
    sleep_ms(subschedule.e_to_w_wait_time);
    task2_W(&local_data);
*/