#include "i2c.h"
#include "util/except.h"
#include "core.h"

#include <util/defs.h>

typedef union {
    struct {
        uint32_t sda_force_out : 1;
        uint32_t scl_force_out : 1;
        uint32_t sample_scl_level : 1;
        uint32_t : 1;
        uint32_t ms_mode : 1;
        uint32_t trans_start : 1;
        uint32_t tx_lsb_first : 1;
        uint32_t rx_lsb_first : 1;
        uint32_t clk_en : 1;
        uint32_t : 23;
    };
    uint32_t packed;
} MMIO I2C_CTR_REG;
STATIC_ASSERT(sizeof(I2C_CTR_REG) == sizeof(uint32_t));

typedef union {
    struct {
        uint32_t thres : 3;
        uint32_t en : 1;
        uint32_t : 28;
    };
    uint32_t packed;
} MMIO I2C_FILTER_CFG_REG;
STATIC_ASSERT(sizeof(I2C_FILTER_CFG_REG) == sizeof(uint32_t));

typedef union {
    struct {
        uint32_t rxfifo_full_thrhd : 5;
        uint32_t txfifo_empty_thrhd : 5;
        uint32_t nonfifo_en : 1;
        uint32_t fifo_addr_cfg_en : 1;
        uint32_t rx_fifo_rst : 1;
        uint32_t tx_fifo_rst : 1;
        uint32_t nonfifo_rx_thres : 6;
        uint32_t nonfifo_tx_thres : 6;
        uint32_t : 6;
    };
    uint32_t packed;
} MMIO I2C_FIFO_CONF_REG;
STATIC_ASSERT(sizeof(I2C_FIFO_CONF_REG) == sizeof(uint32_t));

typedef union {
    struct {
        uint32_t byte_num : 8;
        uint32_t ack_check_en : 1;
        uint32_t ack_exp : 1;
        uint32_t ack_value : 1;
        uint32_t op_code : 3;
        uint32_t : 17;
        uint32_t done : 1;
    };
    uint32_t packed;
} MMIO I2C_COMD_REG;
STATIC_ASSERT(sizeof(I2C_COMD_REG) == sizeof(uint32_t));

typedef union {
    struct {
        uint32_t rxfifo_full : 1;
        uint32_t txfifo_empty : 1;
        uint32_t rxfifo_ovf : 1;
        uint32_t end_detect : 1;
        uint32_t slave_tran_comp : 1;
        uint32_t arbitration_lost : 1;
        uint32_t master_tran_comp : 1;
        uint32_t trans_complete : 1;
        uint32_t time_out : 1;
        uint32_t trans_start : 1;
        uint32_t ack_err : 1;
        uint32_t rx_rec_full : 1;
        uint32_t tx_send_empty : 1;
        uint32_t : 19;
    };
    uint32_t packed;
} MMIO I2C_INT_REG;
STATIC_ASSERT(sizeof(I2C_INT_REG) == sizeof(uint32_t));

i2c_t g_i2c0 = { .regs = (void*)0x3FF53000, .scl = GPIO_I2CEXT0_SCL, .sda = GPIO_I2CEXT0_SDA };
i2c_t g_i2c1 = { .regs = (void*)0x3FF67000, .scl = GPIO_I2CEXT1_SCL, .sda = GPIO_I2CEXT1_SDA };

#define I2C_SCL_LOW_PERIOD(x)   (*(volatile uint32_t*)(i2c->regs + 0))
#define I2C_CTR(x)              (*(volatile I2C_CTR_REG*)(i2c->regs + 0x4))
#define I2C_TO(x)               (*(volatile uint32_t*)(i2c->regs + 0xc))
#define I2C_FIFO_CONF(x)        (*(volatile I2C_FIFO_CONF_REG*)(i2c->regs + 0x18))
#define I2C_DATA(x)             (*(volatile uint32_t*)(i2c->regs + 0x1c))

#define I2C_INT_RAW(x)          (*(volatile I2C_INT_REG*)(i2c->regs + 0x20))
#define I2C_INT_CLR(x)          (*(volatile I2C_INT_REG*)(i2c->regs + 0x24))
#define I2C_INT_ENA(x)          (*(volatile I2C_INT_REG*)(i2c->regs + 0x28))
#define I2C_INT_STATUS(x)       (*(volatile I2C_INT_REG*)(i2c->regs + 0x2c))

