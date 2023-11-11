#include "hardware/i2c.h"

#include "util/log.h"
#include "drivers/axp202/axp202.h"
#include "hardware/dport.h"

static err_t init_power() {
    err_t err = NO_ERROR;

    // setup the power controller
    CHECK_AND_RETHROW(axp202_init());

    // The backlight is set on ldo2, set its voltage to 3.3v
    CHECK_AND_RETHROW(axp202_set_ldo2_voltage(3300));

    // disable current limiting
    // TODO: why?
    CHECK_AND_RETHROW(axp202_set_vbus_current_limit(AXP202_VBUS_CURRENT_LIMIT_NONE));

    // enable backlight and tft/touch power output
    CHECK_AND_RETHROW(axp202_set_power_output(AXP202_CHANNEL_LDO2, true));
    CHECK_AND_RETHROW(axp202_set_power_output(AXP202_CHANNEL_LDO3, true));

cleanup:
    return err;
}

void target_entry(void) {
    err_t err = NO_ERROR;

    LOG_INFO("Target: TTGO-TWATCH-2020-V2");

    // enable needed clocks
    DPORT_PERIP_CLK_EN.i2c_ext0 = 1;
    DPORT_PERIP_CLK_EN.ledc = 1;

    // Setup all the esp32 hardware
    LOG_TRACE("Init sensor i2c");
    i2c_init(&g_i2c0, 21, 22, 100 * 1000);

    // now probe all the ttgo hardware
    LOG_TRACE("Initializing drivers");
    CHECK_AND_RETHROW(init_power());

cleanup:
    if (IS_ERROR(err)) {
        LOG_CRITICAL("Failed to initialize hardware :(");
        while (1);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stubs for the drivers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

err_t target_axp202_read_bytes(uint8_t addr, uint8_t* bytes, size_t length) { return i2c_master_read(&g_i2c0, addr, bytes, length); }
err_t target_axp202_write_bytes(uint8_t addr, const uint8_t* bytes, size_t length) { return i2c_master_write(&g_i2c0, addr, bytes, length); }
