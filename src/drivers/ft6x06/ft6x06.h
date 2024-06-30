#pragma once

#include "util/except.h"

#include <stdint.h>
#include <stdbool.h>


err_t ft6x06_init();

/**
 * Read the current touch event
 *
 * @param touched
 * @param x
 * @param y
 * @return
 */
err_t ft6x06_touch(bool* touched, uint16_t* x, uint16_t* y);

err_t target_ft6x06_read_bytes(uint8_t addr, uint8_t* bytes, size_t length);
err_t target_ft6x06_write_bytes(uint8_t addr, const uint8_t* bytes, size_t length);

