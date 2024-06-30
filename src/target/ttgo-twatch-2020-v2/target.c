#include "hardware/i2c.h"

#include "util/log.h"
#include "drivers/axp202/axp202.h"
#include "hardware/dport.h"
#include "hardware/spi.h"
#include "drivers/st7789/st7789.h"
#include "drivers/ft6x06/ft6x06.h"
#include "task/timer.h"
#include "intrin.h"
#include "vectors.h"
#include "core-isa.h"
#include "task/timer_internal.h"

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
    delay(10);
    axp202_set_power_output(AXP202_CHANNEL_EXTEN, false);
    delay(8);
    axp202_set_power_output(AXP202_CHANNEL_EXTEN, true);

    ft6x06_init();

cleanup:
    return err;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// System time management - use ccount + high priority overflow
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * The amount of overflows we had in the timer
 */
static volatile uint32_t m_ccount_overflow = 0;

/**
 * The next tick we should dispatch the timers on
 */
static uint64_t m_next_tick = 0;

static void ccount_overflow_interrupt() {
    // increase the overflow
    m_ccount_overflow += 1;

    // re-arm the timer
    WSR(CCOMPARE2, 0);
}

/**
 * Initialize the system time management, just register the interrupt
 * and arm the timer
 */
static void init_system_time() {
    // overflow for keeping the time properly
    WSR(CCOMPARE2, 0);
    register_interrupt(XCHAL_TIMER2_INTERRUPT, ccount_overflow_interrupt);
}

/**
 * Get the system time properly
 */
uint64_t target_get_current_tick() {
    uint32_t high, low;
    do {
        high = m_ccount_overflow;
        low = RSR(CCOUNT);
    } while (high != m_ccount_overflow);
    return (((uint64_t)high << 32) | low) / 80;
}

void target_set_next_tick(uint64_t tick) {
    m_next_tick = tick;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Init routine
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
             80 * 1000 * 1000, SPI_DATA_MODE3);

    //
    // Initialize hardware
    //

    LOG_TRACE("Initializing hardware");
    CHECK_AND_RETHROW(init_power());
    CHECK_AND_RETHROW(init_display());
    CHECK_AND_RETHROW(init_touchscreen());

cleanup:
    if (IS_ERROR(err)) {
        LOG_CRITICAL("Failed to initialize hardware :(");
        while (1);
    }
}

void target_loop() {
    // TODO: use interrupt instead
    if (m_next_tick != 0) {
        if (target_get_current_tick() >= m_next_tick) {
            timer_dispatch();
        }
    }

    // TODO: go to sleep
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
