#include "hardware/i2c.h"

#include "util/log.h"
#include "drivers/axp202/axp202.h"
#include "hardware/dport.h"
#include "hardware/spi.h"
#include "drivers/st7789/st7789.h"
#include "drivers/ft6x06/ft6x06.h"
#include "task/time.h"
#include "intrin.h"
#include "vectors.h"
#include "core-isa.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pin assignments
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SENSOR_SDA  21
#define SENSOR_SCL  22

#define TOUCH_SDA   23
#define TOUCH_SCL   32

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
    CHECK_AND_RETHROW(axp202_set_power_output(AXP202_CHANNEL_EXTEN, false));

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

    // make sure dc is output
    gpio_enable_output(ST7789_DC, true);
    gpio_set_low(ST7789_CS);

    // initialize the display itself
    st7789_init();

    // turn on the backlight
    gpio_enable_output(ST7789_BL, true);
    gpio_set_high(ST7789_BL);

cleanup:
    return err;
}

static err_t init_touchscreen() {
    err_t err = NO_ERROR;

    axp202_set_power_output(AXP202_CHANNEL_EXTEN, true);
    udelay(10000);
    axp202_set_power_output(AXP202_CHANNEL_EXTEN, false);
    udelay(8000);
    axp202_set_power_output(AXP202_CHANNEL_EXTEN, true);

    ft6x06_init();

cleanup:
    return err;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// System time management - use ccount + high priority overflow
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static volatile uint32_t m_ccount_overflow = 0;

static void ccount_overflow_interrupt() {
    // increase the overflow
    m_ccount_overflow += 1;

    LOG_TRACE("GOT OVERFLOW");

    // re-arm the timer
    WSR(CCOMPARE2, 0xFFFFFFFF);
}

/**
 * Initialize the system time management, just register the interrupt
 * and arm the timer
 */
static void init_system_time() {
    register_interrupt(XCHAL_TIMER2_INTERRUPT, ccount_overflow_interrupt);
    WSR(CCOMPARE2, 0xFFFFFFFF);
}

/**
 * Get the system time properly
 */
uint64_t get_system_time() {
    uint32_t high, low;
    do {
        high = m_ccount_overflow;
        low = RSR(CCOUNT);
    } while (high != m_ccount_overflow);
    return (((uint64_t)high << 32) | low) / 80;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Init routine
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void test() {
    LOG_TRACE("LOL");
    WSR(CCOMPARE0, RSR(CCOUNT) + 80 * 1000000);
}

void target_entry(void) {
    err_t err = NO_ERROR;

    LOG_INFO("Target: TTGO-TWATCH-2020-V2");

    //
    // Misc platform init
    //

    init_system_time();

    // no reset please
    DPORT_PERIP_RST_EN.packed = 0;

    //
    // Reset all the hardware and gate it in
    //

    DPORT_PERIP_RST_EN.i2c_ext0 = 1;
    DPORT_PERIP_CLK_EN.i2c_ext0 = 1;
    DPORT_PERIP_RST_EN.i2c_ext0 = 0;

    DPORT_PERIP_RST_EN.i2c_ext1 = 1;
    DPORT_PERIP_CLK_EN.i2c_ext1 = 1;
    DPORT_PERIP_RST_EN.i2c_ext1 = 0;

    DPORT_PERIP_RST_EN.spi2 = 1;
    DPORT_PERIP_CLK_EN.spi2 = 1;
    DPORT_PERIP_RST_EN.spi2 = 0;

    //
    // Connect all the required busses
    //

    // sensor i2c
    i2c_init(&g_i2c0, SENSOR_SDA, SENSOR_SCL, 100 * 1000);

    // touch i2c
    i2c_init(&g_i2c1, TOUCH_SDA, TOUCH_SCL, 100 * 1000);

    // display spi
    spi_init(&g_spi2,
             ST7789_SCLK, ST7789_MOSI, INVALID_GPIO, ST7789_CS,
             80 * 1000 * 1000, SPI_DATA_MODE0);

    //
    // Initialize hardware
    //

    LOG_TRACE("Initializing hardware");

    register_interrupt(XCHAL_TIMER0_INTERRUPT, test);
    WSR(CCOMPARE0, RSR(CCOUNT) + 80 * 1000000);
    for(;;);

//    while (1) {
//        udelay(1000000);
//        LOG_TRACE("TICK! %08x", RSR(CCOUNT));
//    }
//    CHECK_AND_RETHROW(init_power());
//    CHECK_AND_RETHROW(init_touchscreen());
//    CHECK_AND_RETHROW(init_display());

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

err_t target_ft6x06_read_bytes(uint8_t addr, uint8_t* bytes, size_t length) { return i2c_master_read(&g_i2c1, addr, bytes, length); }
err_t target_ft6x06_write_bytes(uint8_t addr, const uint8_t* bytes, size_t length) { return i2c_master_write(&g_i2c1, addr, bytes, length); }

void target_st7789_gpio_dc_set_high() { gpio_set_high(ST7789_DC); }
void target_st7789_gpio_dc_set_low() { gpio_set_low(ST7789_DC); }
void target_st7789_write_byte(uint8_t byte) { spi_write_byte(&g_spi2, byte); }
void target_st7789_write_bytes(const uint8_t* bytes, size_t len) { spi_write(&g_spi2, bytes, len); }