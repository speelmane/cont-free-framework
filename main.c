#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include "hardware/regs/xip.h"
#include "hardware/regs/busctrl.h"
#include "pico/multicore.h"
#include "include/common.h" //subschedule
#include "timer_functions.h" //timer_funcs

#define PERF_COUNTER_0 (BUSCTRL_BASE + BUSCTRL_PERFCTR0_OFFSET)
#define __core_0(group) __attribute__((section(".core_0." group)))
#define __core_1(group) __attribute__((section(".core_1." group)))

void __core_1("sram_2")(scheduler_core_1)()
{
    // subschedule_t subschedule_core_0 = {
    //     .cpu_id = 1,
    //     .r_to_e_wait_time = 10,
    //     .e_to_w_wait_time = 10
    // };
    
    // // fill the table

    // //sync primitive
    // while(1)
    // {
    //     task1(subschedule_core_0);
    //     sleep_ms(10000);
    // } 

    // TMP PURE RAM ONLY CODE
    while(1)
    {
        int a = 5;
        for (int i = 0; i< 1024; i++)
        {
            a+=(i+2);
        }
        a--;
    }
}

void __core_0("sram_0")(scheduler_core_0)()
{
    io_rw_32 * perf_counter = (io_rw_32 *) PERF_COUNTER_0;
    *perf_counter = 0;

    subschedule_t subschedule_core_0 = {
        .cpu_id = 0,
        .r_to_e_wait_time = 10,
        .e_to_w_wait_time = 10
    };

    // fill the table

    //sync primitive

    while(1)
    {
        task1(subschedule_core_0);
        uint64_t timestamp_before = timestamp_core_0();
        sleep_ms_core_0(10000);
        uint64_t timestamp_after = timestamp_core_0();
        printf("Timestamp before: %lli, after: %lli\n", timestamp_before, timestamp_after);
        printf("Address of sleep ms core 0: %p, busy wait: %p and ts_get: %p\n", sleep_ms_core_0, busy_wait_until_core_0, timestamp_core_0);
        printf("Perf counter in loop: %d\n\n", *perf_counter);
    }
}

void core1_entry(void)
{
    scheduler_core_1();
}

/*! @brief Global system initialization function.
* Performs the following actions:
* - turns off XIP cache
* - sets up performance counter to count contested XIP accesses
*/
void system_init()
{
    /* XIP cache turn off */ 
    io_rw_32 * xip_ctrl_reg = (io_rw_32 *) XIP_CTRL_BASE + XIP_CTRL_OFFSET;
    * xip_ctrl_reg = (* xip_ctrl_reg & ~XIP_CTRL_EN_BITS);
    /* XIP cache turn off end */ 

    /* Counter init to count contested XIP accesses */ 
    io_rw_32 * perf_counter0_sel = (io_wo_32 *) (BUSCTRL_BASE + BUSCTRL_PERFSEL0_OFFSET);
    * perf_counter0_sel = 0x00000000 | BUSCTRL_PERFSEL0_VALUE_XIP_MAIN_CONTESTED;
    /* Counter init end*/
}

int main() {
    stdio_init_all();
    system_init();
    // multicore_launch_core1(core1_entry);
    scheduler_core_0();
    return 0;
}

