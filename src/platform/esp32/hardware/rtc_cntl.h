#pragma once

#include <util/defs.h>

typedef struct {
    uint32_t : 3;
    uint32_t lslp_mem_force_pd : 1;
    uint32_t lslp_mem_force_pu : 1;
    uint32_t rom0_force_pd : 1;
    uint32_t rom0_force_pu : 1;
    uint32_t inter_ram0_force_pd : 1;
    uint32_t inter_ram0_force_pu : 1;
    uint32_t inter_ram1_force_pd : 1;
    uint32_t inter_ram1_force_pu : 1;
    uint32_t inter_ram2_force_pd : 1;
    uint32_t inter_ram2_force_pu : 1;
    uint32_t inter_ram3_force_pd : 1;
    uint32_t inter_ram3_force_pu : 1;
    uint32_t inter_ram4_force_pd : 1;
    uint32_t inter_ram4_force_pu : 1;
    uint32_t wifi_force_pd : 1;
    uint32_t wifi_force_pu : 1;
    uint32_t dg_wrap_force_pd : 1;
    uint32_t dg_wrap_force_pu : 1;
    uint32_t : 3;
    uint32_t rom0_pd_en : 1;
    uint32_t inter_ram0_pd_en : 1;
    uint32_t inter_ram1_pd_en : 1;
    uint32_t inter_ram2_pd_en : 1;
    uint32_t inter_ram3_pd_en : 1;
    uint32_t inter_ram4_pd_en : 1;
    uint32_t wifi_pd_en : 1;
    uint32_t dg_wrap_pd_en : 1;
} MMIO RTC_CNTL_DIG_PWC_REG;
STATIC_ASSERT(sizeof(RTC_CNTL_DIG_PWC_REG) == sizeof(uint32_t));

typedef struct {
    uint32_t : 9;
    uint32_t dg_pad_autohold : 1;
    uint32_t clr_dg_pad_autohold : 1;
    uint32_t dg_pad_autohold_en: 1;
    uint32_t dg_pad_force_noiso : 1;
    uint32_t dg_pad_force_iso : 1;
    uint32_t dg_pad_force_unhold : 1;
    uint32_t dg_pad_force_hold : 1;
    uint32_t rom_force_iso : 1;
    uint32_t rom_force_noiso : 1;
    uint32_t inter_ram0_force_iso : 1;
    uint32_t inter_ram0_force_noiso : 1;
    uint32_t inter_ram1_force_iso : 1;
    uint32_t inter_ram1_force_noiso : 1;
    uint32_t inter_ram2_force_iso : 1;
    uint32_t inter_ram2_force_noiso : 1;
    uint32_t inter_ram3_force_iso : 1;
    uint32_t inter_ram3_force_noiso : 1;
    uint32_t inter_ram4_force_iso : 1;
    uint32_t inter_ram4_force_noiso : 1;
    uint32_t wifi_force_iso : 1;
    uint32_t wifi_force_noiso : 1;
    uint32_t dg_wrap_force_iso : 1;
    uint32_t dg_wrap_force_noiso : 1;
} MMIO RTC_CNTL_DIG_ISO_REG;
STATIC_ASSERT(sizeof(RTC_CNTL_DIG_ISO_REG) == sizeof(uint32_t));

extern volatile RTC_CNTL_DIG_PWC_REG RTC_CNTL_DIG_PWC;
extern volatile RTC_CNTL_DIG_ISO_REG RTC_CNTL_DIG_ISO;
