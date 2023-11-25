#pragma once

// for 40MHz crystal
#define APB_CLOCK_HZ (80 * 1000 * 1000)
#define XTAL_CLOCK_HZ (40 * 1000 * 1000)
#define I2C_CLOCK_HZ (80 * 1000 * 1000)
#define PWM_CLOCK_HZ (160 * 1000 * 1000)

extern void* xthal_memcpy(void *dst, const void *src, unsigned len);
