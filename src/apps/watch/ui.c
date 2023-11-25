#include "ui.h"
#include "plat.h"

handler_t *g_handler;
int g_top = 0;
int g_startscroll_above = 0;

void ui_update_scrolloff(int newscrolloff, int *start, int *lines) {
    if (newscrolloff > g_scrolloff) {
        *start = g_scrolloff + 240;
        *lines = newscrolloff - g_scrolloff;
    } else if (newscrolloff < g_scrolloff) {
        *start = newscrolloff;
        *lines = g_scrolloff - newscrolloff;
    } else {
        *lines = 0;
    }
    g_scrolloff = newscrolloff;
}