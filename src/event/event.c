#include "event.h"
#include "lock.h"
#include "util/except.h"

#include <util/list.h>

uint32_t g_event_pending = 0;

/**
 * A lock to protect the event queue
 */
static lock_t m_event_queue_lock = INIT_LOCK(TPL_HIGH_LEVEL);

/**
 * A list of event's to notify for each priority level
 */
static list_t m_event_queue[TPL_HIGH_LEVEL + 1];

/**
 * Event that is signalled whenever the core is idle
 */
static event_t m_idle_loop_event;

static void acquire_event_lock() {
    lock_acquire(&m_event_queue_lock);
}

static void release_event_lock() {
    lock_release(&m_event_queue_lock);
}

void dispatch_event_notifies(tpl_t priority) {
    acquire_event_lock();

    list_entry_t* head = &m_event_queue[priority];

    while (!list_is_empty(head)) {
        event_t* event = CR(head->next, event_t, notify_link);
        list_remove(&event->notify_link);

        if (event->type == EVENT_NOTIFY_SIGNAL) {
            event->signal_count = 0;
        }

        release_event_lock();
        event->notify_function(event, event->notify_context);
        acquire_event_lock();
    }

    g_event_pending &= ~(1u << priority);
    release_event_lock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// API
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void notify_event(event_t* event) {
    // if already queued remove it
    if (event->notify_link.next != NULL) {
        list_remove(&event->notify_link);
    }

    // queue it again
    list_insert_tail(&m_event_queue[event->notify_tpl], &event->notify_link);
    g_event_pending |= (1 << event->notify_tpl);
}

void signal_event(event_t* event) {
    acquire_event_lock();

    // set the signal status
    if (event->signal_count == 0) {
        event->signal_count++;

        // do we need to signal this event?
        if (event->type == EVENT_NOTIFY_SIGNAL) {
            notify_event(event);
        }
    }

    release_event_lock();
}

bool check_event(event_t* event) {
    bool status = false;

    // check if we need to queue it
    if ((event->signal_count == 0) && (event->type == EVENT_NOTIFY_WAIT)) {
        acquire_event_lock();
        if (event->signal_count == 0) {
            notify_event(event);
        }
        release_event_lock();
    }

    // if the event looks signalled then clear it
    if (event->signal_count != 0) {
        acquire_event_lock();
        if (event->signal_count != 0) {
            event->signal_count = 0;
            status = true;
        }
        release_event_lock();
    }

    return status;
}

size_t wait_for_event(size_t event_count, event_t* events) {
    for (;;) {
        // check if any event is ready
        for (size_t i = 0; i < event_count; i++) {
            if (check_event(&events[i])) {
                return i;
            }
        }

        // no event is ready, signal the idle event
        signal_event(&m_idle_loop_event);
    }
}

void close_event(event_t* event) {
    acquire_event_lock();

    if (event->notify_link.next != NULL) {
        list_remove(&event->notify_link);
    }

    release_event_lock();
}
