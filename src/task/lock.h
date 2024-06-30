#pragma once

#include "tpl.h"

#include <stdbool.h>

typedef struct lock {
    // the tpl of the owner
    tpl_t owner_tpl;

    // the tpl we are going to raise
    // to in the lock
    tpl_t tpl;

#ifdef __DEBUG__
    // for debug, makes sure that we are locking
    // and unlocking properly
    bool locked;
#endif
} lock_t;

/**
 * Acquire the lock
 */
void lock_acquire(lock_t* lock);

/**
 * Release the lock
 */
void lock_release(lock_t* lock);
