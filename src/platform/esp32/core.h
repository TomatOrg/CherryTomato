#pragma once


#define XCHAL_NUM_AREGS			64 /* num of physical addr regs */
#define XCHAL_NUM_INTLEVELS     6 /* number of interrupt levels (not including level zero) */

// for 40MHz crystal
#define APB_CLOCK_HZ (80 * 1000 * 1000)
#define XTAL_CLOCK_HZ (40 * 1000 * 1000)
#define I2C_CLOCK_HZ (80 * 1000 * 1000)
#define PWM_CLOCK_HZ (160 * 1000 * 1000)
