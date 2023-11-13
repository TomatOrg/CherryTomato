#include "axp202.h"

//! REG MAP
#define AXP202_STATUS (0x00)
#define AXP202_MODE_CHGSTATUS (0x01)
#define AXP202_OTG_STATUS (0x02)
#define AXP202_IC_TYPE (0x03)
#define AXP202_DATA_BUFFER1 (0x04)
#define AXP202_DATA_BUFFER2 (0x05)
#define AXP202_DATA_BUFFER3 (0x06)
#define AXP202_DATA_BUFFER4 (0x07)
#define AXP202_DATA_BUFFER5 (0x08)
#define AXP202_DATA_BUFFER6 (0x09)
#define AXP202_DATA_BUFFER7 (0x0A)
#define AXP202_DATA_BUFFER8 (0x0B)
#define AXP202_DATA_BUFFER9 (0x0C)
#define AXP202_DATA_BUFFERA (0x0D)
#define AXP202_DATA_BUFFERB (0x0E)
#define AXP202_DATA_BUFFERC (0x0F)
#define AXP202_LDO234_DC23_CTL (0x12)
#define AXP202_DC2OUT_VOL (0x23)
#define AXP202_LDO3_DC2_DVM (0x25)
#define AXP202_DC3OUT_VOL (0x27)
#define AXP202_LDO24OUT_VOL (0x28)
#define AXP202_LDO3OUT_VOL (0x29)
#define AXP202_IPS_SET (0x30)
#define AXP202_VOFF_SET (0x31)
#define AXP202_OFF_CTL (0x32)
#define AXP202_CHARGE1 (0x33)
#define AXP202_CHARGE2 (0x34)
#define AXP202_BACKUP_CHG (0x35)
#define AXP202_POK_SET (0x36)
#define AXP202_DCDC_FREQSET (0x37)
#define AXP202_VLTF_CHGSET (0x38)
#define AXP202_VHTF_CHGSET (0x39)
#define AXP202_APS_WARNING1 (0x3A)
#define AXP202_APS_WARNING2 (0x3B)
#define AXP202_TLTF_DISCHGSET (0x3C)
#define AXP202_THTF_DISCHGSET (0x3D)
#define AXP202_DCDC_MODESET (0x80)
#define AXP202_ADC_EN1 (0x82)
#define AXP202_ADC_EN2 (0x83)
#define AXP202_ADC_SPEED (0x84)
#define AXP202_ADC_INPUTRANGE (0x85)
#define AXP202_ADC_IRQ_RETFSET (0x86)
#define AXP202_ADC_IRQ_FETFSET (0x87)
#define AXP202_TIMER_CTL (0x8A)
#define AXP202_VBUS_DET_SRP (0x8B)
#define AXP202_HOTOVER_CTL (0x8F)
#define AXP202_GPIO0_CTL (0x90)
#define AXP202_GPIO0_VOL (0x91)
#define AXP202_GPIO1_CTL (0x92)
#define AXP202_GPIO2_CTL (0x93)
#define AXP202_GPIO012_SIGNAL (0x94)
#define AXP202_GPIO3_CTL (0x95)
#define AXP202_INTEN1 (0x40)
#define AXP202_INTEN2 (0x41)
#define AXP202_INTEN3 (0x42)
#define AXP202_INTEN4 (0x43)
#define AXP202_INTEN5 (0x44)
#define AXP202_INTSTS1 (0x48)
#define AXP202_INTSTS2 (0x49)
#define AXP202_INTSTS3 (0x4A)
#define AXP202_INTSTS4 (0x4B)
#define AXP202_INTSTS5 (0x4C)

static err_t axp202_read(uint8_t reg, uint8_t* data) {
    err_t err = NO_ERROR;

    CHECK_AND_RETHROW(target_axp202_write_bytes(0x35, &reg, 1));
    CHECK_AND_RETHROW(target_axp202_read_bytes(0x35, data, 1));

cleanup:
    return err;
}

static err_t axp202_write(uint8_t reg, uint8_t data) {
    err_t err = NO_ERROR;

    uint8_t buf[] = {
        reg, data
    };
    CHECK_AND_RETHROW(target_axp202_write_bytes(0x35, buf, 2));

cleanup:
    return err;
}

static err_t axp202_probe() {
    err_t err = NO_ERROR;

    // read the ic type
    uint8_t ic_type;
    CHECK_AND_RETHROW(axp202_read(AXP202_IC_TYPE, &ic_type));
    LOG_TRACE("\tChip ID: 0x%02x", ic_type);
    CHECK(ic_type == 0x41);

cleanup:
    return err;
}

err_t axp202_init() {
    err_t err = NO_ERROR;

    LOG_TRACE("\t--- AXP202 ---");
    CHECK_AND_RETHROW(axp202_probe());

cleanup:
    return err;
}

err_t axp202_set_ldo2_voltage(uint16_t mv) {
    err_t err = NO_ERROR;

    // check the range is valid
    CHECK(mv >= AXP202_LD02_MV_MIN);
    CHECK(mv <= AXP202_LD02_MV_MAX);
    CHECK((mv % AXP202_LD02_MV_STEP) == 0);

    // calculate the real value
    uint8_t val = ((mv - AXP202_LD02_MV_MIN) / AXP202_LD02_MV_STEP);

    // write it
    uint8_t rval;
    CHECK_AND_RETHROW(axp202_read(AXP202_LDO24OUT_VOL, &rval));
    rval &= 0xF0;
    rval |= (val << 4);
    CHECK_AND_RETHROW(axp202_write(AXP202_LDO24OUT_VOL, rval));



cleanup:
    return err;
}

err_t axp202_set_ldo3_volatge(uint16_t mv) {
    err_t err = NO_ERROR;

    uint8_t rval = (mv - 700) / 25;
    CHECK_AND_RETHROW(axp202_write(AXP202_LDO3OUT_VOL, rval));

cleanup:
    return err;
}

err_t axp202_set_ldo4_voltage(axp202_ldo4_t mv) {
    err_t err = NO_ERROR;

    uint8_t rval;
    CHECK_AND_RETHROW(axp202_read(AXP202_LDO24OUT_VOL, &rval));
    rval &= 0x0F;
    rval |= mv;
    CHECK_AND_RETHROW(axp202_write(AXP202_LDO24OUT_VOL, rval));

cleanup:
    return err;
}

err_t axp202_set_vbus_current_limit(axp202_vbus_current_limit_t limit) {
    err_t err = NO_ERROR;

    uint8_t reg;
    CHECK_AND_RETHROW(axp202_read(AXP202_IPS_SET, &reg));
    reg |= limit;
    CHECK_AND_RETHROW(axp202_write(AXP202_IPS_SET, reg));

cleanup:
    return err;
}

err_t axp202_set_power_output(axp202_channel_t channel, bool on) {
    err_t err = NO_ERROR;

    uint8_t reg;
    CHECK_AND_RETHROW(axp202_read(AXP202_LDO234_DC23_CTL, &reg));
    if (on) {
        reg |= (1 << channel);
    } else {
        reg &= ~(1 << channel);
    }
    CHECK_AND_RETHROW(axp202_write(AXP202_LDO234_DC23_CTL, reg));

cleanup:
    return err;
}
