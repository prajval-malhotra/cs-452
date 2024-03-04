#include "system_timer.h"
#include "rpi.h"

static constexpr uint32_t TIMER_TICK = 10000; // in us, amounts to 10ms

void reset_timer(uint32_t timer) {
    TIMER_REG(timer) = 0u;
    TIMER_REG(TIMER_CS) = 1u << ((timer - TIMER_C0) >> 2); // either 1, 2, 4, or 8 depending on the timer
}

static uint32_t timer_tick_targets[4] = {0, 0, 0, 0};

void set_timer(uint32_t timer) {
    // timer register should be one of TIMER_C0, C1, C2, or C3
    size_t timer_ind = (timer - TIMER_C0) >> 2;
    if (timer_tick_targets[timer_ind] == 0) {
        timer_tick_targets[timer_ind] = get_time();
    }
    timer_tick_targets[timer_ind] += TIMER_TICK;
    TIMER_REG(timer) = timer_tick_targets[timer_ind];
}
