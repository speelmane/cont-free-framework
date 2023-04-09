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

#define __core_0(group) __attribute__((section(".core_0." group)))
#define __core_1(group) __attribute__((section(".core_1." group)))

/* Bus performance counters, nr 0 reserved for XIP access counting, the rest can verify other contention sources if that is the wish */
#define bus_perf_0_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR0_OFFSET))
#define bus_perf_1_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR1_OFFSET))
#define bus_perf_2_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR2_OFFSET))
#define bus_perf_3_hw ((bus_ctrl_perf_hw_t *)(BUSCTRL_BASE + BUSCTRL_PERFCTR3_OFFSET))

/* in SRAM1 but only accessed before the schedule so no contensted accesses expected */
semaphore_t sync_sem;

/*! @brief Task scheduler for core 1
*   @category schedulers
*
* Scheduler is set to be local, thus, executed from SRAM2.
* Initialization routine contains table fill function (should maybe be moved before the scheduler, accept as parameter the schedule)
*/
void __core_1("sram_2")(scheduler_core_1)()
{
    // extern char __core_1_end__[];

    // subschedule_t subschedule_core_1 = {
    //     .cpu_id = 1,
    //     .timestamp_func = timestamp_core_1,
    //     .sleep_func = sleep_ms_core_1,
    //     .exec_copy_func_dst = __core_1_end__,
    //     .r_to_e_wait_time = 10,
    //     .e_to_w_wait_time = 10
    // };

    // // fill the table

    // while(1)
    // {
    //     task1(subschedule_core_1);
    //     sleep_ms_core_1(10000);
    // } 

    // // TMP PURE RAM ONLY CODE
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
void __core_0("sram_0")(scheduler_core_0)()
{
    // reset the counter
    bus_perf_0_hw->value = 0;

    extern char __core_0_end__[];

    subschedule_t subschedule_core_0 = {
        .cpu_id = 0,
        .timestamp_func = timestamp_core_0,
        .sleep_func = sleep_ms_core_0,
        .exec_copy_func_dst = __core_0_end__,
        .r_to_e_wait_time = 1000000,
        .e_to_w_wait_time = 1000000
    };

    // fill the table

    while(1)
    {
        task1(subschedule_core_0);
        uint64_t timestamp_before = timestamp_core_0();
        sleep_ms_core_0(10000000);
        printf("Perf counter after sleeps: %d\n\n", bus_perf_0_hw->value);
        uint64_t timestamp_after = timestamp_core_0();
        printf("Timestamp before: %lli, after: %lli\n", timestamp_before, timestamp_after);
        printf("Perf counter in loop: %d\n\n", bus_perf_0_hw->value);
    }
}

void core1_entry(void)
{
    // fill the table here, pass in table as parameter to scheduler_core_1()
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
    sem_release(&sync_sem);
    printf("Perf counter before scheduler, not cleared ever: %d\n", bus_perf_0_hw->value);
    // fill the table here, pass in table as parameter to scheduler_core_0()
    scheduler_core_0();
    return 0;
}

