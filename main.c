#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include "hardware/regs/xip.h"
#include "hardware/regs/busctrl.h"
#include "pico/multicore.h"
#include "include/common.h"
#include "include/task1.h"

#define PERF_COUNTER_0 (BUSCTRL_BASE + BUSCTRL_PERFCTR0_OFFSET)
#define __core_0(group) __attribute__((section(".core_0." group)))
#define __core_1(group) __attribute__((section(".core_1." group)))
#define __runtime_copy(group) __attribute__((section(".runtime_copy." group)))


// #define __CORE1_DATA __attribute__((__section__(".core1_data")))
// #define __CORE0_DATA __attribute__((__section__(".core0_data")))

// Placed in RAM, avoid such declarations
int32_t global_int_define_uninitialized;

// Placed in FLASH
const int32_t global_int_define_initialized = 123456;

int __runtime_copy("runtime_copy")(system_test_call)()
{
    int i = 0;
    i = i + 2;
    i--;
    return i;
}

int __core_1("sram_2")(system_init_core_1)()
{
    int i = 0;
    // Disable XIP caches
    io_rw_32 * xip_ctrl_reg = (io_rw_32 *) XIP_CTRL_BASE + XIP_CTRL_OFFSET;
    * xip_ctrl_reg = (* xip_ctrl_reg & ~XIP_CTRL_EN_BITS);

    // Set up scheduling tables
    i++;
    int ai = i * 3;

// array cause can refer to it as an address?

    return ai;
}

int __core_0("sram_0")(system_init_core_0)()
{
    int i = 0;
    // Disable XIP caches
    io_rw_32 * xip_ctrl_reg = (io_rw_32 *) XIP_CTRL_BASE + XIP_CTRL_OFFSET;
    * xip_ctrl_reg = (* xip_ctrl_reg & ~XIP_CTRL_EN_BITS);

    // Set up scheduling tables
    i++;
    int ai = i * 3;


    // i = system_test_call();
    return ai;
}

int __scratch_x("sram_4")(system_init_x)()
{
    int i = 0;
    // Disable XIP caches
    io_rw_32 * xip_ctrl_reg = (io_rw_32 *) XIP_CTRL_BASE + XIP_CTRL_OFFSET;
    * xip_ctrl_reg = (* xip_ctrl_reg & ~XIP_CTRL_EN_BITS);

    // Set up scheduling tables
    i++;

    int ai = i * 3;

    return ai;
}

void core1_entry(void)
{
    int32_t core1_local_variable_initialized[20] = {0};
    int i = 0;

    int static_int_core1= 56;
    int another_static_int_core1= 57;


    while(1)
    {
        static_int_core1++;

        for(i; i<20; i++)
        {
            another_static_int_core1++;
            core1_local_variable_initialized[i]++;
        }

        // printf("Static int core 1 address: %p, value: %d\n", &static_int_core1, static_int_core1);
        // printf("Local var itialized core 1 address: %p\n", core1_local_variable_initialized);
        sleep_ms(10);
    }
}


int main() {
    stdio_init_all();

    // On stack
    int32_t core0_local_variable_uninitialized = 0;

    // On stack
    int32_t core0_local_variable_initialized[20] = {0xFAFAFAFA};

    int static_int_core0= 56;
    int another_static_int_core0= 57;

    // set up the counter to count contested XIP accesses
    io_rw_32 * perf_counter0_sel = (io_wo_32 *) (BUSCTRL_BASE + BUSCTRL_PERFSEL0_OFFSET);
    * perf_counter0_sel = 0x00000000 | BUSCTRL_PERFSEL0_VALUE_XIP_MAIN_CONTESTED;

    io_rw_32 * perf_counter = (io_rw_32 *) PERF_COUNTER_0;
   
    int sys_x = system_init_x();
    int sys_0 = system_init_core_0();
    int sys_1 = system_init_core_1();

    printf("X: %d, 0: %d, 1: %d \n", sys_x, sys_0, sys_1);

    int cpu_id = get_core_num();

    subschedule_t subschedule_core_0 = {
        .cpu_id = cpu_id,
        .r_to_e_wait_time = 10,
        .e_to_w_wait_time = 10
    };
    // multicore_launch_core1(core1_entry);

    // task1(subschedule_core_0);

    int32_t local_data;
    static_int_core0++;
    another_static_int_core0++;
    printf("Size of global int: %d\n", sizeof(global_int_define_initialized));
    memcpy(&local_data, &global_int_define_initialized, sizeof(global_int_define_initialized));
    core0_local_variable_initialized[0]++;

    extern int __stack[],__runtime_copy_start__[],__runtime_copy_end__[], __core_1_end__[];
    printf("Stack address top: %p\n", __stack);
    printf("Runtime copy start: %p\n", __runtime_copy_start__);
    printf("Runtime copy end: %p\n", __runtime_copy_end__);
    printf("Core 1 end: %p\n", __core_1_end__);



    int size = (int) (__runtime_copy_end__) - (int)(__runtime_copy_start__);
    printf("Size: %x\n", size);

    int (* ram_func)(void) = (memcpy(__core_1_end__, __runtime_copy_start__, size) + 1);

    printf("Regular sys_init_call: %p\n", system_init_core_1);
    printf("Address of sys_init_call: %p\n", &system_init_core_1);
    printf("Dereference sys_init_call: %p\n", *system_init_core_1);

    printf("Regular func pointer: %p\n", ram_func);
    printf("Dereference a func pointer: %p\n", *ram_func);
    printf("Address of func pointer: %p\n", &ram_func);



    int i = ram_func();


    while(1)
    {
        

        printf("Copied int address: %p, value: %d\n", &local_data, local_data);
        printf("Static int core 0 address: %p, value: %d\n", &static_int_core0, static_int_core0);

        printf("Global int unitialized address: %p\n", &global_int_define_uninitialized);
        printf("Global int initialized address: %p\n", &global_int_define_initialized);
        printf("Local var unitialized address: %p\n", &core0_local_variable_uninitialized);
        printf("Local var initialized address: %p\n", &core0_local_variable_initialized);

        printf("Address of sys init x func: %p\n", &system_init_x);
        printf("Address of sys init core 0 func: %p\n", &system_init_core_0);
        printf("Address of sys init core 1 func: %p\n", &system_init_core_1);



        sleep_ms(5000);
    } 

    return 0;
}

