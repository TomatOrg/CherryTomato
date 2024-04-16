#include "timer.h"
#include "lock.h"
#include "target/target.h"
#include "util/except.h"

/**
 * The list of timers, sorted by expiry
 */
static list_t m_timer_list = INIT_LIST(m_timer_list);

/**
 * Lock to protect the timer list, runs at a timer priority which
 * is not with interrupts disabled
 */
static lock_t m_timer_lock = { .tpl = TPL_TIMER };

/**
 * Update the timer configuration
 */
static void timer_update() {
    if (list_is_empty(&m_timer_list)) {
        // if there are no more timers then we have no need
        // for any more timers
        target_set_next_tick(0);
    } else {
        target_set_next_tick(CR(m_timer_list.next, timer_t, link)->trigger_time);
    }
}

static void timer_insert(timer_t* timer) {
    uint64_t trigger_time = timer->trigger_time;

    // find the place in the list to put the timer
    list_entry_t* link;
    for (link = m_timer_list.next; link != &m_timer_list; link = link->next) {
        timer_t* timer2 = CR(link, timer_t, link);
        if (timer2->trigger_time > trigger_time) {
            break;
        }
    }

    // finally insert it
    list_insert_tail(link, &timer->link);
}

static void timer_cancel_unlocked(timer_t* timer) {
    if (timer->link.next != NULL) {
        list_remove(&timer->link);
    }

    timer->trigger_time = 0;
    timer->interval = 0;
}

void timer_set_timeout(timer_t* timer, size_t timeout) {
    lock_acquire(&m_timer_lock);
    timer_cancel_unlocked(timer);
    timer->trigger_time = target_get_current_tick() + timeout;
    timer_insert(timer);
    timer_update();
    lock_release(&m_timer_lock);
}

void timer_set_interval(timer_t* timer, size_t interval) {
    ASSERT(interval != 0);

    lock_acquire(&m_timer_lock);
    timer_cancel_unlocked(timer);
    timer->trigger_time = target_get_current_tick() + interval;
    timer->interval = interval;
    timer_insert(timer);
    timer_update();
    lock_release(&m_timer_lock);
}

void timer_cancel(timer_t* timer) {
    lock_acquire(&m_timer_lock);
    timer_cancel_unlocked(timer);
    timer_update();
    lock_release(&m_timer_lock);
}

void timer_dispatch() {
    lock_acquire(&m_timer_lock);

    // if we have a case where the time passed was too long and
    // we need to run the timer multiple times, we are going to
    // reset the timer and let the platform call us again
    uint64_t current_time = target_get_current_tick();

    while (!list_is_empty(&m_timer_list)) {
        // get the timer and check if we need to dispatch it
        timer_t* timer = CR(m_timer_list.next, timer_t, link);
        if (timer->trigger_time > current_time) {
            break;
        }

        // remove it from the list
        list_remove(&timer->link);

        // signal
        event_signal(&timer->event);

        // if periodic then set the next time to run it
        if (timer->interval != 0) {
            timer->trigger_time = timer->trigger_time + timer->interval;

            // make sure we won't have a loop of being called too many times
            if (timer->trigger_time <= current_time) {
                timer->trigger_time = current_time;
            }

            timer_insert(timer);
        }
    }

    timer_update();

    lock_release(&m_timer_lock);
}
