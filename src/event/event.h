#pragma once

#include <stddef.h>
#include <stdint.h>
#include "tpl.h"
#include "util/list.h"

typedef struct event event_t;

typedef void (*event_notify_t)(event_t* event, void* context);

typedef enum event_type {
    /**
     * has notification function that can be waited on with check_event or
     * with wait_for_events
     */
    EVENT_NOTIFY_WAIT,

    /**
     * has notification function that is queued when the event is
     * signaled with signal_event
     */
    EVENT_NOTIFY_SIGNAL
} event_type_t;

struct event {
    // is this event signalled
    uint32_t signal_count;

    // the type of this event
    event_type_t type;

    // the notification function and its context
    tpl_t notify_tpl;
    event_notify_t notify_function;
    void* notify_context;

    // entry in a list of notifications
    list_entry_t notify_link;
};

/**
 * Signal the given event, possibly queueing its notification function.
 *
 * Can be called from any tpl
 */
void signal_event(event_t* event);

/**
 * Check if the event if signalled, if the event is a wait event then it's
 * notification will first be called, and then the event will be checked if
 * its signalled
 *
 * Can be called from any tpl
 */
bool check_event(event_t* event);


/**
 * Wait for an event to be ready. Returns the index
 * of the event that was signalled
 *
 * Can only be called from application tpl
 */
size_t wait_for_event(size_t event_count, event_t* events);

/**
 * Close the event, removing it from all the queues
 */
void close_event(event_t* event);
