#include <stdbool.h>
#include <stdint.h>
#include <util/printf.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "event.h"
#include "font.h"
#include "physics.h"
#include "plat.h"
#include "text.h"
#include "ui.h"
#include "roundedrect.h"
#include "util/log.h"
#include <util/divmod.h>

static int m_timers_scrolloff[2] = {0, 0};
static int m_curr_selected = -1;
static inertial_state_t m_timer_inertial = {.has_top_constraint = 0, .has_bottom_constraint = 1};
static inertial_state_t m_scrollbar_inertial = {.round = true};

void draw_scrollbar(bool wraparound, int x, int timer_scrolloff, int top) {
    int n = -floordiv(-timer_scrolloff, 40);
    int o = floormod(-timer_scrolloff, 40);
    for (int i = -2; i <= 2; i++) {
        int num = wraparound ? floormod(n + i, 60) : ((n + i) % 24);
        if (wraparound || (!wraparound && num >= 0)) {
            // TODO: dont use sprintf_
            static char buf[3];
            sprintf_(buf, "%02d", num);
            text_drawline(font_bebas4, buf, x, o + top + 210 + 40 * i);
        }
    }
    for (int l = 0; l < g_nlines; l++) {
        int fading = 48 - ABS((top + 240 - 48) - (g_line + l));
        if (fading < 0) fading = 0;
        if (fading > 48) fading = 48;
        // TODO: rewrite this
        for (int i = 0; i < 40; i++) {
            uint16_t px = __builtin_bswap16(g_target[g_pitch * l + x + i]);
            uint32_t r = ((px >> 0) & ((1 << 5) - 1)) * fading / 48;
            uint32_t g = ((px >> 5) & ((1 << 6) - 1)) * fading / 48;
            uint32_t b = ((px >> 11) & ((1 << 5) - 1)) * fading / 48;
            g_target[g_pitch * l + x + i] = __builtin_bswap16(r | (g << 5) | (b << 11));
        }
    }
}


static int m_mode = 0;
static int m_notifstate = 0;

void redraw_checkboxes(int top) {
    uint16_t green = 10 | (50 << 5) | (10 << 11);
    uint16_t gray = 10 | (20 << 5) | (10 << 11);

    text_drawline(font_roboto, "Alarm", 50, top + 40);
    roundedrect(20, top + 26, 14, 14, (m_mode == 0) ? green : gray);

    text_drawline(font_roboto, "Timer", 50 + 100, top + 40);
    roundedrect(20 + 100, top + 26, 14, 14, (m_mode == 1) ? green : gray);

    text_drawline(font_roboto, "Vibrate", 50, top + 80);
    roundedrect(20, top + 66, 14, 14, (m_notifstate == 0) ? green : gray);

    text_drawline(font_roboto, "Notify", 50 + 100, top + 80);
    roundedrect(20 + 100, top + 66, 14, 14, (m_notifstate == 1) ? green : gray);
}


void timer_draw(int top) {
    draw_scrollbar(false, 90 - 70, m_timers_scrolloff[0], top);
    draw_scrollbar(true, 90, m_timers_scrolloff[1], top);

    for (int l = 0; l < g_nlines; l++) {
        int fading = 48 - ABS((top + 240 - 48) - (g_line + l));
        if (fading < 0) fading = 0;
        if (fading > 48) fading = 48;
        // draw dividers
        uint32_t v = fading * 31 / 48;
        uint32_t v2 = fading * 31 / 48 / 2;

        g_target[g_pitch * l + 75-1] = __builtin_bswap16(v2 | ((v2 * 2) << 5) | (v2 << 11));
        g_target[g_pitch * l + 75] = __builtin_bswap16(v | ((v * 2) << 5) | (v << 11));
        g_target[g_pitch * l + 75+1] = __builtin_bswap16(v2 | ((v2 * 2) << 5) | (v2 << 11));

        g_target[g_pitch * l + 145-1] = __builtin_bswap16(v2 | ((v2 * 2) << 5) | (v2 << 11));
        g_target[g_pitch * l + 145] = __builtin_bswap16(v | ((v * 2) << 5) | (v << 11));
        g_target[g_pitch * l + 145+1] = __builtin_bswap16(v2 | ((v2 * 2) << 5) | (v2 << 11));
    }

    redraw_checkboxes(top);


    text_drawline(font_roboto, "+", 180, top + 200);
}

void timer_handle(ui_event_t *e) {
    int start, lines;
    bool is_viewscroll = false;

    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_DOWN) {
        int tx = e->touchevent.x, ty = m_timer_inertial.scroll + e->touchevent.y;
        m_curr_selected = -1;

        int row = -1;
        int col = -1;

        if (ty >= 0 && ty < 60) { row = 0; }
        if (ty >= 60 && ty < 100) { row = 1; }

        if (tx >= 0 && tx < 100) { col = 0; }
        if (tx >= 100 && tx < 240) { col = 1; }

        if (col != -1 && row != -1) {
            if (row == 0) {
                m_mode = col;
                g_pitch = 240;
                int start = g_top + 20;
                int lines = 30;
                for (int l = 0; l < lines; l += NLINES) {
                    g_line = start + l;
                    g_nlines = MIN(lines - l, NLINES);
                    memset(g_target, 0, 240 * 2 * NLINES);
                    redraw_checkboxes(g_top);
                    plat_update(0, start + l, 240, g_nlines);
                }
            } else if (row == 1) {
                m_notifstate = col;
                g_pitch = 240;
                int start = g_top + 60;
                int lines = 30;
                for (int l = 0; l < lines; l += NLINES) {
                    g_line = start + l;
                    g_nlines = MIN(lines - l, NLINES);
                    memset(g_target, 0, 240 * 2 * NLINES);
                    redraw_checkboxes(g_top);
                    plat_update(0, start + l, 240, g_nlines);
                }
            }
        }

        if (ty >= 120 && ty < 240) {
            if (tx >= 0 && tx < 75) m_curr_selected = 0;
            else if (tx >= 75 && tx < 145) m_curr_selected = 1;
        }
        if (m_curr_selected != -1) m_timer_inertial.type = SCROLL_NONE;
    }
    if (m_curr_selected == -1) {
        handle_inertial(&m_timer_inertial, e);
        ui_update_scrolloff(g_top + m_timer_inertial.scroll, &start, &lines);
        is_viewscroll = true;
    }

    if (m_curr_selected != -1 || m_scrollbar_inertial.type != SCROLL_NONE) {
        int oldscro = m_timers_scrolloff[m_curr_selected];
        m_scrollbar_inertial.scroll = oldscro;
        bool wrap = (m_curr_selected == 0);
        m_scrollbar_inertial.has_top_constraint = wrap;
        handle_inertial(&m_scrollbar_inertial, e);
        m_timers_scrolloff[m_curr_selected] = m_scrollbar_inertial.scroll;

        int startoff = 90 + 70 * (m_curr_selected - 1);
        start = g_top + (240 - 96);
        lines = 96;
        g_pitch = 40;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = start + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            draw_scrollbar(m_curr_selected != 0, 0, m_timers_scrolloff[m_curr_selected], g_top);
            plat_update(startoff, start + l, 40, g_nlines);
        }
    }

    if (is_viewscroll) {
        g_pitch = 240;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = start + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            timer_draw(g_top);
            watchface_draw(g_top + 240);
            plat_update(0, start + l, 240, g_nlines);
        }

        if (m_timer_inertial.type == SCROLL_NONE && m_timer_inertial.scroll == 240) {
            g_handler = watchface_handle;
            m_timer_inertial.scroll = 0;
            g_top += 240;
        }
    }
}
