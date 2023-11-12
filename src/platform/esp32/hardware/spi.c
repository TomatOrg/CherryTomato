#include "spi.h"
#include "core.h"

#include <util/defs.h>

spi_t g_spi2 = { (void*)0x3FF64000, GPIO_HSPICLK, GPIO_HSPID, GPIO_HSPIQ, GPIO_HSPICS0 };
spi_t g_spi3 = { (void*)0x3FF65000, GPIO_VSPICLK, GPIO_VSPID, GPIO_VSPIQ, GPIO_VSPICS0 };

typedef union {
    struct {
        uint32_t : 13;
        uint32_t fastrd_mode : 1;
        uint32_t fread_dual : 1;
        uint32_t : 5;
        uint32_t fread_quad : 1;
        uint32_t wp : 1;
        uint32_t : 1;
        uint32_t fread_dio : 1;
        uint32_t fread_qio : 1;
        uint32_t rd_bit_order : 1;
        uint32_t wr_bit_order : 1;
        uint32_t : 5;
    };
    uint32_t packed;
} MMIO SPI_CTRL_REG;
STATIC_ASSERT(sizeof(SPI_CTRL_REG) == sizeof(uint32_t));

typedef union {
    struct {
        uint32_t l : 6;
        uint32_t h : 6;
        uint32_t n : 6;
        uint32_t pre : 13;
        uint32_t equ_sysclk : 1;
    };
    uint32_t packed;
} MMIO SPI_CLOCK_REG;
STATIC_ASSERT(sizeof(SPI_CLOCK_REG) == sizeof(uint32_t));

typedef union {
    struct {
        uint32_t cs0_dis : 1;
        uint32_t cs1_dis : 1;
        uint32_t cs2_dis : 1;
        uint32_t : 2;
        uint32_t ck_dis : 1;
        uint32_t master_cs_pol : 3;
        uint32_t : 2;
        uint32_t master_ck_sel : 3;
        uint32_t : 15;
        uint32_t ck_idle_edge : 1;
        uint32_t cs_keep_active : 1;
        uint32_t : 1;
    };
    uint32_t packed;
} MMIO SPI_PIN_REG;
STATIC_ASSERT(sizeof(SPI_PIN_REG) == sizeof(uint32_t));

typedef union {
    struct {
        uint32_t doutdin : 1;
        uint32_t : 3;
        uint32_t cs_hold : 1;
        uint32_t cs_setup : 1;
        uint32_t ck_i_edge : 1;
        uint32_t ck_out_edge : 1;
        uint32_t : 2;
        uint32_t rd_byte_order : 1;
        uint32_t wr_byte_order : 1;
        uint32_t fwrite_dual : 1;
        uint32_t fwrite_quad : 1;
        uint32_t fwrite_dio : 1;
        uint32_t fwrite_qio : 1;
        uint32_t sio : 1;
        uint32_t : 7;
        uint32_t usr_miso_highpart : 1;
        uint32_t usr_mosi_highpart : 1;
        uint32_t usr_dummy_idle : 1;
        uint32_t usr_mosi : 1;
        uint32_t usr_miso : 1;
        uint32_t usr_dummy : 1;
        uint32_t usr_addr : 1;
        uint32_t usr_command : 1;
    };
    uint32_t packed;
} MMIO SPI_USER_REG;
STATIC_ASSERT(sizeof(SPI_USER_REG) == sizeof(uint32_t));

typedef union {
    struct {
        uint32_t : 18;
        uint32_t usr : 1;
        uint32_t : 13;
    };
    uint32_t packed;
} MMIO SPI_CMD_REG;
STATIC_ASSERT(sizeof(SPI_CMD_REG) == sizeof(uint32_t));

#define SPI_CMD(x)          (*(volatile SPI_CMD_REG*)(x->regs + 0x0))
#define SPI_CTRL(x)         (*(volatile SPI_CTRL_REG*)(x->regs + 0x8))
#define SPI_CLOCK(x)        (*(volatile SPI_CLOCK_REG*)(x->regs + 0x18))
#define SPI_USER(x)         (*(volatile SPI_USER_REG*)(x->regs + 0x1c))
#define SPI_MOSI_DLEN(x)    (*(volatile uint32_t*)(x->regs + 0x28))
#define SPI_MISO_DLEN(x)    (*(volatile uint32_t*)(x->regs + 0x2c))
#define SPI_W(x, n)         (*(volatile uint32_t*)(x->regs + 0x80 + (4 * (n))))
#define SPI_PIN(x)          (*(volatile SPI_PIN_REG*)(x->regs + 0x34))
#define SPI_SALVE(x)        (*(volatile uint32_t*)(x->regs + 0x38))

