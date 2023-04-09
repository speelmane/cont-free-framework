#ifndef TIMER_FUNCTIONS /* TIMER FUNCTIONS */
#define TIMER FUNCTIONS
#include "hardware/structs/timer.h"

#define __core_0(group) __attribute__((section(".core_0." group)))
#define __core_1(group) __attribute__((section(".core_1." group)))


/* Function prototypes */
void __core_0("time")(busy_wait_until_core_0)(uint64_t target);

uint64_t __core_0("time")(timestamp_core_0)();

void __core_0("time")(sleep_ms_core_0)(uint64_t us);

void __core_1("time")(busy_wait_until_core_1)(uint64_t target);

uint64_t __core_1("time")(timestamp_core_1)();

void __core_1("time")(sleep_ms_core_1)(uint64_t us);

#endif /* TIMER FUNCTIONS */