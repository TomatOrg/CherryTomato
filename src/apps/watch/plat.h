#pragma once
#include <stdbool.h>
#include <stdint.h>

#define NLINES 16

extern uint16_t *g_target;
extern int g_scrolloff;
extern int g_line;
extern int g_nlines;
extern int g_pitch;

void plat_update(int x, int y, int w, int h);
