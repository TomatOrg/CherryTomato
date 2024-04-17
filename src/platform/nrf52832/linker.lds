
ENTRY(_start)

MEMORY {
    IRAM (RX) : ORIGIN = 0x20000000, len = 64K
    DRAM (RW) : ORIGIN = 0x00000000, len = 512K
}

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
    } > IRAM

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
