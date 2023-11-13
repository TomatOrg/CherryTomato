#include "hardware/i2c.h"

#include "util/log.h"
#include "drivers/axp202/axp202.h"
#include "hardware/dport.h"
#include "hardware/spi.h"
#include "drivers/st7789/st7789.h"
#include "event/delay.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pin assignments
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SENSOR_SDA  21
#define SENSOR_SCL  22

#define ST7789_MOSI 19
#define ST7789_SCLK 18
#define ST7789_DC   27
#define ST7789_CS   5

#define ST7789_BL   25

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Hardware initialization
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static err_t init_power() {
    err_t err = NO_ERROR;

    // setup the power controller
    CHECK_AND_RETHROW(axp202_init());

    // make sure its off
    CHECK_AND_RETHROW(axp202_set_power_output(AXP202_CHANNEL_LDO3, false));
    CHECK_AND_RETHROW(axp202_set_power_output(AXP202_CHANNEL_LDO2, false));

    // Set the backlight and st7789 voltage to 3.3
    CHECK_AND_RETHROW(axp202_set_ldo2_voltage(3300));
    CHECK_AND_RETHROW(axp202_set_ldo3_volatge(3300));

    // disable current limiting
    // TODO: why?
    CHECK_AND_RETHROW(axp202_set_vbus_current_limit(AXP202_VBUS_CURRENT_LIMIT_NONE));

    // enable backlight and tft/touch power output
    CHECK_AND_RETHROW(axp202_set_power_output(AXP202_CHANNEL_LDO2, true));
    CHECK_AND_RETHROW(axp202_set_power_output(AXP202_CHANNEL_LDO3, true));

cleanup:
    return err;
}

static err_t init_display() {
    err_t err = NO_ERROR;

    // turn on the backlight
    gpio_enable_output(ST7789_BL, true);
    gpio_set_high(ST7789_BL);

    // make sure dc is output
    gpio_enable_output(ST7789_DC, true);
    gpio_set_low(ST7789_CS);
    gpio_set_low(ST7789_CS);

    // initialize the display itself
    st7789_init();

cleanup:
    return err;
}

void target_entry(void) {
    err_t err = NO_ERROR;

    LOG_INFO("Target: TTGO-TWATCH-2020-V2");

    // enable needed clocks
    DPROT_PERIP_REG perip_clk_en = DPORT_PERIP_CLK_EN;
    perip_clk_en.i2c_ext0 = 1;      // sensor i2c
    perip_clk_en.spi2 = 1;          // display spi
    DPORT_PERIP_CLK_EN = perip_clk_en;

    // sensor i2c
    i2c_init(&g_i2c0, SENSOR_SDA, SENSOR_SCL, 100 * 1000);

    // display spi
    spi_init(&g_spi2,
             ST7789_SCLK, ST7789_MOSI, INVALID_GPIO, ST7789_CS,
             8 * 1000 * 1000, SPI_DATA_MODE3);

    // now probe all the ttgo hardware
    LOG_TRACE("Initializing drivers");
    CHECK_AND_RETHROW(init_power());
    CHECK_AND_RETHROW(init_display());

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

void target_st7789_gpio_dc_set_high() { gpio_set_high(ST7789_DC); }
void target_st7789_gpio_dc_set_low() { gpio_set_low(ST7789_DC); }
void target_st7789_write_byte(uint8_t byte) { spi_write_byte(&g_spi2, byte); }
void target_st7789_write_bytes(uint8_t* byte, size_t len) { spi_write(&g_spi2, byte, len); }