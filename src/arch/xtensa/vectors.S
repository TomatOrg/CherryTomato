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

.align 64
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
    s32e	a1, a9, -12
    s32e	a2, a9, -8
    s32e	a3, a9, -4
    s32e	a4, a0, -32
    s32e	a5, a0, -28
    s32e	a6, a0, -24
    s32e	a7, a0, -20
    rfwo

.align 64
window_underflow_8:
/* windows underflow 8 - 0xc0 */
    l32e	a0, a9, -16
    l32e	a1, a9, -12
    l32e	a2, a9, -8
    l32e	a7, a1, -12
    l32e	a3, a9, -4
    l32e	a4, a7, -32
    l32e	a5, a7, -28
    l32e	a6, a7, -24
    l32e	a7, a7, -20
    rfwu

.align 64
window_overflow_12:
/* windows overflow 12 - 0x100 */
    s32e	a0, a13, -16
    l32e	a0, a1, -12
    s32e	a1, a13, -12
    s32e	a2, a13, -8
    s32e	a3, a13, -4
    s32e	a4, a0, -48
    s32e	a5, a0, -44
    s32e	a6, a0, -40
    s32e	a7, a0, -36
    s32e	a8, a0, -32
    s32e	a9, a0, -28
    s32e	a10, a0, -24
    s32e	a11, a0, -20
    rfwo

.align 64
window_underflow_12:
/* windows underflow 12 - 0x140 */
    l32e	a0, a13, -16
    l32e	a1, a13, -12
    l32e	a2, a13, -8
    l32e	a11, a1, -12
    l32e	a3, a13, -4
    l32e	a4, a11, -48
    l32e	a5, a11, -44
    l32e	a6, a11, -40
    l32e	a7, a11, -36
    l32e	a8, a11, -32
    l32e	a9, a11, -28
    l32e	a10, a11, -24
    l32e	a11, a11, -20
    rfwu

/***********************************************************************************************************************
 * Interrupt levels
 **********************************************************************************************************************/

/*
 * interrupt level 2 - 0x180
 */
.type  interrupt_2,@function
.align 64
interrupt_2:
    rfi 2
.size interrupt_2, . - interrupt_2

/*
 * interrupt level 3 - 0x1c0
 */
.type  interrupt_3,@function
.align 64
interrupt_3:
    rfi 3
.size interrupt_3, . - interrupt_3

/*
 * interrupt level 4 - 0x200
 */
.type  interrupt_4,@function
.align 64
interrupt_4:
    rfi 4
.size interrupt_4, . - interrupt_4

/*
 * interrupt level 5 - 0x240
 */
.type  interrupt_5,@function
.align 64
interrupt_5:
    rfi 5
.size interrupt_5, . - interrupt_5

/*
 * interrupt level 6 (debug) - 0x280
 */
.type  nmi_interrupt,@function
.align 64
debug_interrupt:
    rfi 6
.size debug_interrupt, . - debug_interrupt

/*
 * interrupt level 7 (nmi) - 0x2c0
 */
.type  nmi_interrupt,@function
.align 64
nmi_interrupt:
    rfi 7
.size nmi_interrupt, . - nmi_interrupt


/***********************************************************************************************************************
 * Normal exceptions
 **********************************************************************************************************************/

#define EXCEPTION_REGS_SIZE     0
#define EXCEPTION_REGS_PS       0
#define EXCEPTION_REGS_SAR      0
#define EXCEPTION_REGS_LCOUNT   0
#define EXCEPTION_REGS_AREG0    0
#define EXCEPTION_REGS_AREG1    0
#define EXCEPTION_REGS_AREG2    0
#define EXCEPTION_REGS_AREG3    0
#define EXCEPTION_REGS_AREG4    0
#define EXCEPTION_REGS_AREG5    0
#define EXCEPTION_REGS_AREG6    0
#define EXCEPTION_REGS_AREG7    0
#define EXCEPTION_REGS_AREG8    0
#define EXCEPTION_REGS_AREG9    0
#define EXCEPTION_REGS_AREG10   0
#define EXCEPTION_REGS_AREG11   0
#define EXCEPTION_REGS_AREG12   0
#define EXCEPTION_REGS_AREG13   0
#define EXCEPTION_REGS_AREG14   0
#define EXCEPTION_REGS_AREG15   0

/*
 * kernel exception - 0x300
 */
