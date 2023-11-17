#pragma once

#include "gpio.h"
#include "util/except.h"
#include "event/token.h"

typedef struct spi {
    void* regs;
    uint8_t sclk;
    uint8_t mosi;
    uint8_t miso;
    uint8_t cs;
} spi_t;

// the available spi devices
extern spi_t g_spi2;
extern spi_t g_spi3;

#define INVALID_GPIO 0xFF

typedef enum spi_data_mode {
    SPI_DATA_MODE0,
    SPI_DATA_MODE1,
    SPI_DATA_MODE2,
    SPI_DATA_MODE3,
} spi_data_mode_t;

void spi_init(
    spi_t* spi,
    uint8_t sck, uint8_t mosi, uint8_t miso, uint8_t cs,
    uint32_t frequency, spi_data_mode_t mode
);

/**
 * Perform a blocking transfer using aligned data source
 */
void spi_write(spi_t* spi, const uint8_t* bytes, size_t len, event_t* event);

/**
 * Perform a blocking transfer of a single byte
 */
void spi_write_byte(spi_t* spi, uint8_t byte);
