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
#include <util/divmod.h>

static int m_timers_scrolloff[3] = {0, 0, 0};
static int m_curr_selected = -1;
static inertial_state_t m_timer_inertial = {.has_top_constraint = 0, .has_bottom_constraint = 1};
static inertial_state_t m_scrollbar_inertial = {.round = true};

void draw_scrollbar(bool wraparound, int x, int timer_scrolloff, int top) {
    int n = -floordiv(-timer_scrolloff, 40);
    int o = floormod(-timer_scrolloff, 40);
    for (int i = -1; i <= 1; i++) {
        int num = wraparound ? floormod(n + i, 60) : ((n + i) % 24);
        if (wraparound || (!wraparound && num >= 0)) {
            // TODO: dont use sprintf_
            static char buf[3];
            sprintf_(buf, "%02d", num);
            text_drawline(font_bebas4, buf, x, o + top + 210 + 40 * i);
        }
    }
    for (int l = 0; l < g_nlines; l++) {
        int fading = 32 - ABS((top + 210 - 16) - (g_line + l));
        if (fading < 0) fading = 0;
        if (fading > 32) fading = 32;
        // TODO: rewrite this
        for (int i = 0; i < 60; i++) {
            uint16_t px = __builtin_bswap16(g_target[g_pitch * l + x + i]);
            uint32_t r = ((px >> 0) & ((1 << 5) - 1)) * fading / 32;
            uint32_t g = ((px >> 5) & ((1 << 6) - 1)) * fading / 32;
            uint32_t b = ((px >> 11) & ((1 << 5) - 1)) * fading / 32;
            g_target[g_pitch * l + x + i] = __builtin_bswap16(r | (g << 5) | (b << 11));
        }
    }
}

void timer_draw(int top) {
    draw_scrollbar(false, 60 + 20 - 60, m_timers_scrolloff[0], top);
    draw_scrollbar(true, 60 + 20, m_timers_scrolloff[1], top);
    draw_scrollbar(true, 60 + 20 + 60, m_timers_scrolloff[2], top);
}

void timer_handle(ui_event_t *e) {
    int start, lines;
    bool is_viewscroll = false;

    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_DOWN) {
        int tx = e->touchevent.x, ty = m_timer_inertial.scroll + e->touchevent.y;
        m_curr_selected = -1;
        if (ty >= 160) {
            if (tx >= (60 + 20 - 60) && tx < (60 + 20)) m_curr_selected = 0;
            else if (tx >= (60 + 20) && tx < (60 + 20 + 60)) m_curr_selected = 1;
            else if (tx >= (60 + 20 + 60) && tx < (60 + 20 + 60 + 60)) m_curr_selected = 2;
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

        int startoff = 60 + 20 - 60 + 60 * m_curr_selected;
        start = g_top + 160;
        lines = 64;
        g_pitch = 60;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = start + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            draw_scrollbar(m_curr_selected != 0, 0, m_timers_scrolloff[m_curr_selected], g_top);
            plat_update(startoff, start + l, 60, g_nlines);
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
