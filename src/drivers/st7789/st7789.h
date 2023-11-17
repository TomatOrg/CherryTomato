#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "event/token.h"

void st7789_init();


// implemented by the platform
void target_st7789_gpio_dc_set_high();
void target_st7789_gpio_dc_set_low();
void target_st7789_write_byte(uint8_t byte);
void target_st7789_write_bytes(const uint8_t* bytes, size_t len, event_t* event);
