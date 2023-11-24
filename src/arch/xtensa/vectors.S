
#include "frame_offsets.h"

.section .text

/**
 * The way interrupt vectors work in xtensa is that there are 64byte between vector entries and
 * it jumps to the constant offset between each of them.
 *
 * qemu xtensa_cpu_do_interrupt codes:
 *  3 - window overflow 4
 *  4 - window underflow 4
 *  5 - window overflow 8
 *  6 - window underflow 8
 *  7 - window overflow 12
 *  8 - window underflow 12
 *  9 - irq
 *  10 - kernel
 *  11 - user
 *  12 - double
 *  13 - debug
 */

/***********************************************************************************************************************
 * Window overflow/underflow, these are pretty simple
 *
 * For these we simply need to spill/load to/from the stack the first x gpregs,
 * we need to use the special rfwo (return from window overflow) and rfwu (return from window underflow)
 * to properly handle these
 **********************************************************************************************************************/

.global __vecbase

.align 0x1000
__vecbase:
window_overflow_4:
/* window overflow 4 - 0x00 */
    s32e	a0, a5, -16
    s32e	a1, a5, -12
    s32e	a2, a5,  -8
    s32e	a3, a5,  -4
    rfwo

.align 64
window_underflow_4:
/* window underflow 4 - 0x40 */
    l32e	a0, a5, -16
    l32e	a1, a5, -12
    l32e	a2, a5,  -8
    l32e	a3, a5,  -4
    rfwu

.align 64
window_overflow_8:
/* windows overflow 8 - 0x80 */
    s32e	a0, a9, -16
    l32e	a0, a1, -12
    s32e	a2, a9,  -8
    s32e	a1, a9, -12
    s32e	a3, a9,  -4
    s32e	a4, a0, -32
    s32e	a5, a0, -28
    s32e	a6, a0, -24
    s32e	a7, a0, -20
    rfwo

.align 64
window_underflow_8:
/* windows underflow 8 - 0xc0 */
    l32e	a1, a9, -12
    l32e	a0, a9, -16
    l32e	a7, a1, -12
    l32e	a2, a9,  -8
    l32e	a4, a7, -32
    l32e	a3, a9,  -4
    l32e	a5, a7, -28
    l32e	a6, a7, -24
    l32e	a7, a7, -20
    rfwu

.align 64
window_overflow_12:
/* windows overflow 12 - 0x100 */
    s32e	a0,  a13, -16
    l32e	a0,  a1,  -12
    s32e	a1,  a13, -12
    s32e	a2,  a13,  -8
    s32e	a3,  a13,  -4
    s32e	a4,  a0,  -48
    s32e	a5,  a0,  -44
    s32e	a6,  a0,  -40
    s32e	a7,  a0,  -36
    s32e	a8,  a0,  -32
    s32e	a9,  a0,  -28
    s32e	a10, a0,  -24
    s32e	a11, a0,  -20
    rfwo

.align 64
window_underflow_12:
/* windows underflow 12 - 0x140 */
    l32e	a1,  a13, -12
    l32e	a0,  a13, -16
    l32e	a11, a1,  -12
    l32e	a2,  a13,  -8
    l32e	a4,  a11, -48
    l32e	a8,  a11, -32
    l32e	a3,  a13,  -4
    l32e	a5,  a11, -44
    l32e	a6,  a11, -40
    l32e	a7,  a11, -36
    l32e	a9,  a11, -28
    l32e	a10, a11, -24
    l32e	a11, a11, -20
    rfwu

/***********************************************************************************************************************
 * Interrupt levels
 **********************************************************************************************************************/

.align 64
.type level_2_vector,@function
level_2_vector:
/* interrupt level 2 - 0x180 */
    j level_2_vector
    rfi 2
.size level_2_vector, . - level_2_vector

.align 64
level_3_vector:
/* interrupt level 3 - 0x1c0 */
    j level_3_vector
    rfi 3

.align 64
level_4_vector:
/* interrupt level 4 - 0x200 */
    j level_4_vector
    rfi 4

.align 64
level_5_vector:
/* interrupt level 5 - 0x240 */
    j level_5_vector
    rfi 5

.align 64
debug_vector:
/* interrupt level 6 (debug) - 0x280 */
    j debug_vector
    rfi 6

.align 64
nmi_vector:
/* interrupt level 7 (nmi) - 0x2c0 */
    j nmi_vector
    rfi 7

/***********************************************************************************************************************
 * Normal exceptions
 **********************************************************************************************************************/

/*
 * kernel exception - 0x300
 */
.align 64
.type kernel_exception,@function
kernel_exception:
    // save a3 and a2, w ecan use depc because we don't expect
    // a double fault to happen, and if it happens we will not
    // restore execution anyways
    wsr.excsave1 a2

    // allocate the interrupt frame
    addi a2, a1, -16 - IFRAME_SIZE

    // save a0 to the iframe
    s32i a0, a2, IFRAME_A(0)

    // just call the common exception handler, we will dispatch
    // the correct handler from there
    j dispatch_kernel_exception
.size kernel_exception, . - kernel_exception

/*
 * user exception - 0x340
 */
.align 64
user_exception:
    rfe

.align 64
/* double exception - 0x380 */
    rfde


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The actual exception handlers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

