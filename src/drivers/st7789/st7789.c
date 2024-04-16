#include <string.h>
#include "st7789.h"
#include "util/log.h"
#include "hardware/gpio.h"
#include "task/time.h"

#define ST7789_NOP			0x00
#define ST7789_SWRESET		0x01
#define ST7789_RDDID		0x04
#define ST7789_RDDST		0x09

#define ST7789_RDDPM		0x0A      // Read display power mode
#define ST7789_RDD_MADCTL	0x0B      // Read display MADCTL
#define ST7789_RDD_COLMOD	0x0C      // Read display pixel format
#define ST7789_RDDIM		0x0D      // Read display image mode
#define ST7789_RDDSM		0x0E      // Read display signal mode
#define ST7789_RDDSR		0x0F      // Read display self-diagnostic result (ST7789V)

#define ST7789_SLPIN		0x10
#define ST7789_SLPOUT		0x11
#define ST7789_PTLON		0x12
#define ST7789_NORON		0x13

#define ST7789_INVOFF		0x20
#define ST7789_INVON		0x21
#define ST7789_GAMSET		0x26      // Gamma set
#define ST7789_DISPOFF		0x28
#define ST7789_DISPON		0x29
#define ST7789_CASET		0x2A
#define ST7789_RASET		0x2B
#define ST7789_RAMWR		0x2C
#define ST7789_RGBSET		0x2D      // Color setting for 4096, 64K and 262K colors
#define ST7789_RAMRD		0x2E

#define ST7789_PTLAR		0x30
#define ST7789_VSCRDEF		0x33      // Vertical scrolling definition (ST7789V)
#define ST7789_TEOFF		0x34      // Tearing effect line off
#define ST7789_TEON			0x35      // Tearing effect line on
#define ST7789_MADCTL		0x36      // Memory data access control
#define ST7789_VSCSAD		0x37      // Vertical scroll start address of RAM
#define ST7789_IDMOFF		0x38      // Idle mode off
#define ST7789_IDMON		0x39      // Idle mode on
#define ST7789_RAMWRC		0x3C      // Memory write continue (ST7789V)
#define ST7789_RAMRDC		0x3E      // Memory read continue (ST7789V)
#define ST7789_COLMOD		0x3A

#define ST7789_RAMCTRL		0xB0      // RAM control
#define ST7789_RGBCTRL		0xB1      // RGB control
#define ST7789_PORCTRL		0xB2      // Porch control
#define ST7789_FRCTRL1		0xB3      // Frame rate control
#define ST7789_PARCTRL		0xB5      // Partial mode control
#define ST7789_GCTRL		0xB7      // Gate control
#define ST7789_GTADJ		0xB8      // Gate on timing adjustment
#define ST7789_DGMEN		0xBA      // Digital gamma enable
#define ST7789_VCOMS		0xBB      // VCOMS setting
#define ST7789_LCMCTRL		0xC0      // LCM control
#define ST7789_IDSET		0xC1      // ID setting
#define ST7789_VDVVRHEN		0xC2      // VDV and VRH command enable
#define ST7789_VRHS			0xC3      // VRH set
#define ST7789_VDVSET		0xC4      // VDV setting
#define ST7789_VCMOFSET		0xC5      // VCOMS offset set
#define ST7789_FRCTR2		0xC6      // FR Control 2
#define ST7789_CABCCTRL		0xC7      // CABC control
#define ST7789_REGSEL1		0xC8      // Register value section 1
#define ST7789_REGSEL2		0xCA      // Register value section 2
#define ST7789_PWMFRSEL		0xCC      // PWM frequency selection
#define ST7789_PWCTRL1		0xD0      // Power control 1
#define ST7789_VAPVANEN		0xD2      // Enable VAP/VAN signal output
#define ST7789_CMD2EN		0xDF      // Command 2 enable
#define ST7789_PVGAMCTRL	0xE0      // Positive voltage gamma control
#define ST7789_NVGAMCTRL	0xE1      // Negative voltage gamma control
#define ST7789_DGMLUTR		0xE2      // Digital gamma look-up table for red
#define ST7789_DGMLUTB		0xE3      // Digital gamma look-up table for blue
#define ST7789_GATECTRL		0xE4      // Gate control
#define ST7789_SPI2EN		0xE7      // SPI2 enable
#define ST7789_PWCTRL2		0xE8      // Power control 2
#define ST7789_EQCTRL		0xE9      // Equalize time control
#define ST7789_PROMCTRL		0xEC      // Program control
#define ST7789_PROMEN		0xFA      // Program mode enable
#define ST7789_NVMSET		0xFC      // NVM setting
#define ST7789_PROMACT		0xFE      // Program action

/**
 * Utility to send a command, sets the DC to low, then sends the command,
 * and then to high  again, being ready for data
 */
static void st7789_write_command(uint8_t c) {
    target_st7789_gpio_dc_set_low();
    target_st7789_write_byte(c);
    target_st7789_gpio_dc_set_high();
}

static void st7789_init_color_format() {
    st7789_write_command(ST7789_MADCTL);
    target_st7789_write_byte(0x08);

    st7789_write_command(0xB6);
    target_st7789_write_byte(0x0A);
    target_st7789_write_byte(0x82);

    st7789_write_command(ST7789_COLMOD);
    target_st7789_write_byte(0x55);
    udelay(10000);
}

