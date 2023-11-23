#include "ft6x06.h"

// taken from https://github.com/STMicroelectronics/stm32-ft6x06

/* Current mode register of the FT6X06 (R/W) */
#define FT6X06_DEV_MODE_REG         0x00U

/* Gesture ID register */
#define FT6X06_GEST_ID_REG          0x01U

/* Touch Data Status register : gives number of active touch points (0..2) */
#define FT6X06_TD_STAT_REG          0x02U

/* P1 X, Y coordinates, weight and misc registers */
#define FT6X06_P1_XH_REG            0x03U
#define FT6X06_P1_XL_REG            0x04U
#define FT6X06_P1_YH_REG            0x05U
#define FT6X06_P1_YL_REG            0x06U
#define FT6X06_P1_WEIGHT_REG        0x07U
#define FT6X06_P1_MISC_REG          0x08U

/* P2 X, Y coordinates, weight and misc registers */
#define FT6X06_P2_XH_REG            0x09U
#define FT6X06_P2_XL_REG            0x0AU
#define FT6X06_P2_YH_REG            0x0BU
#define FT6X06_P2_YL_REG            0x0CU
#define FT6X06_P2_WEIGHT_REG        0x0DU
#define FT6X06_P2_MISC_REG          0x0EU

/* Threshold for touch detection */
#define FT6X06_TH_GROUP_REG         0x80U

/* Filter function coefficients */
#define FT6X06_TH_DIFF_REG          0x85U

/* Control register */
#define FT6X06_CTRL_REG             0x86U

/* The time period of switching from Active mode to Monitor mode when there is no touching */
#define FT6X06_TIMEENTERMONITOR_REG 0x87U

/* Report rate in Active mode */
#define FT6X06_PERIODACTIVE_REG     0x88U

/* Report rate in Monitor mode */
#define FT6X06_PERIODMONITOR_REG    0x89U

/* The value of the minimum allowed angle while Rotating gesture mode */
#define FT6X06_RADIAN_VALUE_REG     0x91U

/* Maximum offset while Moving Left and Moving Right gesture */
#define FT6X06_OFFSET_LR_REG        0x92U

/* Maximum offset while Moving Up and Moving Down gesture */
#define FT6X06_OFFSET_UD_REG        0x93U

/* Minimum distance while Moving Left and Moving Right gesture */
#define FT6X06_DISTANCE_LR_REG      0x94U

/* Minimum distance while Moving Up and Moving Down gesture */
#define FT6X06_DISTANCE_UD_REG      0x95U

/* Maximum distance while Zoom In and Zoom Out gesture */
#define FT6X06_DISTANCE_ZOOM_REG    0x96U

/* High 8-bit of LIB Version info */
#define FT6X06_LIB_VER_H_REG        0xA1U

/* Low 8-bit of LIB Version info */
#define FT6X06_LIB_VER_L_REG        0xA2U

/* Chip Selecting */
#define FT6X06_CIPHER_REG           0xA3U

/* Interrupt mode register (used when in interrupt mode) */
#define FT6X06_GMODE_REG            0xA4U

/* Current power mode the FT6X06 system is in (R) */
#define FT6X06_PWR_MODE_REG         0xA5U

/* FT6X06 firmware version */
#define FT6X06_FIRMID_REG           0xA6U

/* FT6X06 Chip identification register */
#define FT6X06_CHIP_ID_REG          0xA8U

/* Release code version */
#define FT6X06_RELEASE_CODE_ID_REG  0xAFU

/* Current operating mode the FT6X06 system is in (R) */
#define FT6X06_STATE_REG            0xBCU

static err_t ft6x06_read(uint8_t reg, uint8_t* data, size_t n) {
    err_t err = NO_ERROR;

    CHECK_AND_RETHROW(target_ft6x06_write_bytes(0x38, &reg, 1));
    CHECK_AND_RETHROW(target_ft6x06_read_bytes(0x38, data, n));

cleanup:
    return err;
}

static err_t ft6x06_probe() {
    err_t err = NO_ERROR;

    // TODO: do something

cleanup:
    return err;
}

err_t ft6x06_init() {
    err_t err = NO_ERROR;

    LOG_TRACE("\t--- F6x06 ---");
    CHECK_AND_RETHROW(ft6x06_probe());

cleanup:
    return err;
}

err_t ft6x06_touch(bool* touched, uint16_t *x, uint16_t *y) {
    err_t err = NO_ERROR;

    uint8_t status[5];
    ft6x06_read(FT6X06_TD_STAT_REG, status, 5);

    if (status[0] == 0 || status[0] > 2) {
        *touched = false;
        goto cleanup;
    }

    *touched = true;

    *x = (status[1] << 8) | status[2];
    *y = (status[3] << 8) | status[4];

cleanup:
    return err;
}

