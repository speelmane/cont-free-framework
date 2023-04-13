
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

/* 
    Alarm setup
*/
#define ALARM_CORE_0 0
#define ALARM_CORE_1 1
#define ALARM_IRQ_CORE_0 TIMER_IRQ_0
#define ALARM_IRQ_CORE_1 TIMER_IRQ_1

/* By default the cores should wait until the flag is cleared */
__core_0_data("flag") char alarm_flag_core_0 = 0;
__core_1_data("flag") char alarm_flag_core_1 = 0;

__core_0_data("table") table_entry_t schedule_table_core_0[TABLE_SIZE];
__core_1_data("table") table_entry_t schedule_table_core_1[TABLE_SIZE];

__core_1_data("vtor") uint32_t core1_vector_table[32] = {0};

/*
    A semaphore for task synchronization
*/
semaphore_t sync_sem;

static void __core_0_code("irq")(alarm_irq_core_0)(void);

static void __core_1_code("irq")(alarm_irq_core_1)(void);

static void __core_0_code("alarm")(alarm_setup_core_0)(uint32_t delay_us);

static void __core_1_code("alarm")(alarm_setup_core_1)(uint32_t delay_us);

void __core_1_code("sram_2")(scheduler_core_1)(table_entry_t table[]);

void __core_0_code("sram_0")(scheduler_core_0)(table_entry_t table[]);

void core1_entry(void);

void system_init();


int main() {
    stdio_init_all();
    system_init();
    // multicore_launch_core1(core1_entry);
    multicore_lauch_core1_separate_vtor_table(core1_entry, &core1_vector_table);

    while (1)
    {
        printf("VTOR address core 0: %p\n", scb_hw->vtor);
        sleep_ms_core_0(5*1000000);
    }
    
    // //wait slightly to ensure core 1 is def set up
    // sleep_ms_core_0(500000);

    // // fill the scheduling table for the core
    // fill_schedule_core_0(schedule_table_core_0);

    // sem_release(&sync_sem);
    // scheduler_core_0(schedule_table_core_0);
    return 0;
}

/*! @brief Core 1 entry point
*   @category core 1
*/
void core1_entry(void)
{
    // // fill the scheduling table for the core
    // fill_schedule_core_1(schedule_table_core_1);
    // sem_acquire_blocking(&sync_sem);
    // scheduler_core_1(schedule_table_core_1);

    while (1)
    {
        printf("VTOR address core 1: %p\n", scb_hw->vtor);
        sleep_ms_core_1(5*1000000);
    }
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

    /* Set up timer alarms for both cores */
    /**
     * 
     * 
     * add stuff here
     * 
     * 
     * 
    */
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

/*! @brief Task scheduler for core 1
*   @category schedulers
*
* Scheduler is set to be local, thus, executed from SRAM2.
* Initialization routine contains table fill function (should maybe be moved before the scheduler, accept as parameter the schedule)
*/
void __core_1_code("sram_2")(scheduler_core_1)(table_entry_t table[])
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


static void __core_0_code("alarm")(alarm_setup_core_0)(uint32_t delay_us) {
    // Enable the interrupt for our alarm (the timer outputs 4 alarm irqs)
    hw_set_bits(&timer_hw->inte, 1u << ALARM_CORE_0);
    // Set irq handler for alarm irq
    irq_set_exclusive_handler(ALARM_IRQ_CORE_0, alarm_irq_core_0);
    // Enable the alarm irq
    irq_set_enabled(ALARM_IRQ_CORE_0, true);
    // Enable interrupt in block and at processor

    // Alarm is only 32 bits so if trying to delay more
    // than that need to be careful and keep track of the upper
    // bits
    uint64_t target = timer_hw->timerawl + delay_us;

    // Write the lower 32 bits of the target time to the alarm which
    // will arm it
    timer_hw->alarm[ALARM_CORE_0] = (uint32_t) target;
}

static void __core_1_code("alarm")(alarm_setup_core_1)(uint32_t delay_us) {
    // Enable the interrupt for our alarm (the timer outputs 4 alarm irqs)
    hw_set_bits(&timer_hw->inte, 1u << ALARM_CORE_1);
    // Set irq handler for alarm irq
    irq_set_exclusive_handler(ALARM_IRQ_CORE_1, alarm_irq_core_1);
    // Enable the alarm irq
    /* TODO: CHANGE TO THE INLINED STATEMENTS (NOT POSSIBLE WITH )*/
    irq_set_enabled(ALARM_IRQ_CORE_1, true);
    // Enable interrupt in block and at processor

    // Alarm is only 32 bits so if trying to delay more
    // than that need to be careful and keep track of the upper
    // bits
    uint64_t target = timer_hw->timerawl + delay_us;

    // Write the lower 32 bits of the target time to the alarm which
    // will arm it
    timer_hw->alarm[ALARM_CORE_1] = (uint32_t) target;
}

static void __core_0_code("irq")(alarm_irq_core_0)(void) {
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_CORE_0);

    // Assume alarm 0 has fired
    printf("Alarm IRQ CORE 0 fired\n");
    alarm_flag_core_0 = true;
}

static void __core_1_code("irq")(alarm_irq_core_1)(void) {
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_CORE_1);

    // Assume alarm 1 has fired
    printf("Alarm IRQ CORE 1 fired\n");
    alarm_flag_core_1 = true;
}