#define I2C_SDA_HOLD(x)         (*(volatile uint32_t*)(i2c->regs + 0x30))
#define I2C_SDA_SAMPLE(x)       (*(volatile uint32_t*)(i2c->regs + 0x34))
#define I2C_SCL_HIGH_PERIOD(x)  (*(volatile uint32_t*)(i2c->regs + 0x38))
#define I2C_SCL_START_HOLD(x)   (*(volatile uint32_t*)(i2c->regs + 0x40))
#define I2C_SCL_RSTART_SETUP(x) (*(volatile uint32_t*)(i2c->regs + 0x44))
#define I2C_SCL_STOP_HOLD(x)    (*(volatile uint32_t*)(i2c->regs + 0x48))
#define I2C_SCL_STOP_SETUP(x)   (*(volatile uint32_t*)(i2c->regs + 0x4c))

#define I2C_SCL_FILTER_CFG(x)   (*(volatile I2C_FILTER_CFG_REG*)(i2c->regs + 0x50))
#define I2C_SDA_FILTER_CFG(x)   (*(volatile I2C_FILTER_CFG_REG*)(i2c->regs + 0x54))

#define I2C_COMDn(n)            (*(volatile I2C_COMD_REG*)(i2c->regs + 0x58 + ((n) * 4)))

static void i2c_reset_fifo(i2c_t* i2c) {
    // reset the fifo buffers
    I2C_FIFO_CONF_REG reg = I2C_FIFO_CONF(i2c);
    reg.tx_fifo_rst = 1;
    reg.rx_fifo_rst = 1;
    reg.nonfifo_en = 0;
    reg.nonfifo_rx_thres = 1;
    reg.nonfifo_tx_thres = 32;
    I2C_FIFO_CONF(i2c) = reg;

    // now enable it
    reg = I2C_FIFO_CONF(i2c);
    reg.tx_fifo_rst = 0;
    reg.rx_fifo_rst = 0;
    I2C_FIFO_CONF(i2c) = reg;
}

static void i2c_reset_command_list(i2c_t* i2c) {
    for (int i = 0; i < 16; i++) {
        I2C_COMDn(i).packed = 0;
    }
    i2c->cmd_index = 0;
}

static void i2c_reset(i2c_t* i2c) {
    // disable and clear all interrupts
    I2C_INT_ENA(i2c).packed = 0;
    I2C_INT_CLR(i2c).packed = 0x3ffff;

    // reset the fifo
    i2c_reset_fifo(i2c);

    // reset the command list
    i2c_reset_command_list(i2c);
}

static void i2c_set_frequency(i2c_t* i2c, uint32_t source_clk, uint32_t bus_freq) {
    // calculate all the times we need
    uint32_t half_cycle = source_clk / bus_freq / 2;
    uint32_t scl_low = half_cycle;
    uint32_t scl_high = half_cycle;
    uint32_t sda_hold = half_cycle / 2;
    uint32_t sda_sample = scl_high / 2;
    uint32_t setup = half_cycle;
    uint32_t hold = half_cycle;
    uint32_t tout = half_cycle * 20;

    // format for writing to the register
    scl_low -= 1;
    scl_high -= 7 + 6;

    //
    // write it out to the controller
    //

    I2C_SCL_LOW_PERIOD(i2c) = scl_low;
    I2C_SCL_HIGH_PERIOD(i2c) = scl_high;

    I2C_SDA_HOLD(i2c) = sda_hold;
    I2C_SDA_SAMPLE(i2c) = sda_sample;

    I2C_SCL_RSTART_SETUP(i2c) = setup;
    I2C_SCL_STOP_SETUP(i2c) = setup;

    I2C_SCL_START_HOLD(i2c) = hold;
    I2C_SCL_STOP_HOLD(i2c) = hold;

    I2C_TO(i2c) = tout;
}

