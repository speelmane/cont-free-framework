
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

__core_0_data("mask") uint32_t alarm_mask_core_0 = (1u << ALARM_IRQ_CORE_0);
__core_1_data("mask") uint32_t alarm_mask_core_1 = (1u << ALARM_IRQ_CORE_1);

__core_0_data("table") table_entry_t schedule_table_core_0[TABLE_SIZE_CORE_0];
__core_1_data("table") table_entry_t schedule_table_core_1[TABLE_SIZE_CORE_1];

__core_1_data("vtor") uint32_t vector_table_core_1[48] = {0};

semaphore_t sync_sem;

/* Function prototypes */

static void __core_0_code("irq")(alarm_irq_core_0)(void);
static void __core_1_code("irq")(alarm_irq_core_1)(void);

void __core_1_code("sram_2")(scheduler_core_1)(table_entry_t table[]);
void __core_0_code("sram_0")(scheduler_core_0)(table_entry_t table[]);

void system_init();

/* End of function prototypes */

/*! @brief Core 1 entry point
*   @category core 1
*/
void main_core_1(void)
{
    // // fill the scheduling table for the core
    fill_schedule_core_1(schedule_table_core_1);

    /* Interrupt setup routine */
    hw_set_bits(&timer_hw->inte, 1u << ALARM_CORE_1);

    irq_set_exclusive_handler(ALARM_IRQ_CORE_1, alarm_irq_core_1);

    *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET)) = alarm_mask_core_1;
    *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)) = alarm_mask_core_1;
    /* End of interrupt setup routine */

    sem_acquire_blocking(&sync_sem);
    scheduler_core_1(schedule_table_core_1);

    // while (1)
    // {
    //     // printf("VTOR address core 1: %p\n", scb_hw->vtor);
    //     // sleep_ms_core_1(5*1000000);
    //     tight_loop_contents();
    // }
}

int main() {
    stdio_init_all();
    system_init();
    multicore_lauch_core1_separate_vtor_table(main_core_1, (uint32_t) vector_table_core_1);

    //wait slightly to ensure core 1 is def set up
    sleep_ms_core_0(500000);

    // fill the scheduling table for the core
    fill_schedule_core_0(schedule_table_core_0);

    // Enable the interrupt for our alarm
    // TODO: check if this is reset after the interrupt fires
    hw_set_bits(&timer_hw->inte, 1u << ALARM_CORE_0);

    // set interrupt handler
    irq_set_exclusive_handler(ALARM_IRQ_CORE_0, alarm_irq_core_0);

    // Enable the alarm irq, extract from irq_set_enable(reg, bool)
    *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET)) = alarm_mask_core_0;
    *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)) = alarm_mask_core_0;

    sem_release(&sync_sem);
    scheduler_core_0(schedule_table_core_0);
    return 0;
}



/*! @brief Global register and other system function initialization.
*   @category init functions
*
* Performs the following actions:
* - turns off XIP cache
* - sets up performance counter to count contested XIP accesses
*/
void system_init()
{
    /* XIP cache turn off */ 
    xip_ctrl_hw->ctrl = (xip_ctrl_hw->ctrl & ~XIP_CTRL_EN_BITS);

    /* Counter init to count contested accesses */ 
    bus_perf_0_hw->sel = arbiter_xip_main_perf_event_access_contested;

    /* Semaphore init */
    sem_init(&sync_sem, 0, 1);

    memcpy(vector_table_core_1, (const uint32_t *) scb_hw->vtor, sizeof(vector_table_core_1));

    vector_table_core_1[0] = 0x21040000;
}



/*! @brief Task scheduler for core 0
*   @category schedulers
*
* Scheduler is set to be local, thus, executed from SRAM0.
* Initialization routine contains table fill function (should maybe be moved before the scheduler, accept as parameter the schedule)
*/
void __core_0_code("sram_0")(scheduler_core_0)(table_entry_t table[])
{
    // reset the counter
    bus_perf_0_hw->value = 0;

    // Run the first task after specified time, the rest will be done within the handler
    uint64_t target = timer_hw->timerawl + table[0].scheduled_wait_time;
    // Write the lower 32 bits of the target time to the alarm which will arm it
    timer_hw->alarm[ALARM_CORE_0] = (uint32_t) target;

    while(1)
    {
        for(int i=0; i<TABLE_SIZE_CORE_0; i++)
        {
            while(!alarm_flag_core_0);
            alarm_flag_core_0 = false;
            table[i].task(table[i].subschedule);
        }

        printf("Perf counter in loop: %d\n\n", bus_perf_0_hw->value);
    }
}

/*! @brief Task scheduler for core 1
*   @category schedulers
*
* Scheduler is set to be local, thus, executed from SRAM2.
* Initialization routine contains table fill function (should maybe be moved before the scheduler, accept as parameter the schedule)
*/
void __core_1_code("sram_2")(scheduler_core_1)(table_entry_t table[])
{
    uint64_t target = timer_hw->timerawl + table[0].scheduled_wait_time;

    // Write the lower 32 bits of the target time to the alarm which will arm it
    timer_hw->alarm[ALARM_CORE_1] = (uint32_t) target;

    while(1)
    {
        // // pure ram code
        // int a = 5;
        // for (int i = 0; i< 1024; i++)
        // {
        //     a+=(i+2);
        // }
        // a--;
        // sleep_ms_core_1(5000);
        for(int i=0; i<TABLE_SIZE_CORE_1; i++)
        {
            while(!alarm_flag_core_1);
            alarm_flag_core_1 = false;
            table[i].task(table[i].subschedule);
        }
    }
}

void __core_0_code("irq")(alarm_irq_core_0)(void) {
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_CORE_0);

    static int task_counter = 0;

    // Assume alarm 0 has fired
    // printf("Alarm IRQ CORE 0 fired, ts: %d\n", task_counter);

    alarm_flag_core_0 = true;

    if(task_counter < (TABLE_SIZE_CORE_0 - 1) )
    {
        task_counter++;
    }
    else
    {
        task_counter = 0;
    }

    uint64_t target = timer_hw->timerawl + schedule_table_core_0[task_counter].scheduled_wait_time;

    // Write the lower 32 bits of the target time to the alarm which will arm it
    timer_hw->alarm[ALARM_CORE_0] = (uint32_t) target;
}

static void __core_1_code("irq")(alarm_irq_core_1)(void) {
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_CORE_1);

    static int task_counter = 0;


    // Assume alarm 1 has fired
    // printf("Alarm IRQ CORE 1 fired, ts: %d\n", task_counter);
    alarm_flag_core_1 = true;

    if(task_counter < (TABLE_SIZE_CORE_1 - 1) )
    {
        task_counter++;
    }
    else
    {
        task_counter = 0;
    }

    uint64_t target = timer_hw->timerawl + schedule_table_core_1[task_counter].scheduled_wait_time;

    // Write the lower 32 bits of the target time to the alarm which will arm it
    timer_hw->alarm[ALARM_CORE_1] = (uint32_t) target;
}