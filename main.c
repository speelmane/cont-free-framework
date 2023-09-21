
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
#include "pico/platform.h"
#include "hardware/structs/scb.h"
#include "schedule.h"


/* Bus performance counters, nr 0 reserved for XIP contested accesses and nr 1 for SRAM_5 contested accesses, the rest can verify other contention sources if that is the wish */
#define bus_perf_0_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR0_OFFSET))
#define bus_perf_1_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR1_OFFSET))
#define bus_perf_2_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR2_OFFSET))
#define bus_perf_3_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR3_OFFSET))

/* Alarm setup */
#define ALARM_TASK_RELEASE_CORE_0 0
#define ALARM_TASK_RELEASE_CORE_1 1
#define ALARM_WRITE_RELEASE_CORE_0 2
#define ALARM_WRITE_RELEASE_CORE_1 3

#define ALARM_TASK_IRQ_CORE_0 TIMER_IRQ_0
#define ALARM_TASK_IRQ_CORE_1 TIMER_IRQ_1
#define ALARM_WRITE_IRQ_CORE_0 TIMER_IRQ_2
#define ALARM_WRITE_IRQ_CORE_1 TIMER_IRQ_3

/* By default the cores should wait until the flag is cleared */
__core_0_data("flag") char volatile task_release_flag_core_0 = false;
__core_1_data("flag") char volatile task_release_flag_core_1 = false;

__core_0_data("writerelease") char volatile write_release_flag_core_0 = false;
__core_1_data("writerelease") char volatile write_release_flag_core_1 = false;

__core_0_data("counter") int task_counter_0 = 0;
__core_1_data("counter") int task_counter_1 = 0;

__core_0_data("loops") uint64_t volatile target_task_release_core_0 = SCHEDULE_OFFSET + START_OFFSET_CORE_0;
__core_1_data("loops") uint64_t volatile target_task_release_core_1 = SCHEDULE_OFFSET + START_OFFSET_CORE_1;

__core_0_data("writes") uint64_t volatile target_write_release_core_0 = 0; //TODO: set this to whatever the first shift is going to be, ABSOLUTE TIME
__core_1_data("writes") uint64_t volatile target_write_release_core_1 = 0; //TODO: set this to whatever the first shift is going to be, ABSOLUTE TIME

__core_0_data("table") table_entry_t schedule_table_core_0[TABLE_SIZE_CORE_0];
__core_1_data("table") table_entry_t schedule_table_core_1[TABLE_SIZE_CORE_1];


/*! VTOR table for core 1, content invalid, content to be initialized in @ref system_init*/
uint32_t __attribute__((section(".ram_vector_table_core_1"))) vector_table_core_1[48] = {0};

/* Function prototypes */
static void __core_0_code("irq")(alarm_task_irq_core_0)(void);
static void __core_1_code("irq")(alarm_task_irq_core_1)(void);
static void __core_0_code("irq")(alarm_write_irq_core_0)(void);
static void __core_1_code("irq")(alarm_write_irq_core_1)(void);

void __core_0_code("scheduler")(scheduler_core_0)(table_entry_t table[]);
void __core_1_code("scheduler")(scheduler_core_1)(table_entry_t table[]);

void system_init();

/* End of function prototypes */

/*! @brief Core 1 entry point
*   @category core 1
*/
void main_core_1(void)
{
    fill_schedule_core_1(schedule_table_core_1);

    /* Interrupt setup routine */
    // task release interrupt
    hw_set_bits(&timer_hw->inte, 1u << ALARM_TASK_RELEASE_CORE_1);
    irq_set_exclusive_handler(ALARM_TASK_IRQ_CORE_1, alarm_task_irq_core_1);
    irq_set_enabled(ALARM_TASK_IRQ_CORE_1, true);

    // write release interrupt
    hw_set_bits(&timer_hw->inte, 1u << ALARM_WRITE_RELEASE_CORE_1);
    irq_set_exclusive_handler(ALARM_WRITE_IRQ_CORE_1, alarm_write_irq_core_1);
    irq_set_enabled(ALARM_WRITE_IRQ_CORE_1, true);
    /* End of interrupt setup routine */

    scheduler_core_1(schedule_table_core_1);
}

int main() {
    stdio_init_all();
    system_init();
    multicore_lauch_core1_separate_vtor_table(main_core_1, (uint32_t) vector_table_core_1);

    //wait slightly to ensure core 1 is def set up
    sleep_ms_core_0(100000);

    fill_schedule_core_0(schedule_table_core_0);

    /* Interrupt setup routine */
    // task release interrupt
    hw_set_bits(&timer_hw->inte, 1u << ALARM_TASK_RELEASE_CORE_0);
    irq_set_exclusive_handler(ALARM_TASK_IRQ_CORE_0, alarm_task_irq_core_0);
    irq_set_enabled(ALARM_TASK_IRQ_CORE_0, true);

    // write release interrupt
    hw_set_bits(&timer_hw->inte, 1u << ALARM_WRITE_RELEASE_CORE_0);
    irq_set_exclusive_handler(ALARM_WRITE_IRQ_CORE_0, alarm_write_irq_core_0);
    irq_set_enabled(ALARM_WRITE_IRQ_CORE_0, true);
    /* End of interrupt setup routine */

    scheduler_core_0(schedule_table_core_0);
    return 0;
}



