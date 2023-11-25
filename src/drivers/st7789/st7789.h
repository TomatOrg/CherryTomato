#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void st7789_init();
void st7789_fillrect(uint16_t col, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void st7789_blit(uint16_t* buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void st7789_set_vertical_scrolloff(uint16_t scrolloff);

// implemented by the platform
void target_st7789_gpio_dc_set_high();
void target_st7789_gpio_dc_set_low();
void target_st7789_write_byte(uint8_t byte);
void target_st7789_write_bytes(const uint8_t* bytes, size_t len);