void i2c_init(
    i2c_t* i2c,
    uint8_t sda, uint8_t scl,
    uint32_t frequency
) {
    //
    // setup the pins properly
    //
    gpio_set_output_high(scl, true);
    gpio_set_output_high(sda, true);

    gpio_set_to_open_drain_output(scl);
    gpio_enable_input(scl, true);
    gpio_internal_pull_up(scl, true);
    gpio_connect_peripheral_to_output(scl, i2c->scl);
    gpio_connect_input_to_peripheral(scl, i2c->scl);

    gpio_set_to_open_drain_output(sda);
    gpio_enable_input(sda, true);
    gpio_internal_pull_up(sda, true);
    gpio_connect_peripheral_to_output(sda, i2c->sda);
    gpio_connect_input_to_peripheral(sda, i2c->sda);

    //
    // Initialize the controller
    //

    // Set the I2C to master mode, with open drain output for SCL and SDA
    I2C_CTR_REG reg = (I2C_CTR_REG){
        .ms_mode = 1,
        .sda_force_out = 1,
        .scl_force_out = 1,
        .tx_lsb_first = 0,
        .rx_lsb_first = 0,
        .clk_en = 1,
    };
    I2C_CTR(i2c) = reg;

    // Setup the filter
    I2C_FILTER_CFG_REG cfg = (I2C_FILTER_CFG_REG){ .en = 1, .thres = 7 };
    I2C_SCL_FILTER_CFG(i2c) = cfg;
    I2C_SDA_FILTER_CFG(i2c) = cfg;

    //
    // Initialize the clocks
    //
    i2c_set_frequency(i2c, I2C_CLOCK_HZ, frequency);

    // TODO: can we get away with default i2c frequency

    // reset the controller
    i2c_reset(i2c);
}

typedef enum i2c_ack {
    I2C_ACK = 0,
    I2C_NACK = 1,
} i2c_ack_t;

typedef enum i2c_opcode {
    I2C_RSTART = 0,
    I2C_WRITE = 1,
    I2C_READ = 2,
    I2C_STOP = 3,
} i2c_opcode_t;

static volatile I2C_COMD_REG* i2c_get_next_cmd(i2c_t* i2c) {
    ASSERT(i2c->cmd_index < 16);
    volatile I2C_COMD_REG* cmd = &I2C_COMDn(i2c->cmd_index);
    i2c->cmd_index++;
    return cmd;
}

static void i2c_add_start_cmd(i2c_t* i2c) {
    I2C_COMD_REG reg = (I2C_COMD_REG){
        .op_code = I2C_RSTART,
        .ack_exp = I2C_NACK,
        .ack_value = I2C_NACK
    };
    *i2c_get_next_cmd(i2c) = reg;
}

static void i2c_add_stop_cmd(i2c_t* i2c) {
    I2C_COMD_REG reg = (I2C_COMD_REG){
        .op_code = I2C_STOP,
        .ack_exp = I2C_NACK,
        .ack_value = I2C_NACK
    };
    *i2c_get_next_cmd(i2c) = reg;
}

static void i2c_add_write_cmd(i2c_t* i2c, i2c_ack_t ack_exp, bool ack_check_en, uint8_t length) {
    I2C_COMD_REG reg = (I2C_COMD_REG){
        .op_code = I2C_WRITE,
        .ack_exp = ack_exp,
        .ack_check_en = ack_check_en,
        .ack_value = I2C_NACK,
        .byte_num = length
    };
    *i2c_get_next_cmd(i2c) = reg;
}

static void i2c_add_read_cmd(i2c_t* i2c, i2c_ack_t ack_value, uint8_t length) {
    I2C_COMD_REG reg = (I2C_COMD_REG){
        .op_code = I2C_READ,
        .ack_exp = I2C_NACK,
        .ack_value = ack_value,
        .byte_num = length
    };
    *i2c_get_next_cmd(i2c) = reg;
}

static void i2c_write_fifo(i2c_t* i2c, uint8_t data) {
    I2C_DATA(i2c) = data;
}

static uint8_t i2c_read_fifo(i2c_t* i2c) {
    return (uint8_t)I2C_DATA(i2c);
}

static void i2c_clear_all_interrupts(i2c_t* i2c) {
    I2C_INT_CLR(i2c).packed = 0x3ffff;
}

