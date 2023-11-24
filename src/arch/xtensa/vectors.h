#pragma once

#include "frame_offsets.h"

#include <util/defs.h>

#include <stdint.h>

extern char __vecbase[];

typedef struct interrupt_frame {
    uint32_t a[16];
    uint32_t pc;
    uint32_t ps;
    uint32_t sar;
    uint32_t lcount;
    uint32_t lbeg;
    uint32_t lend;
    uint32_t window_mask;
    uint32_t exccause;
} interrupt_frame_t;

STATIC_ASSERT(sizeof(interrupt_frame_t) % 16 == 0);
STATIC_ASSERT(offsetof(interrupt_frame_t, pc) == IFRAME_PC);
STATIC_ASSERT(offsetof(interrupt_frame_t, ps) == IFRAME_PS);
STATIC_ASSERT(offsetof(interrupt_frame_t, sar) == IFRAME_SAR);
STATIC_ASSERT(offsetof(interrupt_frame_t, lcount) == IFRAME_LCOUNT);
STATIC_ASSERT(offsetof(interrupt_frame_t, lbeg) == IFRAME_LBEG);
STATIC_ASSERT(offsetof(interrupt_frame_t, lend) == IFRAME_LEND);
STATIC_ASSERT(offsetof(interrupt_frame_t, window_mask) == IFRAME_WINDOWMASK);
STATIC_ASSERT(offsetof(interrupt_frame_t, window_mask) == IFRAME_WINDOWMASK);
STATIC_ASSERT(sizeof(interrupt_frame_t) == IFRAME_SIZE);
