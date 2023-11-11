#pragma once

#include "util/except.h"

#include <stdint.h>
#include <stdbool.h>

/**
 * Initialize the AXP20x driver
 */
err_t axp202_init();

typedef enum axp202_ldo4 {
    AXP202_LDO4_1250MV = 0,
    AXP202_LDO4_1300MV = 1,
    AXP202_LDO4_1400MV = 2,
    AXP202_LDO4_1500MV = 3,
    AXP202_LDO4_1600MV = 4,
    AXP202_LDO4_1700MV = 5,
    AXP202_LDO4_1800MV = 6,
    AXP202_LDO4_1900MV = 7,
    AXP202_LDO4_2000MV = 8,
    AXP202_LDO4_2500MV = 9,
    AXP202_LDO4_2700MV = 10,
    AXP202_LDO4_2800MV = 11,
    AXP202_LDO4_3000MV = 12,
    AXP202_LDO4_3100MV = 13,
    AXP202_LDO4_3200MV = 14,
    AXP202_LDO4_3300MV = 15,
} axp202_ldo4_t;

#define AXP202_LD02_MV_STEP 100
#define AXP202_LD02_MV_MIN  1800
#define AXP202_LD02_MV_MAX  3300

/**
 * Set the LDO2 output voltage, in mV units, with 100mV
 * steps from 1.8V to 3.3V
 */
err_t axp202_set_ldo2_voltage(uint16_t mv);

/**
 * Set the LDO4 output voltage, must be one of the given
 * options
 */
err_t axp202_set_ldo4_voltage(axp202_ldo4_t mv);

typedef enum axp202_vbus_current_limit {
    AXP202_VBUS_CURRENT_LIMIT_900MA = 0b00,
    AXP202_VBUS_CURRENT_LIMIT_500MA = 0b01,
    AXP202_VBUS_CURRENT_LIMIT_100MA = 0b10,
    AXP202_VBUS_CURRENT_LIMIT_NONE = 0b11,
} axp202_vbus_current_limit_t;

/**
 * Turns off the VBUS current limiting
 */
err_t axp202_set_vbus_current_limit(axp202_vbus_current_limit_t limit);

typedef enum axp202_channel {
    AXP202_CHANNEL_EXTEN = 0,
    AXP202_CHANNEL_DCDC3 = 1,
    AXP202_CHANNEL_LDO2 = 2,
    AXP202_CHANNEL_LDO4 = 3,
    AXP202_CHANNEL_DCDC2 = 4,
    AXP202_CHANNEL_LDO3 = 6,
} axp202_channel_t;

/**
 * Enable/Disable the given power output channel
 */
err_t axp202_set_power_output(axp202_channel_t channel, bool on);

// implemented by the platform
err_t target_axp202_read_bytes(uint8_t addr, uint8_t* bytes, size_t length);
err_t target_axp202_write_bytes(uint8_t addr, const uint8_t* bytes, size_t length);

