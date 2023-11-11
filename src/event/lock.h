#pragma once

#include <stdbool.h>
#include "tpl.h"

typedef enum critical_section {
    LOCK_EXITED = 0,
    LOCK_ACQUIRED = 1,
} lock_state_t;

/**
 * A simple critical section designed for single-core systems that just
 * makes sure different TPLs don't mess with each other
 */
typedef struct lock {
    tpl_t tpl;
    tpl_t owner_tpl;
    lock_state_t state;
} lock_t;

#define INIT_LOCK(_tpl) ((lock_t){ .tpl = _tpl, .state = LOCK_EXITED })

void lock_acquire(lock_t* lock);

void lock_release(lock_t* lock);