.type  kernel_exception,@function
.align 64
 kernel_exception:
//    // save a2 and allocate the exception context on it
//    wsr a2, EXCSAVE1
//    addi a2, a1, -16 - EXCEPTION_REGS_SIZE
//
//    // save a0 and a3 to exception context
//    s32i a0, a2, EXCEPTION_REGS_AREG0
//    s32i a3, a2, EXCEPTION_REGS_AREG3
//
//    // lookup the handler based on the cause
//    rsr a0, EXCCAUSE
//    movi a3, g_raw_exception_handlers
//    addx a0, a0, a3
//    l32i a0, a0, 0
//
//    // jump to the handler
//    jx a0
    rfe
.size kernel_exception, . - kernel_exception

/*
 * user exception - 0x340
 */
.type  user_exception,@function
.align 64
user_exception:
    rfe
.size user_exception, . - user_exception

/*
 * double exception - 0x380
 */
.type  double_exception,@function
.align 64
double_exception:
    rfde
.size double_exception, . - double_exception

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Actual exception handlers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *  The raw exception handler for kernel exceptions
 */
//.type  raw_exception_handler,@function
//raw_exception_handler:
//    // save a1 and a2, a0 and a3 are already
//    // saved
//    rsr a0, EXCSAVE1
//    s32i a1, a2, EXCEPTION_REGS_AREG1
//    s32i a0, a2, EXCEPTION_REGS_AREG2
//    mov a1, a2
//
//    // save SAR
//    movi a2, 0
//    rsr a3, sar
//    s32i a3, a1, EXCEPTION_REGS_SAR
//
//    // rotate ws so that the current windowbase is at bit0
//    // assume ws = xxwww1yyyy. rotate ws right, so that a2 = yyyyxxwww1
//    rsr a2, WINDOWBASE
//    rsr a3, WINDOWSTART
//    ssr a2
//    slli a2, a3, 16
//    src a2, a3, a2
//    srli a2, a2, 16
//    s32i a2, a1, EXCEPTION_REGS_WINDOWMASK
//
//    // save only the live window-frame
//    _bbsi.l a2, 1, .common_exception
//    s32i a4, a1, EXCEPTION_REGS_AREG4
//    s32i a5, a1, EXCEPTION_REGS_AREG4
//    s32i a6, a1, EXCEPTION_REGS_AREG6
//    s32i a7, a1, EXCEPTION_REGS_AREG7
//    _bbsi.l a2, 2, .common_exception
//    s32i a8, a1, EXCEPTION_REGS_AREG8
//    s32i a9, a1, EXCEPTION_REGS_AREG9
//    s32i a10, a1, EXCEPTION_REGS_AREG10
//    s32i a11, a1, EXCEPTION_REGS_AREG11
//    _bbsi.l a2, 3, .common_exception
//    s32i a12, a1, EXCEPTION_REGS_AREG12
//    s32i a13, a1, EXCEPTION_REGS_AREG13
//    s32i a14, a1, EXCEPTION_REGS_AREG14
//    s32i a15, a1, EXCEPTION_REGS_AREG15
//
//    _bnei a2, 1, .common_exception
//    // copy spill slots of a0 and a1 to imitate movsp
//    // in order to keep exception stack continuous
//    l32i a3, a1, EXCEPTION_REGS_SIZE
//    l32i a0, a1, EXCEPTION_REGS_SIZE + 4
//    s32e a3, a1, -16
//    s32e a0, a1, -12
//
//.common_exception:
//    // save some registers and disable loops
//    movi a2, 0
//    xsr a2, LCOUNT
//    s32i a2, a1, EXCEPTION_REGS_LCOUNT
//
//    // now everything important is saved on the stack, so
//    // we can enable exceptions again
//    rsr a3, PS
//    s32i a3, a1, EXCEPTION_REGS_PS
//    // TODO: wtf is hapenning in here
//
//    // save lbeg and lend
//    rsr a4, LBEG
//    rsr a3, LEND
//    s32i a4, a1, EXCEPTION_REGS_LBEG
//    s32i a3, a1, EXCEPTION_REGS_LEND
//
//    // save atomic
//    rsr a3, SCOMPARE
//    s32i a3, a1, EXCEPTION_REGS_SCOMPARE1
//
//
//.size raw_exception_handler, . - raw_exception_handler

