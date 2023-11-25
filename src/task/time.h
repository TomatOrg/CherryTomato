#pragma once

#include <stdint.h>

/**
 * Get a monotonic time in us units, this value is potentially reset between sleep states
 * so it should not be used for time tracking between sleep states
 */
uint64_t get_system_time();

/**
 * Delay execution for this amount of delays
 */
void udelay(uint32_t us);
