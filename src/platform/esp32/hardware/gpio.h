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

//typedef struct gpio_signal {
//    uint8_t function;
//    uint8_t signal;
//} gpio_signal_t;

#define GPIO_I2CEXT0_SCL    (29)
#define GPIO_I2CEXT0_SDA    (30)
#define GPIO_I2CEXT1_SCL    (95)
#define GPIO_I2CEXT1_SDA    (96)

#define GPIO_SPICLK         (0)
#define GPIO_SPIQ           (1)
#define GPIO_SPID           (2)
#define GPIO_SPIHD          (3)
#define GPIO_SPIWP          (4)
#define GPIO_SPICS0         (5)
#define GPIO_SPICS1         (6)
#define GPIO_SPICS2         (7)

#define GPIO_HSPICLK        (8)
#define GPIO_HSPIQ          (9)
#define GPIO_HSPID          (10)
#define GPIO_HSPIHD         (12)
#define GPIO_HSPIWP         (13)
#define GPIO_HSPICS0        (11)
#define GPIO_HSPICS1        (61)
#define GPIO_HSPICS2        (62)

#define GPIO_VSPICLK        (63)
#define GPIO_VSPIQ          (64)
#define GPIO_VSPID          (65)
#define GPIO_VSPIHD         (66)
#define GPIO_VSPIWP         (67)
#define GPIO_VSPICS0        (68)
#define GPIO_VSPICS1        (69)
#define GPIO_VSPICS2        (70)

void gpio_connect_peripheral_to_output(uint8_t gpio_num, uint8_t signal);
void gpio_connect_input_to_peripheral(uint8_t gpio_num, uint8_t signal);
