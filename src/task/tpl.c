#include "tpl.h"
#include "util/except.h"
#include "event.h"
#include "event_internal.h"

/**
 * The current priority level
 */
static tpl_t m_current_tpl = TPL_APPLICATION;

tpl_t tpl_raise(tpl_t new_tpl) {
    tpl_t old_tpl = m_current_tpl;
    ASSERT(old_tpl <= new_tpl);

    // if we raise to high level disable interrupts
    if ((new_tpl >= TPL_HIGH_LEVEL) && (old_tpl < TPL_HIGH_LEVEL)) {
        disable_interrupts();
    }

    // set the new value
    m_current_tpl = new_tpl;

    return old_tpl;
}

void tpl_restore(tpl_t new_tpl) {
    tpl_t old_tpl = m_current_tpl;
    ASSERT(old_tpl >= new_tpl);

    // check if there are events to run
    while (g_event_pending != 0) {
        // find the tpl of the event to run
        uint32_t pending_tpl = 31 - __builtin_clz(g_event_pending);
        if (pending_tpl <= new_tpl) {
            break;
        }

        // we got something, lower to that event, and
        // make sure to enable interrupts if needed
        m_current_tpl = pending_tpl;
        if (m_current_tpl < TPL_HIGH_LEVEL) {
            enable_interrupts();
        }

        // run the notifies
        event_dispatch_notifies(m_current_tpl);
    }

    // set the new tpl
    m_current_tpl = new_tpl;

    // if we lowered to less than high level then
    // make sure interrupts are enabled
    if (m_current_tpl < TPL_HIGH_LEVEL) {
        enable_interrupts();
    }
}

tpl_t tpl_raise_interrupt() {
    ASSERT(!get_interrupt_state());

    // raise to a high level, getting the interrupted tpl
    tpl_t interrupted_tpl = tpl_raise(TPL_HIGH_LEVEL);
    ASSERT(interrupted_tpl != TPL_HIGH_LEVEL);

    return interrupted_tpl;
}

/**
 * The highest tpl that is currently the target
 * of a restore tpl call
 */
static tpl_t m_in_progress_restore_tpl;

/**
 * A nested interrupt requested that we restore the TPL
 * in our own stack frame
 */
static bool m_deferred_restore_tpl;

void tpl_restore_interrupt(tpl_t interrupted_tpl) {
    ASSERT(!get_interrupt_state());

    // we are already handling this level, so defer the restore to the
    // lower instance
    if (interrupted_tpl == m_in_progress_restore_tpl) {
        // mark for defer
        ASSERT(!m_deferred_restore_tpl);
        m_deferred_restore_tpl = true;

        // TODO: disable interrupts on iret
        return;
    }

    // if we got to here we have a higher priority
    // restore to run, so we will do it on our own
    while (true) {
        ASSERT(!get_interrupt_state());
        ASSERT(m_in_progress_restore_tpl < interrupted_tpl);
        ASSERT(!m_deferred_restore_tpl);

        // raise to our own tpl
        tpl_t saved_in_progress_tpl = m_in_progress_restore_tpl;
        m_in_progress_restore_tpl = interrupted_tpl;

        // restore the lower priority, this will enable interrupts, but
        // that is fine  since it will defer to us if need be
        tpl_restore(interrupted_tpl);

        // disable interrupts again
        disable_interrupts();

        // make sure the state is intact and restore the lower tpl
        ASSERT(m_in_progress_restore_tpl == interrupted_tpl);
        m_in_progress_restore_tpl = saved_in_progress_tpl;

        // get the current defer status
        bool deferred = m_deferred_restore_tpl;
        m_deferred_restore_tpl = false;

        // if no one has deferred to us we can exit
        if (!deferred) {
            return;
        }
    }
}


