#include "gpio.h"
#include "util/except.h"

#include <util/defs.h>

typedef union GPIO_PINn_REG {
    struct {
        uint32_t : 2;
        uint32_t pad_driver : 1;
        uint32_t : 4;
        uint32_t int_type : 3;
        uint32_t wakeup_enable : 1;
        uint32_t : 2;
        uint32_t int_ena : 5;
        uint32_t : 14;
    };
    uint32_t packed;
} MMIO GPIO_PINn_REG;
STATIC_ASSERT(sizeof(GPIO_PINn_REG) == sizeof(uint32_t));

typedef union GPIO_FUNCn_IN_SEL_CFG_REG {
    struct {
        uint32_t in_sel : 6;
        uint32_t in_inv_sel : 1;
        uint32_t sig_in_sel : 1;
        uint32_t : 24;
    };
    uint32_t packed;
} MMIO GPIO_FUNCn_IN_SEL_CFG_REG;
STATIC_ASSERT(sizeof(GPIO_FUNCn_IN_SEL_CFG_REG) == sizeof(uint32_t));

typedef union GPIO_FUNCn_OUT_SEL_CFG_REG {
    struct {
        uint32_t out_sel : 9;
        uint32_t out_inv_sel : 1;
        uint32_t oen_sel : 1;
        uint32_t oen_inv_sel : 1;
        uint32_t : 20;
    };
    uint32_t packed;
} MMIO GPIO_FUNCn_OUT_SEL_CFG_REG;
STATIC_ASSERT(sizeof(GPIO_FUNCn_OUT_SEL_CFG_REG) == sizeof(uint32_t));

typedef union IO_MUX_x {
    struct {
        uint32_t mcu_oe : 1;
        uint32_t slp_sel : 1;
        uint32_t mcu_wpd : 1;
        uint32_t mcu_wpu : 1;
        uint32_t mcu_ie : 1;
        uint32_t mcu_drv : 2;
        uint32_t fun_wpd : 1;
        uint32_t fun_wpu : 1;
        uint32_t fun_ie : 1;
        uint32_t fun_drv : 2;
        uint32_t mcu_sel : 3;
        uint32_t : 17;
    };
    uint32_t packed;
} MMIO IO_MUX_x;
STATIC_ASSERT(sizeof(IO_MUX_x) == sizeof(uint32_t));

extern volatile uint32_t GPIO_OUT;
extern volatile uint32_t GPIO_OUT_W1TS;
extern volatile uint32_t GPIO_OUT_W1TC;

extern volatile uint32_t GPIO_OUT1;
extern volatile uint32_t GPIO_OUT1_W1TS;
extern volatile uint32_t GPIO_OUT1_W1TC;

extern volatile uint32_t GPIO_ENABLE_W1TS;
extern volatile uint32_t GPIO_ENABLE1_W1TS;
extern volatile uint32_t GPIO_ENABLE_W1TC;
extern volatile uint32_t GPIO_ENABLE1_W1TC;

extern volatile GPIO_PINn_REG GPIO_PINn[40];
extern volatile GPIO_FUNCn_IN_SEL_CFG_REG GPIO_FUNCn_IN_SEL_CFG[256];
extern volatile GPIO_FUNCn_OUT_SEL_CFG_REG GPIO_FUNCn_OUT_SEL_CFG[40];

