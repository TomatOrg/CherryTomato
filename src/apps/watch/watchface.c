#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <util/printf.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "event.h"
#include "font.h"
#include "messagestore.h"
#include "physics.h"
#include "plat.h"
#include "text.h"
#include "thumbnail.h"
#include "roundedrect.h"
#include "ui.h"
#include <util/divmod.h>

int g_time_month = 0;
int g_time_monthday = 0;
int g_time_hour = 0;
int g_time_minute = 0;
int g_time_weekday = 0;
inertial_state_t m_watchface_inertial = {.has_top_constraint = 1, .has_bottom_constraint = 1};

#define RED8(x) ((x) & 0xFF)
#define GREEN8(x) ((x >> 8) & 0xFF)
#define BLUE8(x) ((x >> 16) & 0xFF)

static void draw_diagonal(int xstart, int ystart, int xend, int yend, int thickness) {
    int start = MAX(g_line, ystart);
    int end = MIN(g_line + g_nlines, yend);
    int slope = (xend - xstart) * 256 / (yend - ystart);
    for (int y = start; y < end; y++) {
        int x = xstart * 256 + slope * (y - ystart);
        int mod = x % 256;
        x /= 256;

        // TODO: slow and approximate broken gamma
        // should be replaced with a lookup table
        // and use the same gamma curve as specified in the ST7789 settings
        // this is very important at low bpp
        int right = sqrtf(mod / 256.0) * 63.0;
        int left = sqrtf(1.0 - mod / 256.0) * 63.0;

        int rgb = 0xf06b5e;

        g_target[(y - g_line) * g_pitch + x] = __builtin_bswap16(
            ((RED8(rgb) * (left >> 1) / 256) << 0) |
            ((GREEN8(rgb) * left / 256) << 5) |
            ((BLUE8(rgb) * (left >> 1) / 256) << 11));
        g_target[(y - g_line) * g_pitch + x + thickness] = __builtin_bswap16(
            ((RED8(rgb) * (right >> 1) / 256) << 0) |
            ((GREEN8(rgb) * right / 256) << 5) |
            ((BLUE8(rgb) * (right >> 1) / 256) << 11));
        for (int j = 1; j < thickness; j++) g_target[(y - g_line) * g_pitch + x + j] = __builtin_bswap16(
            ((RED8(rgb) * 31 / 256) << 0) |
            ((GREEN8(rgb) * 63 / 256) << 5) |
            ((BLUE8(rgb) * 31 / 256) << 11)
        );
    }
}

void watchface_draw(int top) {
    char text[16];
    const char* weekday = &"SUN\0MON\0TUE\0THU\0WED\0SAT\0FRI\0"[g_time_weekday * 4];
    const char* month = &"JAN\0FEB\0MAR\0APR\0MAY\0JUN\0JUL\0AUG\0SEP\0OCT\0NOV\0DEC"[g_time_month * 4];
    sprintf_(text, "%s %s %d", weekday, month, g_time_monthday);

    char hour[16], min[16];
    sprintf_(hour, "%02d", g_time_hour);
    sprintf_(min, "%02d", g_time_minute);

    text_drawline(font_bebas2, hour, 148, top + 65);
    text_drawline(font_bebas2, min, 131, top + 130);
    text_drawline(font_bebas3, text, 110, top + 162);

    draw_diagonal(129, top + 0, 86, top + 172, 8);

    text_drawline(font_bebas3, "10000", 40, top + 42);
    text_drawline(font_bebas3, "100%", 40, top + 82);
    text_drawline(font_bebas3, "200", 40, top + 122);

    roundedrect_round(15, top + 180, 210, 48, 5, (25 | (40 << 5) | (25 << 11)));
}

void watchface_handle(ui_event_t *e) {
    int start, lines;

    handle_inertial(&m_watchface_inertial, e);
    ui_update_scrolloff(g_top + m_watchface_inertial.scroll, &start, &lines);
    if (e->type == UI_EVENT_REDRAW) {
        start = g_top;
        lines = 240;
    }

    DO_DRAW(0, start, 240, lines, {
        applist_draw(g_top + -240);
        watchface_draw(g_top + 0);
        messagelist_draw(g_top + 240);
    });

    if (m_watchface_inertial.type == SCROLL_NONE && m_watchface_inertial.scroll == -240) {
        g_handler = applist_handle;
        m_watchface_inertial.scroll = 0;
        g_top -= 240;
    }

    if (m_watchface_inertial.type == SCROLL_NONE && m_watchface_inertial.scroll == 240) {
        g_handler = messagelist_handle;
        m_watchface_inertial.scroll = 0;
        g_top += 240;
    }
}
