#include "tpl.h"

#include "event_internal.h"
#include "intrin.h"
#include "event.h"

static tpl_t m_current_tpl = 0;

tpl_t raise_tpl(tpl_t new_tpl) {
    tpl_t old_tpl = m_current_tpl;

    if ((new_tpl >= TPL_HIGH_LEVEL) && (old_tpl < TPL_HIGH_LEVEL)) {
        disable_interrupts();
    }

    m_current_tpl = new_tpl;

    return old_tpl;
}

void restore_tpl(tpl_t new_tpl) {
    tpl_t old_tpl = m_current_tpl;

    if ((old_tpl >= TPL_HIGH_LEVEL) && (new_tpl < TPL_HIGH_LEVEL)) {
        m_current_tpl = TPL_HIGH_LEVEL;
    }

    while (g_event_pending != 0) {
        tpl_t pending_tpl = 31 - __builtin_clz(g_event_pending);
        if (pending_tpl <= new_tpl) {
            break;
        }

        m_current_tpl = pending_tpl;
        if (m_current_tpl < TPL_HIGH_LEVEL) {
            enable_interrupts();
        }

        dispatch_event_notifies(m_current_tpl);
    }

    m_current_tpl = new_tpl;

    if (m_current_tpl < TPL_HIGH_LEVEL) {
        enable_interrupts();
    }
}
