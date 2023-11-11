#pragma once

#include <stdint.h>

#include <util/defs.h>

typedef union {
    struct {
        uint32_t : 1;
        uint32_t spio01 : 1;
        uint32_t uart : 1;
        uint32_t : 1;
        uint32_t i2s0 : 1;
        uint32_t uart1 : 1;
        uint32_t spi2 : 1;
        uint32_t i2c_ext0 : 1;
        uint32_t uhci0 : 1;
        uint32_t rmt : 1;
        uint32_t pcnt : 1;
        uint32_t ledc : 1;
        uint32_t uchi1 : 1;
        uint32_t timergroup : 1;
        uint32_t efuse : 1;
        uint32_t timergroup1 : 1;
        uint32_t spi3 : 1;
        uint32_t pwm0 : 1;
        uint32_t i2c_ext1 : 1;
        uint32_t twai : 1;
        uint32_t pwm1 : 1;
        uint32_t i2s1 : 1;
        uint32_t spi_dma : 1;
        uint32_t uart2 : 1;
        uint32_t uart_mem : 1;
        uint32_t : 7;
    };
    uint32_t packed;
} MMIO DPROT_PERIP_REG;
STATIC_ASSERT(sizeof(DPROT_PERIP_REG) == sizeof(uint32_t));

extern volatile DPROT_PERIP_REG DPORT_PERIP_CLK_EN;
extern volatile DPROT_PERIP_REG DPORT_PERIP_RST_EN;