static void i2c_setup_read(i2c_t* i2c, uint8_t addr, size_t length) {
    i2c_clear_all_interrupts(i2c);

    i2c_add_start_cmd(i2c);
    i2c_add_write_cmd(i2c, I2C_ACK, true, 1);
    if (length > 1) {
        i2c_add_read_cmd(i2c, I2C_ACK, length - 1);
    }
    i2c_add_read_cmd(i2c, I2C_NACK, 1);
    i2c_add_stop_cmd(i2c);

    // Load address and read bit into FIFO
    i2c_write_fifo(i2c, addr << 1 | 1);
}

static void i2c_setup_write(i2c_t* i2c, uint8_t addr, size_t length) {
    i2c_clear_all_interrupts(i2c);

    i2c_add_start_cmd(i2c);
    i2c_add_write_cmd(i2c, I2C_ACK, true, 1 + length);
    i2c_add_stop_cmd(i2c);

    // Load address without read bit into FIFO
    i2c_write_fifo(i2c, addr << 1);
}

static void i2c_start_transmission(i2c_t* i2c) {
    I2C_CTR(i2c).trans_start = 1;
}

static err_t i2c_wait_for_completion(i2c_t* i2c) {
    err_t err = NO_ERROR;

    // wait for the interrupt
    for (;;) {
        I2C_INT_REG reg = I2C_INT_RAW(i2c);
        CHECK(reg.time_out == 0);
        CHECK(reg.ack_err == 0);
        CHECK(reg.arbitration_lost == 0);

        if (reg.trans_complete || reg.end_detect) {
            break;
        }
    }

    // check everything is done properly
    for (int i = 0; i < 16; i++) {
        CHECK(I2C_COMDn(i).packed == 0 || I2C_COMDn(i).done != 0);
    }

cleanup:
    if (IS_ERROR(err)) {
        i2c_reset(i2c);
    }

    return err;
}

static err_t i2c_read_all_from_fifo(i2c_t* i2c,  uint8_t* bytes, size_t length) {
    err_t err = NO_ERROR;

    // wait until we are done
    CHECK_AND_RETHROW(i2c_wait_for_completion(i2c));

    // read bytes from the fifo
    for (size_t i = 0; i < length; i++) {
        bytes[i] = i2c_read_fifo(i2c);
    }

cleanup:
    return err;
}

static err_t i2c_perform_read(i2c_t* i2c, uint8_t addr, uint8_t* bytes, size_t length) {
    err_t err = NO_ERROR;

    i2c_setup_read(i2c, addr, length);
    i2c_start_transmission(i2c);
    CHECK_AND_RETHROW(i2c_read_all_from_fifo(i2c, bytes, length));

cleanup:
    return err;
}

static void i2c_fill_tx_fifo(i2c_t* i2c, const uint8_t* bytes, size_t length) {
    for (size_t i = 0; i < length; i++) {
        i2c_write_fifo(i2c, bytes[i]);
    }
}

static err_t i2c_perform_write(i2c_t* i2c, uint8_t addr, const uint8_t* bytes, size_t length) {
    err_t err = NO_ERROR;

    i2c_setup_write(i2c, addr, length);
    i2c_fill_tx_fifo(i2c, bytes, length);
    i2c_start_transmission(i2c);
    CHECK_AND_RETHROW(i2c_wait_for_completion(i2c));

cleanup:
    return err;
}

err_t i2c_master_read(i2c_t* i2c, uint8_t addr, uint8_t* buffer, size_t buffer_length) {
    err_t err = NO_ERROR;

    i2c_reset_fifo(i2c);
    i2c_reset_command_list(i2c);
    CHECK_AND_RETHROW(i2c_perform_read(i2c, addr, buffer, buffer_length));

cleanup:
    return err;
}

err_t i2c_master_write(i2c_t* i2c, uint8_t addr, const uint8_t* buffer, size_t length) {
    err_t err = NO_ERROR;

    i2c_reset_fifo(i2c);
    i2c_reset_command_list(i2c);
    CHECK_AND_RETHROW(i2c_perform_write(i2c, addr, buffer, length));

cleanup:
    return err;
}
