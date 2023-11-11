
ENTRY(_start)

MEMORY {
    /* SRAM 0 - after the cache range */
    IRAM (RX) : ORIGIN = 0x40080000, len = 128K

    /* SRAM 2 - upper 128kb, after the newlib range */
    DRAM (RW) : ORIGIN = 0x3FFD0000, len = 128K
}

/* mmio hardware drivers need */
INCLUDE "src/platform/esp32/hardware/dport.lds"
INCLUDE "src/platform/esp32/hardware/gpio.lds"

/* builtin boot rom functions */
INCLUDE "src/platform/esp32/lds/bootrom.lds"

/* UART */
UART0_FIFO = 0x3FF40000;
UART0_INT_RAW = 0x3FF40004;
UART0_INT_ST = 0x3FF40008;
UART0_INT_ENA = 0x3FF4000c;
UART0_INT_CLR = 0x3FF40010;
UART0_STATUS = 0x3FF4001c;

/* Watch dogs */
TIMG0_Tx_WDTCONFIG0 = 0x3FF5F048;
TIMG1_Tx_WDTCONFIG0 = 0x3FF60048;
RTC_CNTL_WDTCONFIG0 = 0x3FF4808c;

SECTIONS {
    .text : {
        . = ALIGN(16);
        *(.text)
        *(.text.*)
    } > IRAM

    .rodata : {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata.*)
    } > DRAM

    .data : {
        . = ALIGN(4);
        *(.data)
        *(.data.*)
    } > DRAM

    .bss : {
        . = ALIGN(4);
        __start_bss = .;
        *(.bss)
        *(.bss.*)
        . = ALIGN(4);
        __end_bss = .;
    } > DRAM

}
