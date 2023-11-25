
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
 * Interrupt levels - simply emulate a level1 interrupt so we can more easily share the code
 **********************************************************************************************************************/

/* interrupt level 2 - 0x180 */
.align 64
.type level_2_vector,@function
level_2_vector:
    // save the original sp
    wsr.excsave2 a1

    // allocate the interrupt frame
    addi a1, a1, -16 - IFRAME_SIZE

    // save a0 and a1 to the iframe
    s32i a0, a1, IFRAME_A(0)
    rsr.excsave2 a0
    s32i a0, a1, IFRAME_A(1)

    // save the entire iframe
    call0 save_interrupt_frame

    // call the level5 handler, no need to pass the state
    // since we are not going to make use of it anyways
    call4 level_2_interrupt_handler

    // restore it fully
    call0 restore_interrupt_frame

    // restore a0 and a1, need to make sure a1 is only restored last
    // since it contains the reference to all of our state
    l32i a0, a1, IFRAME_A(0)
    l32i a1, a1, IFRAME_A(1)

    rfi 2
.size level_2_vector, . - level_2_vector

/* interrupt level 3 - 0x1c0 */
.align 64
.type level_3_vector,@function
level_3_vector:
    // save the original sp
    wsr.excsave3 a1

    // allocate the interrupt frame
    addi a1, a1, -16 - IFRAME_SIZE

    // save a0 and a1 to the iframe
    s32i a0, a1, IFRAME_A(0)
    rsr.excsave3 a0
    s32i a0, a1, IFRAME_A(1)

    // save the entire iframe
    call0 save_interrupt_frame

    // call the level5 handler, no need to pass the state
    // since we are not going to make use of it anyways
    call4 level_3_interrupt_handler

    // restore it fully
    call0 restore_interrupt_frame

    // restore a0 and a1, need to make sure a1 is only restored last
    // since it contains the reference to all of our state
    l32i a0, a1, IFRAME_A(0)
    l32i a1, a1, IFRAME_A(1)

    rfi 3
.size level_3_vector, . - level_3_vector

/* interrupt level 4 - 0x200 */
.align 64
.type level_4_vector,@function
level_4_vector:
    // save the original sp
    wsr.excsave4 a1

    // allocate the interrupt frame
    addi a1, a1, -16 - IFRAME_SIZE

    // save a0 and a1 to the iframe
    s32i a0, a1, IFRAME_A(0)
    rsr.excsave4 a0
    s32i a0, a1, IFRAME_A(1)

    // save the entire iframe
    call0 save_interrupt_frame

    // call the level5 handler, no need to pass the state
    // since we are not going to make use of it anyways
    call4 level_4_interrupt_handler

    // restore it fully
    call0 restore_interrupt_frame

    // restore a0 and a1, need to make sure a1 is only restored last
    // since it contains the reference to all of our state
    l32i a0, a1, IFRAME_A(0)
    l32i a1, a1, IFRAME_A(1)

    rfi 4
.size level_4_vector, . - level_4_vector

/* interrupt level 5 - 0x240 */
.align 64
.type level_5_vector,@function
level_5_vector:
    // save the original sp
    wsr.excsave5 a1

    // allocate the interrupt frame
    addi a1, a1, -16 - IFRAME_SIZE

    // save a0 and a1 to the iframe
    s32i a0, a1, IFRAME_A(0)
    rsr.excsave5 a0
    s32i a0, a1, IFRAME_A(1)

    // save the entire iframe
    call0 save_interrupt_frame

    // call the level5 handler, no need to pass the state
    // since we are not going to make use of it anyways
    call4 level_5_interrupt_handler

    // restore it fully
    call0 restore_interrupt_frame

    // restore a0 and a1, need to make sure a1 is only restored last
    // since it contains the reference to all of our state
    l32i a0, a1, IFRAME_A(0)
    l32i a1, a1, IFRAME_A(1)

    rfi 5
.size level_5_vector, . - level_5_vector

/* interrupt level 6 (debug) - 0x280 */
.align 64
.type debug_vector,@function
debug_vector:
    j debug_vector
    rfi 6
