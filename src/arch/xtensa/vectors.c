#include "vectors.h"
#include "intrin.h"
#include "util/log.h"
#include "util/except.h"

#include <util/printf.h>

#include <core-isa.h>

uint32_t g_enabled_interrupts = 0;

static interrupt_handler_t m_interrupt_handlers[XCHAL_NUM_INTERRUPTS];

static void dispatch_interrupts(uint32_t dispatch_mask) {
    ITERATE_SET_BITS(dispatch_mask) {
        if (_mask & g_enabled_interrupts) {
            m_interrupt_handlers[__builtin_ctz(_mask)]();
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
// High and medium priority handlers
//----------------------------------------------------------------------------------------------------------------------

void level_2_interrupt_handler() {
    dispatch_interrupts(RSR(INTERRUPT) & XCHAL_INTLEVEL2_MASK);
}

void level_3_interrupt_handler() {
    dispatch_interrupts(RSR(INTERRUPT) & XCHAL_INTLEVEL3_MASK);
}

void level_4_interrupt_handler() {
    dispatch_interrupts(RSR(INTERRUPT) & XCHAL_INTLEVEL4_MASK);
}

__attribute__((used))
void level_5_interrupt_handler() {
    dispatch_interrupts(RSR(INTERRUPT) & XCHAL_INTLEVEL5_MASK);
}

//----------------------------------------------------------------------------------------------------------------------
// Level 1 and exception handler
//----------------------------------------------------------------------------------------------------------------------

static const char* m_exception_name[] = {
    [XCHAL_EXCCAUSE_ILLEGAL_INSTRUCTION] = "Illegal Instruction",
    [XCHAL_EXCCAUSE_SYSTEM_CALL] = "System Call",
    [XCHAL_EXCCAUSE_INSTRUCTION_FETCH_ERROR] = "Instruction Fetch Error",
    [XCHAL_EXCCAUSE_LOAD_STORE_ERROR] = "Load Store Error",
    [XCHAL_EXCCAUSE_ALLOCA] = "Store Extension Assist",
    [XCHAL_EXCCAUSE_INTEGER_DIVIDE_BY_ZERO] = "Integer Divide by Zero",
};

void kernel_exception_handler(interrupt_frame_t* frame) {
    uint32_t exccause = RSR(EXCCAUSE);
    if (exccause == XCHAL_EXCCAUSE_LEVEL1_INTERRUPT) {
        dispatch_interrupts(RSR(INTERRUPT) & XCHAL_INTLEVEL1_MASK);
        return;
    }

    if (exccause < ARRAY_LENGTH(m_exception_name) && m_exception_name[exccause] != NULL) {
        LOG_CRITICAL("Got an exception `%s`", m_exception_name[exccause]);
    } else {
        LOG_CRITICAL("Got an exception `%d`", exccause);
    }

    LOG_CRITICAL("PC=%08x", frame->pc);
    LOG_CRITICAL("");
    LOG_CRITICAL("A00=%08x  A01=%08x  A02=%08x  A03=%08x", frame->a[0], frame->a[1], frame->a[2], frame->a[3]);
    LOG_CRITICAL("A04=%08x  A05=%08x  A06=%08x  A07=%08x", frame->a[4], frame->a[5], frame->a[6], frame->a[7]);
    LOG_CRITICAL("A08=%08x  A09=%08x  A10=%08x  A11=%08x", frame->a[8], frame->a[9], frame->a[10], frame->a[11]);
    LOG_CRITICAL("A12=%08x  A13=%08x  A14=%08x  A15=%08x", frame->a[12], frame->a[13], frame->a[14], frame->a[15]);

    for(;;) {}
}

//----------------------------------------------------------------------------------------------------------------------
// Register for interrupt handling
//----------------------------------------------------------------------------------------------------------------------

void register_interrupt(int intnum, interrupt_handler_t handler) {
    ASSERT(intnum < XCHAL_NUM_INTERRUPTS);

    disable_interrupts();
    g_enabled_interrupts |= 1 << intnum;
    m_interrupt_handlers[intnum] = handler;
    enable_interrupts();
}
