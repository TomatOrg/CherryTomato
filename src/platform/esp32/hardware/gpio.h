#pragma once

#include <stdint.h>
#include <stdbool.h>

void gpio_set_to_open_drain_output(uint8_t gpio_num);
void gpio_set_to_push_pull_output(uint8_t gpio_num);

void gpio_set_output_high(uint8_t gpio_num, bool high);

void gpio_set_to_input(uint8_t gpio_num);
void gpio_enable_input(uint8_t gpio_num, bool on);

void gpio_internal_pull_up(uint8_t gpio_num, bool on);
void gpio_internal_pull_down(uint8_t gpio_num, bool on);

void gpio_set_high(uint8_t gpio_num);
void gpio_set_low(uint8_t gpio_num);

typedef struct gpio_signal {
    uint8_t num;
    uint8_t signal;
} gpio_signal_t;

#define GPIO_I2CEXT0_SCL    ((gpio_signal_t){ .num = 2, .signal = 29 })
#define GPIO_I2CEXT0_SDA    ((gpio_signal_t){ .num = 2, .signal = 30 })
#define GPIO_I2CEXT1_SCL    ((gpio_signal_t){ .num = 2, .signal = 95 })
#define GPIO_I2CEXT1_SDA    ((gpio_signal_t){ .num = 2, .signal = 96 })

#define LEDC_HS_SIG(x)        ((gpio_signal_t){ .num = 2, .signal = 71 + (x) })
#define LEDC_LS_SIG(x)        ((gpio_signal_t){ .num = 2, .signal = 79 + (x) })

void gpio_connect_peripheral_to_output(uint8_t gpio_num, gpio_signal_t signal);
void gpio_connect_input_to_peripheral(uint8_t gpio_num, gpio_signal_t signal);
