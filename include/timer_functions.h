#ifndef TIMER_FUNCTIONS /* TIMER FUNCTIONS */
#define TIMER FUNCTIONS
#include "hardware/structs/timer.h"
#include "common.h"


/* Function prototypes */
void __core_0_code("time")(busy_wait_until_core_0)(uint64_t target);

uint64_t __core_0_code("time")(timestamp_core_0)();

void __core_0_code("time")(sleep_ms_core_0)(uint64_t us);

void __core_1_code("time")(busy_wait_until_core_1)(uint64_t target);

uint64_t __core_1_code("time")(timestamp_core_1)();

void __core_1_code("time")(sleep_ms_core_1)(uint64_t us);

#endif /* TIMER FUNCTIONS */