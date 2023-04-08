#include "hardware/structs/timer.h"

#define __core_0(group) __attribute__((section(".core_0." group)))
#define __core_1(group) __attribute__((section(".core_1." group)))

void __core_0("time")(busy_wait_until_core_0)(uint64_t target);

/* Returns a timestamp , check func call */
uint64_t __core_0("time")(timestamp_core_0)();

void __core_0("time")(sleep_ms_core_0)(uint64_t ms);