#include "intrin.h"
#include "util/libc.h"
#include "hardware/uart.h"
#include "vectors.h"
#include "apps/entry.h"
#include "target/target.h"
#include "hardware/dport.h"

#include <util/log.h>
#include "task/time.h"

// rom functions needed for ram init
void mmu_init(int cpu_no);

extern char __start_bss[];
extern char __end_bss[];

extern volatile uint32_t TIMG0_Tx_WDTCONFIG0;
extern volatile uint32_t TIMG1_Tx_WDTCONFIG0;
extern volatile uint32_t RTC_CNTL_WDTCONFIG0;

/**
 * We are going to allocate a nice big stack for us to use
 * TODO: place somehwere so it won't fuck us in case of a stack overflow
 */
__attribute__((aligned(16)))
char g_cpu0_stack[8192];

/**
 * Disable watchdogs so we are not
 * gonna die from them
 */
static void disable_watchdogs() {
    TIMG0_Tx_WDTCONFIG0 = 0;
    TIMG1_Tx_WDTCONFIG0 = 0;
    RTC_CNTL_WDTCONFIG0 = 0;
}

/**
 * Initialize uart, just disable the interrupts
 */
void init_uart() {
    UART0_INT_ENA.packed = 0;
}

/**
 * Implement putchar for logging
 */
void putchar_(char c) {
    if (c == '\t') {
        putchar_(' ');
        putchar_(' ');
        putchar_(' ');
        putchar_(' ');
    } else {
        // wait until the tx fifo has space
        while (UART0_STATUS.txfifo_cnt >= 128);
        UART0_FIFO = (uint32_t)c;
    }
}

/**
 * Configure all the basic esp stuff
 */
static void esp_init() {
    // setup illegal regions as illegal
    static const uint32_t illegal_regions[] = {
        0x00000000,
        0x80000000,
        0xa0000000,
        0xc0000000,
        0xe0000000
    };
    for (size_t i = 0; i < ARRAY_LENGTH(illegal_regions); i++) {
        WITLB(0xF, illegal_regions[i]);
        WDTLB(0xF, illegal_regions[i]);
    }

    // setup the main region as cached no allocate
    WDTLB(0x0, 0x20000000);

    // setup ESP32 caching
    // TODO: cache read disable
    // TODO: cache flush
    // TODO: mmu_init

    // reset xtensa timers
    WSR(CCOMPARE0, 0);
    WSR(CCOMPARE1, 0);
    WSR(CCOMPARE2, 0);
    __isync();

    // setup the interrupt vector
    WSR(VECBASE, __vecbase);
}

void entry() {
    // clear bss
    memset(__start_bss, 0, __end_bss - __start_bss);

    // early setup, configure everything we need
    esp_init();
    disable_watchdogs();
    init_uart();

    // initialize the uart (just makes sure no interrupts are fired)
    LOG_INFO("~~~ Cherry Tomato (ESP32) ~~~");

    // setup basic clocks
    DPROT_PERIP_REG clk_en = {
        .uart_mem = 1,
        .uart = 1,
    };
    DPORT_PERIP_CLK_EN = clk_en;

    // setup a clean PS
    WSR(PS, BIT18);

    // enable interrupts
    enable_interrupts();

    // call the target
    target_entry();

    // setup the timer interrupt
    cherry_tomato_entry();
}


void start_event_loop(loop_fn_t* fn, void* arg) {
    while (true) {
        uint64_t starttime = get_system_time();
        fn(arg);
        uint64_t after = get_system_time();
        uint64_t delta = after - starttime;
        if (delta < 16 * 1000) { udelay(16 * 1000 - delta); }
    }
}