/*! @brief Global register and other system function initialization.
*   @category init functions
*
* Performs the following actions:
* - turns off XIP cache
* - sets up performance counters to count contested XIP/SRAM_5 accesses
* - sets the core1 VTOR table to valid content
*/
void system_init()
{
    /* XIP cache turn off */ 
    xip_ctrl_hw->ctrl = (xip_ctrl_hw->ctrl & ~XIP_CTRL_EN_BITS);

    /* Counter init to count contested accesses */ 
    bus_perf_0_hw->sel = arbiter_sram4_perf_event_access_contested;
    bus_perf_1_hw->sel = arbiter_sram5_perf_event_access_contested;
    bus_perf_2_hw->sel = arbiter_xip_main_perf_event_access_contested;
    bus_perf_3_hw->sel = arbiter_rom_perf_event_access_contested;


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
    timer_hw->alarm[ALARM_TASK_RELEASE_CORE_0] = (uint32_t) target_task_release_core_0;

    // reset the counter
    bus_perf_0_hw->value = 0;
    bus_perf_1_hw->value = 0;
    bus_perf_2_hw->value = 0;
    bus_perf_3_hw->value = 0;


    while(1)
    {
        for(int i=0; i<TABLE_SIZE_CORE_0; i++)
        {
            while(!task_release_flag_core_0);
            task_release_flag_core_0 = false;
            table[i].subschedule.write_release_flag = &write_release_flag_core_0;
            table[i].task(table[i].subschedule);
        }
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
    timer_hw->alarm[ALARM_TASK_RELEASE_CORE_1] = (uint32_t) target_task_release_core_1;

    // reset the counter
    bus_perf_0_hw->value = 0;
    bus_perf_1_hw->value = 0;
    bus_perf_2_hw->value = 0;
    bus_perf_3_hw->value = 0;

    while(1)
    {
        for(int i=0; i<TABLE_SIZE_CORE_1; i++)
        {
            while(!task_release_flag_core_1);
            task_release_flag_core_1 = false;
            table[i].subschedule.write_release_flag = &write_release_flag_core_1;
            table[i].task(table[i].subschedule);
        }
    }
}

void __core_0_code("irq")(alarm_task_irq_core_0)(void) {
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_TASK_RELEASE_CORE_0);
    task_release_flag_core_0 = true;

    // schedule write release - CURRENT RELEASE + WRITE RELEASE OFFSET
    target_write_release_core_0 = target_task_release_core_0 + schedule_table_core_0[task_counter_0].write_release_offset;
    timer_hw->alarm[ALARM_WRITE_RELEASE_CORE_0] = (uint32_t) target_write_release_core_0;

    if(task_counter_0 < (TABLE_SIZE_CORE_0 - 1) )
    {
        task_counter_0++;
    }
    else
    {
        task_counter_0 = 0;
    }

    // schedule next task release
    target_task_release_core_0 += schedule_table_core_0[task_counter_0].task_release_offset;

    // Write the lower 32 bits of the target time to the alarm which will arm it
    timer_hw->alarm[ALARM_TASK_RELEASE_CORE_0] = (uint32_t) target_task_release_core_0;

}

void __core_1_code("irq")(alarm_task_irq_core_1)(void) {
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_TASK_RELEASE_CORE_1);
    task_release_flag_core_1 = true;

    // schedule write release - CURRENT RELEASE + WRITE RELEASE OFFSET
    target_write_release_core_1 = target_task_release_core_1 + schedule_table_core_1[task_counter_1].write_release_offset;
    timer_hw->alarm[ALARM_WRITE_RELEASE_CORE_1] = (uint32_t) target_write_release_core_1;
    


    if(task_counter_1 < (TABLE_SIZE_CORE_1 - 1) )
    {
        task_counter_1++;
    }
    else
    {
        task_counter_1 = 0;
    }

    // schedule next task release
    target_task_release_core_1 += schedule_table_core_1[task_counter_1].task_release_offset;

    // Write the lower 32 bits of the target time to the alarm which will arm it
    timer_hw->alarm[ALARM_TASK_RELEASE_CORE_1] = (uint32_t) target_task_release_core_1;

}

void __core_0_code("irq")(alarm_write_irq_core_0)(void) {
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_WRITE_RELEASE_CORE_0);
    write_release_flag_core_0 = true;
}

void __core_1_code("irq")(alarm_write_irq_core_1)(void) {
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_WRITE_RELEASE_CORE_1);
    write_release_flag_core_1 = true;
}