#pragma once

#include <stdint.h>

#include "event.h"

typedef struct timer {
    // the event of this timer
    event_t event;

    // the link in the timer list
    list_entry_t link;

    // when should the timer be triggered
    uint64_t trigger_time;

    // the interval of the timer
    uint64_t interval;
} timer_t;

/**
 * Set the timer as a single shot timer
 */
void timer_set_timeout(timer_t* timer, size_t timeout);

/**
 * Set the timer as an interval timer
 */
void timer_set_interval(timer_t* timer, size_t interval);

/**
 * Cancel the timer
 */
void timer_cancel(timer_t* timer);

/**
 * Delays the execution by the given amount of
 * microseconds, meant for drivers to do small waits
 */
void delay(size_t ms);
