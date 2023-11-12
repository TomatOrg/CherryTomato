#pragma once

#include "gpio.h"
#include "util/except.h"

typedef struct spi {
    void* regs;
    gpio_signal_t sck;
    gpio_signal_t mosi;
    gpio_signal_t miso;
    gpio_signal_t cs;
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
void spi_write(spi_t* spi, uint8_t* bytes, int size);

/**
 * Perform a blocking transfer for data with an 4 byte aligned size
 */
void spi_write_aligned(spi_t* spi, uint8_t* bytes, int size);

/**
 * Perform a blocking transfer of a single byte
 */
void spi_write_byte(spi_t* spi, uint8_t byte);