static volatile IO_MUX_x* get_io_mux_reg(int gpio) {
    uintptr_t addr = -1;
    switch (gpio) {
        case 0: addr = 0x3FF49044; break;
        case 2: addr = 0x3FF49040; break;
        case 4: addr = 0x3FF49048; break;
        case 5: addr = 0x3FF4906C; break;
        case 16: addr = 0x3FF4904C; break;
        case 17: addr = 0x3FF49050; break;
        case 18: addr = 0x3FF49070; break;
        case 19: addr = 0x3FF49074; break;
        case 20: addr = 0x3FF49078; break;
        case 21: addr = 0x3FF4907C; break;
        case 22: addr = 0x3FF49080; break;
        case 23: addr = 0x3FF4908C; break;
        case 25: addr = 0x3FF49024; break;
        case 26: addr = 0x3FF49028; break;
        case 27: addr = 0x3FF4902C; break;
        case 32: addr = 0x3FF4901C; break;
        case 33: addr = 0x3FF49020; break;
        case 34: addr = 0x3FF49014; break;
        case 35: addr = 0x3FF49018; break;
        case 36: addr = 0x3FF49004; break;
        case 37: addr = 0x3FF49008; break;
        case 38: addr = 0x3FF4900C; break;
        case 39: addr = 0x3FF49010; break;
        default: ASSERT(!"Invalid IOMUX reg"); break;
    }
    return (volatile IO_MUX_x*)addr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline void gpio_write_out_en_set(uint8_t gpio_num) {
    if (gpio_num < 32) {
        GPIO_ENABLE_W1TS = 1 << gpio_num;
    } else {
        GPIO_ENABLE1_W1TS = 1 << (gpio_num - 32);
    }
}

static inline void gpio_write_out_en_clear(uint8_t gpio_num) {
    if (gpio_num < 32) {
        GPIO_ENABLE_W1TC = 1 << gpio_num;
    } else {
        GPIO_ENABLE1_W1TC = 1 << (gpio_num - 32);
    }
}

static void gpio_init_output(uint8_t gpio_num, uint8_t alternate, bool open_drain) {
    gpio_write_out_en_set(gpio_num);
    GPIO_PINn[gpio_num].pad_driver = open_drain;

    GPIO_FUNCn_OUT_SEL_CFG[gpio_num].out_sel = 256;

    IO_MUX_x mux_value = *get_io_mux_reg(gpio_num);
    mux_value.mcu_sel = alternate;
    mux_value.fun_ie = open_drain;
    mux_value.fun_wpd = 0;
    mux_value.fun_wpu = 0;
    mux_value.fun_drv = 2; // 20mA
    mux_value.slp_sel = 0;
    *get_io_mux_reg(gpio_num) = mux_value;
}

void gpio_set_to_open_drain_output(uint8_t gpio_num) {
    gpio_init_output(gpio_num, 2, true);
}

void gpio_set_to_push_pull_output(uint8_t gpio_num) {
    gpio_init_output(gpio_num, 2, false);
}

void gpio_set_output_high(uint8_t gpio_num, bool high) {
    if (high) {
        gpio_set_high(gpio_num);
    } else {
        gpio_set_low(gpio_num);
    }
}

void gpio_set_to_input(uint8_t gpio_num) {
    gpio_write_out_en_clear(gpio_num);

    GPIO_FUNCn_OUT_SEL_CFG[gpio_num].out_sel = 256;

    volatile IO_MUX_x* mux = get_io_mux_reg(gpio_num);
    IO_MUX_x mux_value = *mux;
    mux_value.mcu_sel = 2; // gpio function
    mux_value.fun_ie = 1;
    mux_value.fun_wpd = 0;
    mux_value.fun_wpu = 0;
    mux_value.slp_sel = 0;
    *mux = mux_value;
}

void gpio_enable_output(uint8_t gpio_num, bool on) {
    if (on) {
        gpio_write_out_en_set(gpio_num);
    } else {
        gpio_write_out_en_clear(gpio_num);
    }
}

void gpio_enable_input(uint8_t gpio_num, bool on) {
    get_io_mux_reg(gpio_num)->fun_ie = on;
}

void gpio_internal_pull_up(uint8_t gpio_num, bool on) {
    get_io_mux_reg(gpio_num)->fun_wpu = on;
}

void gpio_set_high(uint8_t gpio_num) {
    if (gpio_num < 32) {
        GPIO_OUT_W1TS = 1 << gpio_num;
    } else {
        GPIO_OUT1_W1TS = 1 << (gpio_num - 32);
    }
}

void gpio_set_low(uint8_t gpio_num) {
    if (gpio_num < 32) {
        GPIO_OUT_W1TC = 1 << gpio_num;
    } else {
        GPIO_OUT1_W1TC = 1 << (gpio_num - 32);
    }
}

void gpio_internal_pull_down(uint8_t gpio_num, bool on) {
    get_io_mux_reg(gpio_num)->fun_wpd = on;
}

static inline void gpio_set_alternate_function(uint8_t gpio_num, uint8_t alternate) {
    get_io_mux_reg(gpio_num)->mcu_sel = alternate;
}

void gpio_connect_peripheral_to_output(uint8_t gpio_num, uint8_t signal) {
    GPIO_FUNCn_OUT_SEL_CFG_REG reg = GPIO_FUNCn_OUT_SEL_CFG[gpio_num];
    reg.out_sel = signal;
    reg.out_inv_sel = false;
    reg.oen_sel = false;
    reg.oen_inv_sel = false;
    GPIO_FUNCn_OUT_SEL_CFG[gpio_num] = reg;
}

void gpio_connect_input_to_peripheral(uint8_t gpio_num, uint8_t signal) {
    GPIO_FUNCn_IN_SEL_CFG_REG reg = GPIO_FUNCn_IN_SEL_CFG[signal];
    reg.sig_in_sel = true;
    reg.in_inv_sel = false;
    reg.in_sel = gpio_num;
    GPIO_FUNCn_IN_SEL_CFG[signal] = reg;
}