.type dispatch_kernel_exception,@function
dispatch_kernel_exception:
    // save a1, a2, a3 and set SP
    rsr.excsave1 a0
    s32i a1, a2, IFRAME_A(1)
    s32i a0, a2, IFRAME_A(2)
    s32i a3, a2, IFRAME_A(3)
    mov a1, a2

    // save SAR
    rsr.sar a2
    s32i a2, a1, IFRAME_SAR

    // prepare for spilling the entire frame, we need to have the window be
    // in a position where the window start is at bit0
    rsr.windowbase a2
    rsr.windowstart a3
    ssr a2
    slli a2, a3, 16
    src a2, a3, a2
    srli a2, a2, 16
    s32i a2, a1, IFRAME_WINDOWMASK

    // save only the live window-frame
    bbsi a2, 1, 1f
    s32i a4, a1, IFRAME_A(4)
    s32i a5, a1, IFRAME_A(5)
    s32i a6, a1, IFRAME_A(6)
    s32i a7, a1, IFRAME_A(7)
    bbsi a2, 2, 1f
    s32i a8, a1, IFRAME_A(8)
    s32i a9, a1, IFRAME_A(9)
    s32i a10, a1, IFRAME_A(10)
    s32i a11, a1, IFRAME_A(11)
    bbsi a2, 3, 1f
    s32i a12, a1, IFRAME_A(12)
    s32i a13, a1, IFRAME_A(13)
    s32i a14, a1, IFRAME_A(14)
    s32i a15, a1, IFRAME_A(15)

    bnei a2, 1, 1f

    // we need to emulate a movsp in order
    // to keep the exception stack contig for spilling
    // the window properly
    l32i a3, a1, IFRAME_SIZE
    l32i a0, a1, IFRAME_SIZE + 4
    s32e a3, a1, -16
    s32e a0, a1, -12

1:
    // restore the saved a0
    l32i a0, a1, IFRAME_A(0)

    // save the pc
    rsr.epc1 a2
    s32i a2, a1, IFRAME_PC

    // zero and save the lcount
    movi a2, 0
    xsr a2, lcount
    s32i a2, a1, IFRAME_LCOUNT

    // save the ps
    rsr.ps a3
    s32i a3, a1, IFRAME_PS

    // save the exccause
    rsr.exccause a2
    s32i a2, a1, IFRAME_EXCCAUSE

    // restore the return address for better stack tracing
    // we want this before enabling interrupts again
    l32i a0, a1, IFRAME_A(0)

    // we are going to keep the intlevel
    // but set the window enable, if its 0
    // it means we come from level 1 so mask
    // level 1 interrupts
    movi a0, 1
    extui a3, a3, 0, 4
    moveqz a3, a0, a3
    movi a2, (1 << 18)
    or a3, a3, a2
    wsr.ps a3
    rsync

    // save the rest of the loop stuff
    rsr.lbeg a4
    rsr.lend a3
    s32i a4, a1, IFRAME_LBEG
    s32i a3, a1, IFRAME_LEND

    // call the kernel exception handler with the IFRAME as the argument
    mov a6, a1
    call4 kernel_exception_handler

    // start by restoring the ps
    l32i a3, a1, IFRAME_PS
    wsr.ps a3

    //
    // now the fun part, we first need to check if we need to emulate
    // a movsp in the case that the previous window-frame has been spilled
    // into the space we setup for it instead of the actual stack.
    //
    // to check for that we need to check if the windowstart bit of the
    // previous window-frame was set before but is zero now.
    //

    // first check if it was already spilled before hand
    l32i a2, a1, IFRAME_WINDOWMASK
    beqi a2, 1, .skip_movsp

    // now check if it got spilled
    rsr.windowstart a3
    addi a0, a3, -1
    and a3, a3, a0
    bnez a3, .skip_movsp

    // we need to do a movsp
    addi    a0, a1, -16
    l32i    a3, a0, 0
    l32i    a4, a0, 4
    s32i    a3, a1, IFRAME_SIZE + 0
    s32i    a4, a1, IFRAME_SIZE + 4
    l32i    a3, a0, 8
    l32i    a4, a0, 12
    s32i    a3, a1, IFRAME_SIZE + 8
    s32i    a4, a1, IFRAME_SIZE + 12

.skip_movsp:
    // restore the live range, just like before
    bbsi a2, 1, 1f
    s32i a4, a1, IFRAME_A(4)
    s32i a5, a1, IFRAME_A(5)
    s32i a6, a1, IFRAME_A(6)
    s32i a7, a1, IFRAME_A(7)
    bbsi a2, 2, 1f
    s32i a8, a1, IFRAME_A(8)
    s32i a9, a1, IFRAME_A(9)
    s32i a10, a1, IFRAME_A(10)
    s32i a11, a1, IFRAME_A(11)
    bbsi a2, 3, 1f
    s32i a12, a1, IFRAME_A(12)
    s32i a13, a1, IFRAME_A(13)
    s32i a14, a1, IFRAME_A(14)
    s32i a15, a1, IFRAME_A(15)

    // restore pc and sar
1:
    l32i a2, a1, IFRAME_PC
    l32i a3, a1, IFRAME_SAR
    wsr.epc1 a2
    wsr.sar a3

    // restore loop stuff
    l32i a2, a1, IFRAME_LBEG
    l32i a3, a1, IFRAME_LEND
    wsr.lbeg a2
    l32i a2, a1, IFRAME_LCOUNT
    wsr.lend a3
    wsr.lcount a2

    // restore a0-a3, with a1 being last
    l32i a3, a1, IFRAME_A(3)
    l32i a2, a1, IFRAME_A(2)
    l32i a0, a1, IFRAME_A(0)
    l32i a1, a1, IFRAME_A(1)
    rfe
.size dispatch_kernel_exception, . - dispatch_kernel_exception
