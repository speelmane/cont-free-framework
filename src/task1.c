#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "task1.h"

/* Define global variables, initialization and a const modifier MANDATORY!
** If the conditions are met, the variable is placed in .rodata section
** .rodata will be placed in the flash, regardless of the accessing core.
*/
const int32_t global_coefficients[] = {1, 2, 3, 4, 5, 6, 7, 8};
/**/

static void task1_R(local_data_t * local_data)
{
    // memcpy(&local_data->local_data_in, &global_coefficients, sizeof(global_coefficients));
    // memcpy(local_data->sram_memory, task1_E, )
}

// put this in a pragma of a specific code segment
void task1_E(local_data_t * local_data)
{
    printf("Hello from exec\n");
    local_data->local_data_out[0] = 32;
    // do stuff to struct->task1.local_in and struct->task1.local_out
}
// end the pragma

static void task1_W(local_data_t * local_data)
{
    // flash_write(local_data->)
}

void task1(subschedule_t subschedule) //add relative waiting times as a parameter here
{
    //init structure

    local_data_t local_data; //defines local in and out vars

    if(subschedule.cpu_id == 0)
    {
        local_data.sram_memory = SRAM0_BASE; //set code location for core 0
    }
    else
    {
        local_data.sram_memory = SRAM2_BASE; //set code location for core 1
    }

    task1_E(&local_data);
    // get CPU id for sram reference
    // void (*task1_E_RAM)(); //define a function pointer for the ram func

    // Perform the 3-phases
    // task1_E_RAM = task1_R(&local_data);

    // wait first interval R to E
    // sleep_ms(subschedule.r_to_e_wait_time);
    // *(task1_E_RAM)(&local_data);

    // wait second interval E to W
    // sleep_ms(subschedule.e_to_w_wait_time);
    // task1_W(&local_data);
}