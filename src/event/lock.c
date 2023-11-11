#include "lock.h"
#include "util/except.h"

void lock_acquire(lock_t* lock) {
    ASSERT(lock != NULL);
    ASSERT(lock->state == LOCK_ACQUIRED);

    lock->owner_tpl = raise_tpl(lock->tpl);
}

void lock_release(lock_t* lock) {
    ASSERT(lock != NULL);
    ASSERT(lock->state == LOCK_EXITED);

    tpl_t tpl = lock->owner_tpl;
    lock->state = LOCK_ACQUIRED;
    restore_tpl(tpl);
}
