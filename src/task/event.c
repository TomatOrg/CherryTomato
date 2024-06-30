#include "event.h"
#include "lock.h"
#include "util/except.h"

#include <stdint.h>


uint32_t g_event_pending = 0;

/**
 * The lock that protects the entire event queue
 */
static lock_t m_event_queue_lock = { .tpl = TPL_HIGH_LEVEL };

/**
 * The event queue for each tpl level
 */
static list_t m_event_queue[TPL_HIGH_LEVEL + 1] = {
    INIT_LIST(m_event_queue[0]),
    INIT_LIST(m_event_queue[1]),
    INIT_LIST(m_event_queue[2]),
    INIT_LIST(m_event_queue[3]),
};

static void event_lock_acquire() {
    lock_acquire(&m_event_queue_lock);
}

static void event_lock_release() {
    lock_release(&m_event_queue_lock);
}

void event_dispatch_notifies(tpl_t tpl) {
    event_lock_acquire();

    list_entry_t* head = &m_event_queue[tpl];

    while (!list_is_empty(head)) {
        // pop the event
        event_t* event = CR(head->next, event_t, notify_link);
        list_remove(&event->notify_link);

        // no longer signaled since we are handling it right now
        event->signaled = false;

        // run outside the lock
        event_lock_release();
        event->notify_function(event, event->notify_context);
        event_lock_acquire();
    }

    // we handled everything, clear the bit
    g_event_pending &= ~(1 << tpl);
    event_lock_release();
}

static void event_notify(event_t* event) {
    // queue the event and set the pending mask
    ASSERT(event->notify_function != NULL);
    ASSERT(event->notify_tpl <= TPL_NOTIFY);
    list_insert_tail(&m_event_queue[event->notify_tpl], &event->notify_link);
    g_event_pending |= (1 << event->notify_tpl);
}

void event_signal(event_t* event) {
    event_lock_acquire();

    if (!event->signaled) {
        event->signaled = true;
        event_notify(event);
    }

    event_lock_release();
}
