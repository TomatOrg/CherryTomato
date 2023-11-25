#pragma once

#include <stdint.h>

typedef struct font_charinfo {
    uint32_t startidx;
    uint8_t width;
    uint8_t height;
    uint8_t left;
    uint8_t top;
    uint8_t advance;
    uint16_t codepoint;
} __attribute__((packed)) font_charinfo_t;

typedef struct font_info {
    int count;
    uint8_t storage[];
} font_info_t;

extern uint8_t _binary__bebas1_start[];
extern uint8_t _binary__bebas2_start[];
extern uint8_t _binary__bebas3_start[];
extern uint8_t _binary__bebas4_start[];
extern uint8_t _binary__roboto_start[];

#define font_bebas1 ((font_info_t*)_binary__bebas1_start)
#define font_bebas2 ((font_info_t*)_binary__bebas2_start)
#define font_bebas3 ((font_info_t*)_binary__bebas3_start)
#define font_bebas4 ((font_info_t*)_binary__bebas4_start)
#define font_roboto ((font_info_t*)_binary__roboto_start)
