
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include "hardware/structs/bus_ctrl.h"
#include "hardware/regs/busctrl.h"
#include "hardware/structs/xip_ctrl.h"
#include "hardware/regs/xip.h"
#include "pico/multicore.h"
#include "include/common.h" //subschedule
#include "timer_functions.h" //timer_funcs
#include "pico/sem.h"
#include "pico/platform.h"
#include "hardware/structs/scb.h"
#include "schedule.h"


/* Bus performance counters, nr 0 reserved for XIP access counting, the rest can verify other contention sources if that is the wish */
#define bus_perf_0_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR0_OFFSET))
#define bus_perf_1_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR1_OFFSET))
#define bus_perf_2_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR2_OFFSET))
#define bus_perf_3_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR3_OFFSET))

/* Alarm setup */
#define ALARM_CORE_0 0
#define ALARM_CORE_1 1


#define ALARM_IRQ_CORE_0 TIMER_IRQ_0
#define ALARM_IRQ_CORE_1 TIMER_IRQ_1

/* By default the cores should wait until the flag is cleared */
__core_0_data("flag") char volatile alarm_flag_core_0 = false;
__core_1_data("flag") char volatile alarm_flag_core_1 = false;


__core_0_data("loops") uint64_t volatile target_core_0 = SCHEDULE_OFFSET + START_OFFSET_CORE_0;
__core_1_data("loops") uint64_t volatile target_core_1 = SCHEDULE_OFFSET + START_OFFSET_CORE_1;

__core_0_data("table") table_entry_t schedule_table_core_0[TABLE_SIZE_CORE_0];
__core_1_data("table") table_entry_t schedule_table_core_1[TABLE_SIZE_CORE_1];

__core_1_data("vtor") uint32_t vector_table_core_1[48] = {0};

semaphore_t sync_sem;

/* Function prototypes */

static void __core_0_code("irq")(alarm_irq_core_0)(void);
static void __core_1_code("irq")(alarm_irq_core_1)(void);

void __core_1_code("scheduler")(scheduler_core_1)(table_entry_t table[]);
void __core_0_code("scheduler")(scheduler_core_0)(table_entry_t table[]);

void system_init();

/* End of function prototypes */

/*! @brief Core 1 entry point
*   @category core 1
*/
void main_core_1(void)
{
    fill_schedule_core_1(schedule_table_core_1);

    /* Interrupt setup routine */
    hw_set_bits(&timer_hw->inte, 1u << ALARM_CORE_1);
    irq_set_exclusive_handler(ALARM_IRQ_CORE_1, alarm_irq_core_1);
    irq_set_enabled(ALARM_IRQ_CORE_1, true);
    /* End of interrupt setup routine */

    sem_acquire_blocking(&sync_sem);
    scheduler_core_1(schedule_table_core_1);
}

int main() {
    stdio_init_all();
    system_init();
    multicore_lauch_core1_separate_vtor_table(main_core_1, (uint32_t) vector_table_core_1);

    //wait slightly to ensure core 1 is def set up
    sleep_ms_core_0(500000);

    fill_schedule_core_0(schedule_table_core_0);

    /* Interrupt setup routine */
    hw_set_bits(&timer_hw->inte, 1u << ALARM_CORE_0);
    irq_set_exclusive_handler(ALARM_IRQ_CORE_0, alarm_irq_core_0);
    irq_set_enabled(ALARM_IRQ_CORE_0, true);
    /* End of interrupt setup routine */

    sem_release(&sync_sem);
    scheduler_core_0(schedule_table_core_0);
    return 0;
}



/*! @brief Global register and other system function initialization.
*   @category init functions
*
* Performs the following actions:
* - turns off XIP cache
* - sets up performance counters to count contested XIP/SRAM_5 accesses
* - initializes the initial core syncing semaphore
* - sets the core1 VTOR table to valid content
*/
void system_init()
{
    /* XIP cache turn off */ 
    xip_ctrl_hw->ctrl = (xip_ctrl_hw->ctrl & ~XIP_CTRL_EN_BITS);

    /* Counter init to count contested accesses */ 
    bus_perf_0_hw->sel = arbiter_xip_main_perf_event_access_contested;
    bus_perf_1_hw->sel = arbiter_sram5_perf_event_access_contested;
    bus_perf_2_hw->sel = arbiter_xip_main_perf_event_access;



    /* Semaphore init */
    sem_init(&sync_sem, 0, 1);

    /* The content of VTOR table cannot be random, get the content from the core 0 VTOR table*/
    memcpy(vector_table_core_1, (const uint32_t *) scb_hw->vtor, sizeof(vector_table_core_1));
    /* But set the first word to start of stack1 (must be so according to the documentation)*/
    vector_table_core_1[0] = 0x21040000;
}

/*! @brief Task scheduler for core 0
*   @category schedulers
*
* Scheduler is set to be local, thus, executed from SRAM0.
* Initialization routine contains table fill function (should maybe be moved before the scheduler, accept as parameter the schedule)
*/
void __core_0_code("scheduler")(scheduler_core_0)(table_entry_t table[])
{
    /* Release the first task here */
    timer_hw->alarm[ALARM_CORE_0] = (uint32_t) target_core_0;

    // reset the counter
    bus_perf_0_hw->value = 0;
    bus_perf_1_hw->value = 0;

    while(1)
    {
        for(int i=0; i<TABLE_SIZE_CORE_0; i++)
        {
            while(!alarm_flag_core_0);
            alarm_flag_core_0 = false;
            table[i].task(table[i].subschedule);
        }

        // printf("XIP perf counter: %d\n\n", bus_perf_0_hw->value);
        // printf("SRAM_5 perf counter: %d\n\n", bus_perf_1_hw->value);


    }
}

/*! @brief Task scheduler for core 1
*   @category schedulers
*
* Scheduler is set to be local, thus, executed from SRAM2.
* Initialization routine contains table fill function (should maybe be moved before the scheduler, accept as parameter the schedule)
*/
void __core_1_code("scheduler")(scheduler_core_1)(table_entry_t table[])
{
    // Write the lower 32 bits of the target time to the alarm which will arm it
    timer_hw->alarm[ALARM_CORE_1] = (uint32_t) target_core_1;

    while(1)
    {
        for(int i=0; i<TABLE_SIZE_CORE_1; i++)
        {
            while(!alarm_flag_core_1);
            alarm_flag_core_1 = false;
            table[i].task(table[i].subschedule);
        }
    }
}

static void __core_0_code("irq")(alarm_irq_core_0)(void) {
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_CORE_0);

    alarm_flag_core_0 = true;

    static int task_counter = 0;

    if(task_counter < (TABLE_SIZE_CORE_0 - 1) )
    {
        task_counter++;
    }
    else
    {
        task_counter = 0;
    }

    target_core_0 += schedule_table_core_0[task_counter].scheduled_wait_time;

    // Write the lower 32 bits of the target time to the alarm which will arm it
    timer_hw->alarm[ALARM_CORE_0] = (uint32_t) target_core_0;

}

static void __core_1_code("irq")(alarm_irq_core_1)(void) {
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_CORE_1);

    alarm_flag_core_1 = true;

    static int task_counter = 0;

    if(task_counter < (TABLE_SIZE_CORE_1 - 1) )
    {
        task_counter++;
    }
    else
    {
        task_counter = 0;
    }

    target_core_1 += schedule_table_core_1[task_counter].scheduled_wait_time;

    // Write the lower 32 bits of the target time to the alarm which will arm it
    timer_hw->alarm[ALARM_CORE_1] = (uint32_t) target_core_1;

}