.size debug_vector, . - debug_vector

/* interrupt level 7 (nmi) - 0x2c0 */
.align 64
.type nmi_vector,@function
nmi_vector:
    j nmi_vector
    rfi 7
.size nmi_vector, . - nmi_vector

/***********************************************************************************************************************
 * Normal exceptions
 **********************************************************************************************************************/

/*
 * kernel exception - 0x300
 */
.align 64
.type kernel_exception,@function
kernel_exception:
    // save the original sp
    wsr.excsave1 a1

    // allocate the interrupt frame
    addi a1, a1, -16 - IFRAME_SIZE

    // save a0 and a1 to the iframe
    s32i a0, a1, IFRAME_A(0)
    rsr.excsave1 a0
    s32i a0, a1, IFRAME_A(1)

    // EPC1 might get overwritten
    rsr.epc1 a0
    s32i a0, a1, IFRAME_PC

    // save the entire iframe
    call0 save_interrupt_frame

    // call the kernel exception handler with the iframe as a parameter so
    // that it can have access to the full state
    // use call4 since it a windowed abi function
    mov a6, a1
    call4 kernel_exception_handler

    // restore it fully
    call0 restore_interrupt_frame

    // restore the epc1
    l32i a0, a1, IFRAME_PC
    wsr.epc1 a0

    // restore a0 and a1, need to make sure a1 is only restored last
    // since it contains the reference to all of our state
    l32i a0, a1, IFRAME_A(0)
    l32i a1, a1, IFRAME_A(1)

    rfe
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

.type save_interrupt_frame,@function
save_interrupt_frame:
    // save a2 and a3 so we can use them
    s32i a2, a1, IFRAME_A(2)
    s32i a3, a1, IFRAME_A(3)

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
    l32i a2, a1, IFRAME_SIZE + 4
    s32e a3, a1, -16
    s32e a2, a1, -12

1:
    // zero and save the lcount
    rsr.lcount a2
    s32i a2, a1, IFRAME_LCOUNT

    // save the PS, we will need it when restoring the context
    rsr.ps a3
    s32i a3, a1, IFRAME_PS

    // we are going to keep the intlevel
    // but set the window enable, if its 0
    // it means we come from level 1 so mask
    // level 1 interrupts
    movi a2, 1
    extui a3, a3, 0, 4
    moveqz a3, a2, a3
    movi a2, (1 << 18)
    or a3, a3, a2
    wsr.ps a3
    rsync

    // save the rest of the loop stuff
    rsr.lbeg a4
    rsr.lend a3
    s32i a4, a1, IFRAME_LBEG
    s32i a3, a1, IFRAME_LEND

    // we are done saving everything
    ret
.size save_interrupt_frame, . - save_interrupt_frame

.type restore_interrupt_frame,@function
restore_interrupt_frame:
    // restore the PS, so we will not get any interrupts
    // while restoring everything
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
    beqi a2, 1, 1f

    // now check if it got spilled
    rsr.windowstart a3
    addi a2, a3, -1
    and a3, a3, a2
    bnez a3, 1f

    // we need to do a movsp
    addi    a2, a1, -16
    l32i    a3, a2, 0
    l32i    a4, a2, 4
    s32i    a3, a1, IFRAME_SIZE + 0
    s32i    a4, a1, IFRAME_SIZE + 4
    l32i    a3, a2, 8
    l32i    a4, a2, 12
    s32i    a3, a1, IFRAME_SIZE + 8
    s32i    a4, a1, IFRAME_SIZE + 12

1:
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

1:
    // restore sar
    l32i a3, a1, IFRAME_SAR
    wsr.sar a3

    // restore loop stuff
    l32i a2, a1, IFRAME_LBEG
    l32i a3, a1, IFRAME_LEND
    wsr.lbeg a2
    l32i a2, a1, IFRAME_LCOUNT
    wsr.lend a3
    wsr.lcount a2

    // restore a2,a3
    l32i a3, a1, IFRAME_A(3)
    l32i a2, a1, IFRAME_A(2)

    // we are done restoring everything we can
    ret
.size restore_interrupt_frame, . - restore_interrupt_frame
