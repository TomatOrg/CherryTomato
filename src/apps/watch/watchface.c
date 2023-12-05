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
#include "ui.h"
#include <util/divmod.h>

int g_time_month = 0;
int g_time_monthday = 0;
int g_time_hour = 0;
int g_time_minute = 0;
int g_time_weekday = 0;
inertial_state_t m_watchface_inertial = {.has_top_constraint = 1, .has_bottom_constraint = 1};

void watchface_draw(int top) {
    char month_and_day[16];
    memcpy(month_and_day, &"JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC"[g_time_month * 3], 3);
    month_and_day[3] = ' ';
    month_and_day[4] = '0' + g_time_monthday / 10;
    month_and_day[5] = '0' + g_time_monthday % 10;
    month_and_day[6] = 0;
    char *dayoftheweek = &"SUN\0MON\0TUE\0THU\0WED\0SAT\0FRI\0"[g_time_weekday * 4];
    // TODO: not use sprintf_ here
    char messages[8];
    sprintf_(messages, "%d", g_messages_num);

    text_drawchar(font_bebas1, g_time_hour / 10, 10, 140 + top);
    text_drawchar(font_bebas1, g_time_hour % 10, 10 + 56 + 7, 140 + top);

    text_drawchar(font_bebas2, g_time_minute / 10, 10 + 56 + 7 + 56 + 16, 76 + top);
    text_drawchar(font_bebas2, g_time_minute % 10, 10 + 56 + 7 + 56 + 16 + 32, 76 + top);

    text_drawline(font_bebas3, dayoftheweek, 10 + 56 + 7 + 56 + 16 + 4, 110 + top);
    text_drawline(font_bebas3, month_and_day, 10 + 56 + 7 + 56 + 16 + 4, 140 + top);
}

void watchface_handle(ui_event_t *e) {
    int start, lines;

    handle_inertial(&m_watchface_inertial, e);
    ui_update_scrolloff(g_top + m_watchface_inertial.scroll, &start, &lines);
    if (e->type == UI_EVENT_REDRAW) {
        start = g_top;
        lines = 240;
    }

    g_pitch = 240;
    for (int l = 0; l < lines; l += NLINES) {
        g_line = start + l;
        g_nlines = MIN(lines - l, NLINES);
        memset(g_target, 0, 240 * 2 * NLINES);
        applist_draw(g_top + -240);
        watchface_draw(g_top + 0);
        messagelist_draw(g_top + 240);
        plat_update(0, g_line, 240, g_nlines);
    }

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
