#include "timer.h"
#include "lock.h"
#include "platform/platform.h"

/**
 * Lock to protect the list of timers
 */
static lock_t m_timer_lock = INIT_LOCK(TPL_HIGH_LEVEL - 1);

/**
 * List of timers
 */
static list_t m_timer_list = INIT_LIST(m_timer_list);

/**
 * Lock to protect the system time
 */
static lock_t m_system_time_lock = INIT_LOCK(TPL_HIGH_LEVEL);

/**
 * The current time, updated synchronously with the timers
 */
static uint64_t m_system_time = 0;

uint64_t get_current_system_time() {
    lock_acquire(&m_system_time_lock);
    uint64_t time = m_system_time;
    lock_release(&m_system_time_lock);
    return time;
}

static void insert_event_timer(timer_t* timer) {
    // find where to insert the event timer
    uint64_t trigger_time = timer->trigger_time;
    list_entry_t* link;
    for (link = m_timer_list.next; link != &m_timer_list; link = link->next) {
        timer_t* timer2 = CR(link, timer_t, link);

        if (timer2->trigger_time > trigger_time) {
            break;
        }
    }

    // and insert it
    list_insert_tail(link, &timer->link);
}

void set_timer(timer_t* timer, timer_type_t type, uint64_t trigger_time) {
    lock_acquire(&m_timer_lock);

    // if the timer is queued then remove it
    if (timer->link.next != NULL) {
        list_remove(&timer->link);
    }

    timer->trigger_time = 0;
    timer->period = 0;

    // setup the period
    if (type == TIMER_PERIODIC) {
        if (trigger_time == 0) {
            trigger_time = platform_timer_get_period();
        }
        timer->period = trigger_time;
    }

    // set the trigger time
    timer->trigger_time = get_current_system_time() + trigger_time;
    insert_event_timer(timer);

    // if there is no trigger time trigger it right now
    if (trigger_time == 0) {
        signal_event(&timer->event);
    }

    lock_release(&m_timer_lock);
}

void cancel_timer(timer_t* timer) {
    lock_acquire(&m_timer_lock);

    // if the timer is queued then remove it
    if (timer->link.next != NULL) {
        list_remove(&timer->link);
    }

    // close the backing event of this timer
    close_event(&timer->event);

    lock_release(&m_timer_lock);
}

/**
 * Check the timers, dispatching everything which should be dispatched
 */
static void check_timers(event_t* event, void* context) {
    lock_acquire(&m_timer_lock);
    uint64_t system_timer = get_current_system_time();

    while (!list_is_empty(&m_timer_list)) {
        timer_t* timer = CR(m_timer_list.next, timer_t, link);

        // if the timer is not expired yet break out
        if (timer->trigger_time > system_timer) {
            break;
        }

        // remove it
        list_remove(&timer->link);

        // signal it
        signal_event(&timer->event);

        // if its periodic schedule it again
        if (timer->period != 0) {
            timer->trigger_time = timer->trigger_time + timer->period;

            // if its before now then reset the timer to start from now
            if (timer->trigger_time <= system_timer) {
                timer->trigger_time = system_timer;
                signal_event(&timer->event);
            }

            // add the timer
            insert_event_timer(timer);
        }
    }

    lock_release(&m_timer_lock);
}

/**
 * Event which is triggered whenever there are timers to run
 */
static event_t m_check_timer_event = {
    .type = EVENT_NOTIFY_SIGNAL,
    .notify_tpl = TPL_HIGH_LEVEL - 1,
    .notify_function = check_timers
};

void timer_tick(uint64_t duration) {
    lock_acquire(&m_system_time_lock);

    // update the system time
    m_system_time += duration;

    // if the head f the list is expired, fire the timer event
    // to process it
    if (!list_is_empty(&m_timer_list)) {
        timer_t* timer = CR(m_timer_list.next, timer_t, link);

        if (timer->trigger_time <= m_system_time) {
            signal_event(&m_check_timer_event);
        }
    }

    lock_release(&m_system_time_lock);
}