static void st7789_init_frame_rate() {
    st7789_write_command(ST7789_PORCTRL);
    target_st7789_write_byte(0x0c);
    target_st7789_write_byte(0x0c);
    target_st7789_write_byte(0x00);
    target_st7789_write_byte(0x33);
    target_st7789_write_byte(0x33);

    st7789_write_command(ST7789_GCTRL);
    target_st7789_write_byte(0x35);
}

static void st7789_init_power() {
    st7789_write_command(ST7789_VCOMS);
    target_st7789_write_byte(0x28);

    st7789_write_command(ST7789_LCMCTRL);
    target_st7789_write_byte(0x0c);

    st7789_write_command(ST7789_VDVVRHEN);
    target_st7789_write_byte(0x01);
    target_st7789_write_byte(0xFF);

    st7789_write_command(ST7789_VRHS);
    target_st7789_write_byte(0x10);

    st7789_write_command(ST7789_VDVSET);
    target_st7789_write_byte(0x20);

    st7789_write_command(ST7789_FRCTR2);
    target_st7789_write_byte(0x0F);

    st7789_write_command(ST7789_PWCTRL1);
    target_st7789_write_byte(0xa4);
    target_st7789_write_byte(0xa1);
}

static void st7789_init_gamma() {
    static uint8_t pvgamctrl[] = {
        0xd0, 0x00, 0x02, 0x07, 0x0a, 0x28, 0x32, 0x44, 0x42,
        0x06, 0x0e, 0x12, 0x14, 0x17,
    };
    st7789_write_command(ST7789_PVGAMCTRL);
    for (int i = 0; i < sizeof(pvgamctrl); i++) {
        target_st7789_write_byte(pvgamctrl[i]);
    }

    static uint8_t nvgamctrl[] = {
        0xd0, 0x00, 0x02, 0x07, 0x0a, 0x28, 0x31, 0x54, 0x47,
        0x0e, 0x1c, 0x17, 0x1b, 0x1e,
    };
    st7789_write_command(ST7789_NVGAMCTRL);
    for (int i = 0; i < sizeof(nvgamctrl); i++) {
        target_st7789_write_byte(nvgamctrl[i]);
    }
}

void st7789_init() {
    LOG_TRACE("\t--- ST7789 ---");

    // reset the display
    st7789_write_command(ST7789_SWRESET);
    udelay(150000);

    // put out of sleep
    st7789_write_command(ST7789_SLPOUT);
    udelay(120000);

    // normal display mode
    st7789_write_command(ST7789_NORON);

    st7789_init_color_format();
    st7789_init_frame_rate();
    st7789_init_power();
    st7789_init_gamma();

    st7789_write_command(ST7789_INVON);

    st7789_write_command(ST7789_CASET);
    target_st7789_write_byte(0x00);
    target_st7789_write_byte(0x00);
    target_st7789_write_byte(0x00);
    target_st7789_write_byte(0xEF);

    st7789_write_command(ST7789_RASET);
    target_st7789_write_byte(0x00);
    target_st7789_write_byte(0x00);
    target_st7789_write_byte(0x01);
    target_st7789_write_byte(0x3F);

    st7789_write_command(ST7789_RAMWR);
    for (int i = 0; i < 240 * 320; i++) {
        target_st7789_write_byte(0);
        target_st7789_write_byte(0);
    }

    // wait a bit for commands to take effect
    udelay(120000);

    // put the display on
    st7789_write_command(ST7789_DISPON);
}

void st7789_fillrect(uint16_t col, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    uint16_t xend = x + w - 1;
    uint16_t yend = y + h - 1;

    st7789_write_command(ST7789_CASET);
    target_st7789_write_byte(x >> 8);
    target_st7789_write_byte(x & 0xFF);
    target_st7789_write_byte(xend >> 8);
    target_st7789_write_byte(xend & 0xFF);

    st7789_write_command(ST7789_RASET);
    target_st7789_write_byte(y >> 8);
    target_st7789_write_byte(y & 0xFF);
    target_st7789_write_byte(yend >> 8);
    target_st7789_write_byte(yend & 0xFF);

    st7789_write_command(ST7789_RAMWR);
    for (int i = 0; i < w * h; i++) {
        target_st7789_write_byte(col >> 8);
        target_st7789_write_byte(col & 0xFF);
    }
}

void st7789_blit(uint16_t* buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    uint16_t xend = x + w - 1;
    uint16_t yend = y + h - 1;

    st7789_write_command(ST7789_CASET);
    target_st7789_write_byte(x >> 8);
    target_st7789_write_byte(x & 0xFF);
    target_st7789_write_byte(xend >> 8);
    target_st7789_write_byte(xend & 0xFF);

    st7789_write_command(ST7789_RASET);
    target_st7789_write_byte(y >> 8);
    target_st7789_write_byte(y & 0xFF);
    target_st7789_write_byte(yend >> 8);
    target_st7789_write_byte(yend & 0xFF);

    st7789_write_command(ST7789_RAMWR);
    target_st7789_write_bytes((const uint8_t*)buffer, w * h * 2);
}

void st7789_set_vertical_scrolloff(uint16_t scrolloff) {
    st7789_write_command(ST7789_VSCSAD);
    target_st7789_write_byte(scrolloff >> 8);
    target_st7789_write_byte(scrolloff & 0xFF);
}
