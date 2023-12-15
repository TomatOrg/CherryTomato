#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "font.h"

typedef struct {
    uint16_t length;
    uint16_t numlines;
    uint16_t starts[64]; // TODO: use a FAM
} text_wrapped_t;


void text_drawicon(uint8_t* data, int x, int y);
int text_drawchar(font_info_t *chinfo, int chidx, int x, int basey);
void text_drawline(font_info_t *chinfo, const char *str, int x, int basey);
int text_getlinesize(font_info_t *chinfo, const char *str);
void text_wrap(font_info_t *chinfo, const char *str, int wrapat, text_wrapped_t *dest);
int text_draw_wrapped(font_info_t *chinfo, text_wrapped_t *msg, const char *str, int x, int basey);