static void spi_setup(spi_t* spi, uint32_t frequency) {
    SPI_CLOCK_REG clock = { .packed = 0 };

    // In HW, n, h and l fields range from 1 to 64, pre ranges from 1 to 8K.
    // The value written to register is one lower than the used value.
    if (frequency > ((APB_CLOCK_HZ / 4) * 3)) {
        // Using APB frequency directly will give us the best result here.
        clock.equ_sysclk = 1;
    } else {
        // For best duty cycle resolution, we want n to be as close to 32 as
        // possible, but we also need a pre/n combo that gets us as close as
        // possible to the intended frequency. To do this, we bruteforce n and
        // calculate the best pre to go along with that. If there's a choice
        // between pre/n combos that give the same result, use the one with the
        // higher n.
        int32_t bestn = -1;
        int32_t bestpre = -1;
        int32_t besterr = 0;

        // Start at n = 2. We need to be able to set h/l so we have at least
        // one high and one low pulse.
        for (int n = 2; n < 64; n++) {
            // Effectively, this does:
            //   pre = round((APB_CLK_FREQ / n) / frequency)

            int32_t pre = ((APB_CLOCK_HZ / n) + (frequency / 2)) / frequency;

            if (pre <= 0) {
                pre = 1;
            }

            if (pre > 16) {
                pre = 16;
            }

            int32_t errval = ABS((APB_CLOCK_HZ / (pre * n)) - frequency);
            if (bestn == -1 || errval <= besterr) {
                besterr = errval;
                bestn = n;
                bestpre = pre;
            }
        }

        int32_t n = bestn;
        int32_t pre = bestpre;
        int32_t l = n;

        // Effectively, this does:
        //   h = round((duty_cycle * n) / 256)
        int32_t h = (128 * n + 127) / 256;
        if (h <= 0) {
            h = 1;
        }

        clock.l = l;
        clock.h = h;
        clock.n = n;
        clock.pre = pre;
    }

    // set the clock
    SPI_CLOCK(spi) = clock;
}

void spi_init(
    spi_t* spi,
    uint8_t sck, uint8_t mosi, uint8_t miso, uint8_t cs,
    uint32_t frequency, spi_data_mode_t mode
) {
    // setup the clocks
    spi_setup(spi, frequency);

    // initialize the basic settings
    SPI_USER_REG user = SPI_USER(spi);
    user.usr_miso_highpart = 0;
    user.doutdin = 1;
    user.usr_miso = 1;
    user.usr_mosi = 1;
    user.cs_hold = 1;
    user.usr_dummy_idle = 1;
    user.usr_addr = 0;
    user.usr_command = 0;
    SPI_USER(spi) = user;

    // clear write protection
    SPI_CTRL(spi).wp = 0;

    // clear slave configurations since we are a master
    SPI_SALVE(spi) = 0;

    // setup the mode correctly
    SPI_PIN_REG pin = SPI_PIN(spi);
    pin.ck_idle_edge = (mode == SPI_DATA_MODE2 || mode == SPI_DATA_MODE3) ? 1 : 0;
    SPI_PIN(spi) = pin;

    user = SPI_USER(spi);
    user.ck_out_edge = (mode == SPI_DATA_MODE1 || mode == SPI_DATA_MODE2) ? 1 : 0;
    SPI_USER(spi) = user;

    // setup the pins
    if (sck != INVALID_GPIO) {
        gpio_set_to_push_pull_output(sck);
        gpio_connect_peripheral_to_output(sck, spi->sck);
    }

    if (mosi != INVALID_GPIO) {
        gpio_set_to_push_pull_output(mosi);
        gpio_connect_peripheral_to_output(mosi, spi->mosi);
    }

    if (miso != INVALID_GPIO) {
        gpio_set_to_input(miso);
        gpio_connect_input_to_peripheral(miso, spi->miso);
    }

    if (cs != INVALID_GPIO) {
        gpio_set_to_push_pull_output(cs);
        gpio_connect_peripheral_to_output(cs, spi->cs);
    }
}

void spi_write(spi_t* spi, uint8_t* bytes, int size) {
    // spit into chunks
    for (; size > 0; size -= 64, bytes += 64) {
        int len = MIN(64, size) - 1;
        SPI_MOSI_DLEN(spi) = len;

        // write the data into the memory
        xthal_memcpy((void*)&SPI_W(spi, 0), bytes, ALIGN_DOWN(len, 4));

        // copy the unaligned data
        uint32_t word = 0;
        int unaligned = (len % 4) != 0;
        switch (unaligned) {
            case 0: continue;
            case 1: __builtin_memcpy(&word, bytes, 1); break;
            case 2: __builtin_memcpy(&word, bytes, 2); break;
            case 3: __builtin_memcpy(&word, bytes, 3); break;
        }
        SPI_W(spi, len / 4) = word;

        // flush
        SPI_CMD(spi).usr = 1;
        while (SPI_CMD(spi).usr);
    }
}

void spi_write_aligned(spi_t* spi, uint8_t* bytes, int size) {
    // spit into chunks
    for (; size > 0; size -= 64, bytes += 64) {
        int len = MIN(64, size) - 1;
        SPI_MOSI_DLEN(spi) = len;

        // write the data into the memory
        xthal_memcpy((void*)&SPI_W(spi, 0), bytes, len);

        // flush
        SPI_CMD(spi).usr = 1;
        while (SPI_CMD(spi).usr);
    }
}

void spi_write_byte(spi_t* spi, uint8_t byte) {
    // write it
    SPI_MOSI_DLEN(spi) = 1;
    SPI_W(spi, 0) = byte;

    // flush
    SPI_CMD(spi).usr = 1;
    while (SPI_CMD(spi).usr);
}
