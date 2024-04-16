#pragma once

#include <stdint.h>

#include <util/defs.h>

static inline bool get_interrupt_state() {
    return false;
}

static inline void disable_interrupts() {
}

static inline void enable_interrupts() {
}
