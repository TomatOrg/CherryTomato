#pragma once

#include "util/list.h"
#include "tpl.h"

/**
 * Forward declare
 */
typedef struct event event_t;

/**
 * The notification function, called when the event is notified
 */
typedef void (*event_notify_t)(event_t* event, void* context);

struct event {
    // notification context
    tpl_t notify_tpl;
    event_notify_t notify_function;
    void* notify_context;
    list_entry_t notify_link;

    // is this event signaled
    bool signaled;
};

/**
 * Signal an event
 */
void event_signal(event_t* event);
