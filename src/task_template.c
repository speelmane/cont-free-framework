/* TASK TEMPLATE
For all occurences of "XXX" string/substring, replace it with the corresponding number, e.g., 5, 6 etc, which you have defined for your task.
Please note that section usage must be UNIQUE, i.e., every task must use a separate file, marked with a section attribute unique to the task.
This template identifies the general process of creating a task based on the 3-phase task model.
*/

#include <string.h>
#include <stdio.h>
#include "pico/platform.h"
#include "common.h"

/* This is an attribute tag, that one must use to mark the "execute" function. The structure is as follows:
#define __attribute_name(group) __attribute__((used, section(".section_name." group)))
The section name corresponds to the section name created in the "linker_script.ld" file,
thus, replace "XXX" with the corresponding number created in the linker script for this task.
*/
#define __taskXXX_runtime_copy(group) __attribute__((used, section(".taskXXX_runtime_copy." group)))

/* 
    Any custom defines and type declarations must be declared here
*/

#define ANY_NUMBER 10

/*
 * Create input and output typedefs, put all necessary values WITHIN the structs only.
*/

typedef struct 
{
    int input_data_1;
    int input_data_2[ANY_NUMBER];
} data_in_t;

typedef struct 
{
    int output_data_1;
    int output_data_2[ANY_NUMBER];
    char output_data_3;
} data_out_t;

/**
 * Local data type definition, do not modify!
 * */
typedef struct 
{
    data_in_t local_data_in;
    data_out_t local_data_out;
} local_data_t;



/* Initialize data_in with corresponding input values of your choice.
The definition must be:
                        static
                        marked with a __scratch_y("any string")
*/
static __scratch_y("taskXXX") data_in_t data_in = {
    .input_data_1 = 2,
    .input_data_2 = {1,2,3,4,5,6,7,8,9,0}};

/* Output data is filled during task execution, initialization is not needed */
static __scratch_y("taskXXX") data_out_t data_out;


/* Example of a function defined to be called from the execute phase */
void __inline_external(function_called_from_execute_task)(int any_relevant_data_type[])
{
    /* A call to this function is allowed from "execute_function, as long as the __inline_external attribute is used */
}

/* Place the "execute" atribute defined in  to note that this is an exec function */
void __taskXXX_runtime_copy("taskXXX")(taskXXX_E)(local_data_t * local_data)
{

    int internal_execute_data;
    /* example retrieval of local_data_in to operate on during the execute function */
    internal_execute_data = local_data->local_data_in.input_data_1 * 5;

    int internal_execute_data_passed[ANY_NUMBER];
    for (int i = 0; i < ANY_NUMBER; i++)
    {
        internal_execute_data_passed[i] = local_data->local_data_in.input_data_2[i];
    }
    
    /* Example call to an inlined function */
    function_called_from_execute_task(internal_execute_data_passed);

    /* example push of results produced by the execute function to local_data_out */
    local_data->local_data_out.output_data_1 = internal_execute_data;
}


void taskXXX(subschedule_t subschedule)
{
    /* Init + read routine (FLASH) */
    local_data_t local_data;
    void (*exec_copy_func)(local_data_t * local_data);

    /* Get the previously defined linker script symbols to the C code*/
    extern char __taskXXX_runtime_copy_start__[],  __taskXXX_runtime_copy_end__[];

    /* Perform memcpy on data */
    memcpy(&local_data.local_data_in, &data_in, sizeof(data_in));

    /* Calculate "execute" code size.
    */
    int func_size = (int) (__taskXXX_runtime_copy_end__) - (int)(__taskXXX_runtime_copy_start__);

    /* Perform memcpy on code.
     */
    exec_copy_func = (memcpy(subschedule.exec_copy_func_dst, __taskXXX_runtime_copy_start__, func_size) + 1); // note the +1 because the return address is even but code must execute from an odd address (little endian)

    /* End of Init + read routine */

    /* Exec routine (RAM)*/
    exec_copy_func(&local_data);
    /* End of Exec routine */

    /* Write routine*/
    memcpy(&data_out, &local_data.local_data_out, sizeof(data_out));
    /* End of Write routine*/

    /* End of Write routine and end of task job, return to the scheduler */
}
