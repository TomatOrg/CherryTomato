#pragma once

#include <stdint.h>
#include <stdbool.h>

void gpio_set_to_open_drain_output(uint8_t gpio_num);
void gpio_set_to_push_pull_output(uint8_t gpio_num);

void gpio_set_to_input(uint8_t gpio_num);
void gpio_enable_input(uint8_t gpio_num, bool on);


void gpio_internal_pull_up(uint8_t gpio_num, bool on);
void gpio_internal_pull_down(uint8_t gpio_num, bool on);

void gpio_enable_output(uint8_t gpio_num, bool on);
void gpio_set_output_high(uint8_t gpio_num, bool high);
void gpio_set_high(uint8_t gpio_num);
void gpio_set_low(uint8_t gpio_num);

typedef struct gpio_signal {
    uint8_t function;
    uint8_t signal;
} gpio_signal_t;

#define GPIO_I2CEXT0_SCL    ((gpio_signal_t){ .function = 2, .signal = 29 })
#define GPIO_I2CEXT0_SDA    ((gpio_signal_t){ .function = 2, .signal = 30 })
#define GPIO_I2CEXT1_SCL    ((gpio_signal_t){ .function = 2, .signal = 95 })
#define GPIO_I2CEXT1_SDA    ((gpio_signal_t){ .function = 2, .signal = 96 })

#define GPIO_SPICLK         ((gpio_signal_t){ .function = 1, .signal = 0 })
#define GPIO_SPIQ           ((gpio_signal_t){ .function = 1, .signal = 1 })
#define GPIO_SPID           ((gpio_signal_t){ .function = 1, .signal = 2 })
#define GPIO_SPIHD          ((gpio_signal_t){ .function = 1, .signal = 3 })
#define GPIO_SPIWP          ((gpio_signal_t){ .function = 1, .signal = 4 })
#define GPIO_SPICS0         ((gpio_signal_t){ .function = 1, .signal = 5 })
#define GPIO_SPICS1         ((gpio_signal_t){ .function = 1, .signal = 6 })
#define GPIO_SPICS2         ((gpio_signal_t){ .function = 1, .signal = 7 })

#define GPIO_HSPICLK        ((gpio_signal_t){ .function = 1, .signal = 8 })
#define GPIO_HSPIQ          ((gpio_signal_t){ .function = 1, .signal = 9 })
#define GPIO_HSPID          ((gpio_signal_t){ .function = 1, .signal = 10 })
#define GPIO_HSPIHD         ((gpio_signal_t){ .function = 1, .signal = 12 })
#define GPIO_HSPIWP         ((gpio_signal_t){ .function = 1, .signal = 13 })
#define GPIO_HSPICS0        ((gpio_signal_t){ .function = 1, .signal = 11 })
#define GPIO_HSPICS1        ((gpio_signal_t){ .function = 1, .signal = 61 })
#define GPIO_HSPICS2        ((gpio_signal_t){ .function = 1, .signal = 62 })

#define GPIO_VSPICLK        ((gpio_signal_t){ .function = 1, .signal = 63 })
#define GPIO_VSPIQ          ((gpio_signal_t){ .function = 1, .signal = 64 })
#define GPIO_VSPID          ((gpio_signal_t){ .function = 1, .signal = 65 })
#define GPIO_VSPIHD         ((gpio_signal_t){ .function = 1, .signal = 66 })
#define GPIO_VSPIWP         ((gpio_signal_t){ .function = 1, .signal = 67 })
#define GPIO_VSPICS0        ((gpio_signal_t){ .function = 1, .signal = 68 })
#define GPIO_VSPICS1        ((gpio_signal_t){ .function = 1, .signal = 69 })
#define GPIO_VSPICS2        ((gpio_signal_t){ .function = 1, .signal = 70 })

void gpio_connect_peripheral_to_output(uint8_t gpio_num, gpio_signal_t signal);
void gpio_connect_input_to_peripheral(uint8_t gpio_num, gpio_signal_t signal);
