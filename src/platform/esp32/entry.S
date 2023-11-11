.section .text

.extern entry
.extern g_cpu0_stack

.global _start
.align 16
_start:
    // reset the windowbase and windowstart for function calls
    movi a1, 1
    movi a0, 0
    wsr.windowstart a1
    wsr.windowbase a0
    rsync

    // clear loop count
    movi a0, 0
    wsr.lcount a0
    rsync

    // set the stack and enable register window, we are going to use
    // the app cpu stack for now, just so we will not have a shared stack
    // between the interrupt handlers and the init stack
    movi a0, 8192
    movi sp, g_cpu0_stack
    add sp, sp, a0

    // jump to kernel
    call4 entry