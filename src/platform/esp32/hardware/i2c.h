#pragma once

#include <stdint.h>
#include <stddef.h>

#include "gpio.h"
#include "util/except.h"

typedef struct i2c {
    void* regs;
    uint8_t sda;
    uint8_t scl;
    uint8_t cmd_index;
} i2c_t;

extern i2c_t g_i2c0;
extern i2c_t g_i2c1;

void i2c_init(i2c_t* i2c, uint8_t sda, uint8_t scl, uint32_t frequency);

err_t i2c_master_read(i2c_t* i2c, uint8_t addr, uint8_t* buffer, size_t length);
err_t i2c_master_write(i2c_t* i2c, uint8_t addr, const uint8_t* buffer, size_t length);
