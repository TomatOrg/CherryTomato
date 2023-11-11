#pragma once

#include "event.h"

typedef enum timer_type {
    /**
     * A periodic timer that will run every some time
     */
    TIMER_PERIODIC,

    /**
     * One-shot timer that will run in some time
     */
    TIMER_RELATIVE,
} timer_type_t;

typedef struct timer {
    // the event related to this timer
    event_t event;

    // link to the list of timers
    list_t link;

    // the trigger time for the timer
    uint64_t trigger_time;

    // the period of the timer
    uint64_t period;
} timer_t;

/**
 * Gets the current system time, synchronized with timer code
 */
uint64_t get_current_system_time();

/**
 * Set a timer on an event timer
 */
void set_timer(timer_t* timer, timer_type_t type, uint64_t trigger_time);

/**
 * Cancel the given timer
 */
void cancel_timer(timer_t* timer);

/**
 * Called by the platform to update the timers
 */
void timer_tick(uint64_t duration);
