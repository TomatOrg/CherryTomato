#pragma once

#include <stdint.h>

#include <util/defs.h>
#include "core.h"

//----------------------------------------------------------------------------------------------------------------------
// Special registers and access to them
//----------------------------------------------------------------------------------------------------------------------

#define WSR(reg, value) \
    do { \
        asm volatile ("wsr %0, " STR(reg) :: "r"((uint32_t)value) : "memory"); \
    } while (0)

#define RSR(reg) \
    ({\
        uint32_t value; \
        asm volatile ("rsr %0, " STR(reg) : "=r"(value)); \
        value; \
    })

#define WITLB(at, as) asm volatile ("witlb  %0, %1; \n isync \n " : : "r" (at), "r" (as))
#define WDTLB(at, as) asm volatile ("wdtlb  %0, %1; \n dsync \n " : : "r" (at), "r" (as))

static inline void __rsync(void) {
    asm ("rsync");
}

static inline void __isync(void) {
    asm ("isync");
}

static inline void disable_interrupts() {
    WSR(INTENABLE, 0);
}

static inline void enable_interrupts() {
    WSR(INTENABLE, (1 << XCHAL_NUM_INTLEVELS) - 1);
}
