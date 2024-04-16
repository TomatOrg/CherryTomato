#include "lock.h"
#include "util/except.h"

void lock_acquire(lock_t* lock) {
    ASSERT(!lock->locked);

    lock->owner_tpl = tpl_raise(lock->tpl);

#ifdef __DEBUG__
    lock->locked = true;
#endif
}

void lock_release(lock_t* lock) {
    ASSERT(lock->locked);

#ifdef __DEBUG__
    lock->locked = false;
#endif

    tpl_restore(lock->owner_tpl);
}

