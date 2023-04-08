#include "common.h" // subschedule definition and 
#include "task1.h" //data type, attribute and section start/end definitions (TODO)
#include <string.h>
#include <stdio.h>

#define __task1_runtime_copy(group) __attribute__((used, section(".task1_runtime_copy." group)))
#define __task1_runtime_copy_add(group) __attribute__((used, section(".task1_runtime_copy_add." group)))
#define DEBUG 0

/* Initialize global variables, initialization and a !!const!! modifier MANDATORY!
** If the conditions are met, the variable is placed in .rodata section
** .rodata will be placed in the flash, regardless of the accessing core.
*/
const data_in_t data_in = {.coefficients = {1,2,3,4,5,6,7,8}};

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
    /* Init + read routine (FLASH) */
    local_data_t local_data;
    void (*exec_copy_func)(local_data_t * local_data);
    char * exec_copy_func_dst;

    #ifdef DEBUG
        printf("Local data address: %p\n", &local_data);
        printf("Data in (should be flash) address: %p\n", &data_in);
        printf("Local data IN address: %p\n", &local_data.local_data_in);
        printf("Local data OUT address: %p\n", &local_data.local_data_out);
    #endif

    extern char __core_0_end__[], __core_1_end__[], __task1_runtime_copy_start__[],  __task1_runtime_copy_end__[];
    #ifdef DEBUG
        printf("Runtime copy start: %p\n", __task1_runtime_copy_start__);
        printf("Runtime copy end: %p\n", __task1_runtime_copy_end__);
        printf("Core 0 end: %p\n", __core_0_end__);
        printf("Core 1 end: %p\n", __core_1_end__);
    #endif

    if(subschedule.cpu_id == 0)
    {
        exec_copy_func_dst = __core_0_end__; //set code location for core 0
    }
    else
    {
        exec_copy_func_dst = __core_1_end__; //set code location for core 1
    }

    /* Perform memcpy on data and code */
    memcpy(&local_data.local_data_in, &data_in, sizeof(data_in));

    int func_size = (int) (__task1_runtime_copy_end__) - (int)(__task1_runtime_copy_start__);

    #ifdef DEBUG
        printf("Size: %x\n", func_size);
    #endif


    /* If multiple functions are called, multiple can be copied but destination location for following functions must be adjusted*/
    exec_copy_func = (memcpy(exec_copy_func_dst, __task1_runtime_copy_start__, func_size) + 1); // note the +1 because the return address is even but the function must execue from an odd address (little endian)

    #ifdef DEBUG
        printf("Copied func pointer: %p\n", exec_copy_func);
    #endif

    /* End of Init + read routine */

    /* DELAY between task_read and task_exec functionality */
    // WAIT_MS(0);

    /* Exec routine (RAM)*/
    exec_copy_func(&local_data);

    /* End of Exec routine */

    /* DELAY between task_exec and task_write functionality */
    // WAIT_MS(0);

    /* Write routine (FLASH)*/

    /* TMP!! 
    * SOME PRINTS HERE TO CHECK VALUES ? */
   #ifdef DEBUG
    printf("Flash routine here again\n");
   #endif

    /* End of Write routine and end of task job, return to the scheduler */
}





















/**
 * static void task1_R(local_data_t * local_data)
{
    memcpy(&local_data->local_data_in, &global_coefficients, sizeof(global_coefficients));
    memcpy(local_data->sram_memory, task1_E, )
}

// put this in a pragma of a specific code segment
void task1_E(local_data_t * local_data)
{
    local_data->local_data_out[0] = 32;
    // do stuff to struct->task1.local_in and struct->task1.local_out
}
// end the pragma

static void task1_W(local_data_t * local_data)
{
    flash_write(local_data->)
}

    main sequence:

    get CPU id for sram reference
    void (*task1_E_RAM)(); //define a function pointer for the ram func

    Perform the 3-phases
    task1_E_RAM = task1_R(&local_data);

    wait first interval R to E
    sleep_ms(subschedule.r_to_e_wait_time);
    *(task1_E_RAM)(&local_data);

    wait second interval E to W
    sleep_ms(subschedule.e_to_w_wait_time);
    task1_W(&local_data);
*/