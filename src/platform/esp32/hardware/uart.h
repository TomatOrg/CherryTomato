#pragma once

#include <util/defs.h>

typedef union _UART_STATUS_REG {
    struct {
        uint32_t rxfifo_cnt : 8;
        uint32_t st_urx_out : 4;
        uint32_t _reserved0 : 1;
        uint32_t dsrn : 1;
        uint32_t ctsn : 1;
        uint32_t rxd : 1;
        uint32_t txfifo_cnt : 8;
        uint32_t st_utx_out : 4;
        uint32_t _reserved1 : 1;
        uint32_t dtrn : 1;
        uint32_t rtsn : 1;
        uint32_t txd : 1;
    };
    uint32_t packed;
} MMIO UART_STATUS_REG;
STATIC_ASSERT(sizeof(UART_STATUS_REG) == sizeof(uint32_t));

typedef union _UART_INT_REG {
    struct {
        uint32_t rxfifo_full : 1;
        uint32_t txfifo_empty : 1;
        uint32_t parity_err : 1;
        uint32_t frm_err : 1;
        uint32_t rxfifo_ovf : 1;
        uint32_t dsr_chg : 1;
        uint32_t cts_chg : 1;
        uint32_t brk_det : 1;
        uint32_t rxfifo_tout : 1;
        uint32_t sw_xon : 1;
        uint32_t sw_xoff : 1;
        uint32_t glitch_det : 1;
        uint32_t tx_brk_done : 1;
        uint32_t tx_brk_idle : 1;
        uint32_t tx_done : 1;
        uint32_t rs485_parity_err : 1;
        uint32_t rs485_frm_err : 1;
        uint32_t rs485_clash : 1;
        uint32_t at_cmd_char_det : 1;
        uint32_t _reserved : 13;
    };
    uint32_t packed;
} MMIO UART_INT_REG;
STATIC_ASSERT(sizeof(UART_INT_REG) == sizeof(uint32_t));

extern volatile UART_INT_REG UART0_INT_RAW;
extern volatile UART_INT_REG UART0_INT_ST;
extern volatile UART_INT_REG UART0_INT_ENA;
extern volatile UART_INT_REG UART0_INT_CLR;

extern volatile uint32_t UART0_FIFO;
extern volatile UART_STATUS_REG UART0_STATUS;

