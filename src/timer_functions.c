#include "hardware/structs/timer.h"
#include "timer_functions.h"

void __core_0("time")(busy_wait_until_core_0)(uint64_t target) {
    uint32_t hi_target = (uint32_t)(target >> 32u);
    uint32_t hi = timer_hw->timerawh;
    while (hi < hi_target) {
        hi = timer_hw->timerawh;
        tight_loop_contents();
    }
    while (hi == hi_target && timer_hw->timerawl < (uint32_t) target) {
        hi = timer_hw->timerawh;
        tight_loop_contents();
    }
}

/* Returns a timestamp , check func call */
uint64_t __core_0("time")(timestamp_core_0)() {
    // Need to make sure that the upper 32 bits of the timer
    // don't change, so read that first
    uint32_t hi = timer_hw->timerawh;
    uint32_t lo;
    while (true)
    {
        // Read the lower 32 bits
        lo = timer_hw->timerawl;
        // Now read the upper 32 bits again and
        // check that it hasn't incremented. If it has loop around
        // and read the lower 32 bits again to get an accurate value
        uint32_t next_hi = timer_hw->timerawh;
        if (hi == next_hi) break;
        hi = next_hi;
    }
    return ((uint64_t) hi << 32u) | lo;
}

void __core_0("time")(sleep_ms_core_0)(uint64_t us)
{
    uint64_t current_timestamp = timestamp_core_0();
    uint64_t target = current_timestamp + us;
    busy_wait_until_core_0(target);
}

void __core_1("time")(busy_wait_until_core_1)(uint64_t target) {
    uint32_t hi_target = (uint32_t)(target >> 32u);
    uint32_t hi = timer_hw->timerawh;
    while (hi < hi_target) {
        hi = timer_hw->timerawh;
        tight_loop_contents();
    }
    while (hi == hi_target && timer_hw->timerawl < (uint32_t) target) {
        hi = timer_hw->timerawh;
        tight_loop_contents();
    }
}

/* Returns a timestamp , check func call */
uint64_t __core_1("time")(timestamp_core_1)() {
    // Need to make sure that the upper 32 bits of the timer
    // don't change, so read that first
    uint32_t hi = timer_hw->timerawh;
    uint32_t lo;
    do {
        // Read the lower 32 bits
        lo = timer_hw->timerawl;
        // Now read the upper 32 bits again and
        // check that it hasn't incremented. If it has loop around
        // and read the lower 32 bits again to get an accurate value
        uint32_t next_hi = timer_hw->timerawh;
        if (hi == next_hi) break;
        hi = next_hi;
    } while (true);
    return ((uint64_t) hi << 32u) | lo;
}

void __core_1("time")(sleep_ms_core_1)(uint64_t us)
{
    uint64_t current_timestamp = timestamp_core_1();
    uint64_t target = current_timestamp + us;
    busy_wait_until_core_1(target);
}