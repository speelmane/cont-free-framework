
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

/* Bus performance counters, nr 0 reserved for XIP access counting, the rest can verify other contention sources if that is the wish */
#define bus_perf_0_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR0_OFFSET))
#define bus_perf_1_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR1_OFFSET))
#define bus_perf_2_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR2_OFFSET))
#define bus_perf_3_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR3_OFFSET))





__core_0_data("table") table_entry_t schedule_table_core_0[TABLE_SIZE];
__core_1_data("table") table_entry_t schedule_table_core_1[TABLE_SIZE];

/*
    Define semaphores used in tasks
*/
/* in SRAM1 but only accessed before the schedule so no contensted accesses expected */
semaphore_t sync_sem;

// semaphore_t task_sem_core_0;
// semaphore_t task_sem_core_1;



/*! @brief Task scheduler for core 1
*   @category schedulers
*
* Scheduler is set to be local, thus, executed from SRAM2.
* Initialization routine contains table fill function (should maybe be moved before the scheduler, accept as parameter the schedule)
*/
void __core_1_code("sram_2")(scheduler_core_1)()
{
    while(1)
    {
        int a = 5;
        for (int i = 0; i< 1024; i++)
        {
            a+=(i+2);
        }
        a--;
        sleep_ms_core_1(5000);

    }
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

    while(1)
    {
        for(int i=0; i<TABLE_SIZE; i++)
        {
            sleep_ms_core_0(table[i].scheduled_wait_time);
            table[i].task(table[i].subschedule);
        }

        printf("Perf counter in loop: %d\n\n", bus_perf_0_hw->value);
    }
}

void core1_entry(void)
{
    // fill the scheduling table for the core
    fill_schedule_core_1(schedule_table_core_1);

    sem_acquire_blocking(&sync_sem);
    scheduler_core_1();
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
}

int main() {
    stdio_init_all();
    system_init();
    multicore_launch_core1(core1_entry);

    //wait slightly to ensure core 1 is def set up
    sleep_ms_core_0(500000);

    // fill the scheduling table for the core
    fill_schedule_core_0(schedule_table_core_0);

    sem_release(&sync_sem);
    scheduler_core_0(schedule_table_core_0);
    return 0;
}

