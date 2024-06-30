#pragma once

typedef enum tpl {
    TPL_APPLICATION,
    TPL_CALLBACK,
    TPL_NOTIFY,
    TPL_TIMER,
    TPL_HIGH_LEVEL,
} tpl_t;

/**
 * Raise the task's priority level and return its previous value
 */
tpl_t tpl_raise(tpl_t new_tpl);

/**
 * Restore the task's priority level to its previous value
 */
void tpl_restore(tpl_t old_tpl);

/**
 * Raise a tpl from an interrupt handler, this handles nesting properly.
 *
 * returns the interrupted tpl
 */
tpl_t tpl_raise_interrupt();

/**
 * Restore the tpl from an interrupt handler
 */
void tpl_restore_interrupt(tpl_t interrupted_tpl);
