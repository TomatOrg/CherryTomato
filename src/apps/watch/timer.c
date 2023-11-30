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
#include "task/time.h"
#include "text.h"
#include "ui.h"
#include "roundedrect.h"
#include "util/except.h"
#include "util/log.h"
#include "timer.h"
#include <util/divmod.h>

// ------------------------------------
// timer list
// ------------------------------------

ui_timer_t timers[TIMERS_MAX];
int timers_count = 0;

void timer_add(ui_timer_t* t) {
    ASSERT(timers_count < 16);
    timers[timers_count] = *t;
    timers_count++;
}

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


static int m_type = 0;
static int m_notifstate = 0;

void redraw_checkboxes(int top) {
    uint16_t green = 10 | (50 << 5) | (10 << 11);
    uint16_t gray = 10 | (20 << 5) | (10 << 11);
    uint16_t darkgray = 6 | (12 << 5) | (6 << 11);

    int off;
    if (m_type == 0) off = 0;
    else off = 75;
    roundedrect_round((240 - 160) / 2 , top + 10, 156, 40, 14, darkgray);
    roundedrect_round((240 - 160) / 2 + off + 3, top + 10 + 3, 75, 40 - 6, 11, gray);

    text_drawline(font_roboto, "Alarm", (240 - 160) / 2 + 16, top + 10 + 26);
    text_drawline(font_roboto, "Timer", (240 - 160) / 2 + 75 + 16, top + 10 + 26);


    text_drawline(font_roboto, "Vibrate", 50, top + 80);
    roundedrect(20, top + 66, 14, 14, (m_notifstate == 0) ? green : gray);

    text_drawline(font_roboto, "Notify", 50 + 100, top + 80);
    roundedrect(20 + 100, top + 66, 14, 14, (m_notifstate == 1) ? green : gray);
}

static int m_prevtime = -1;
static int m_oldsize = 0;

void get_values(int *h, int *m) {
    *h = floormod(floordiv(MAX(0, m_timers_scrolloff[0]), 40), 60);
    *m = floormod(floordiv(m_timers_scrolloff[1], 40), 60);
}

static int m_currentminute = 13*60+37;

void draw_hinttext() {
    int start, lines;
    char str[128];
    int h, m;
    get_values(&h, &m);
    int minutes = h * 60 + m;

    // TODO: check also currentminute
    if (m_prevtime == minutes) return;

    if (m_type == 0) { // Alarm
        int dm = minutes - m_currentminute;
        if (dm < 0) dm += 24 * 60;
        int m = dm % 60;
        int h = dm / 60;
        if (h == 0) {
            sprintf_(str, "In %d minutes", m);
        } else {
            sprintf_(str, "In %d hours %02d minutes", h, m);
        }
    }
    else if (m_type == 1) {
        int dm = m_currentminute + minutes;
        int m = dm % 60;
        int h = (dm / 60) % 24;
        sprintf_(str, "At %d:%02d", h, m);
    }
    int textsize = text_getlinesize(font_roboto, str);
    g_pitch = MAX(m_oldsize, textsize);
    start = g_top + 105;
    lines = 20;
    int xoff = (240 - textsize) / 2;
    int real_xoff = xoff - (g_pitch - textsize) / 2;
    for (int l = 0; l < lines; l += NLINES) {
        g_line = start + l;
        g_nlines = MIN(lines - l, NLINES);
        memset(g_target, 0, 240 * 2 * NLINES);
        text_drawline(font_roboto, str, (g_pitch - textsize) / 2, g_top + 105 + 20);
        plat_update(real_xoff, start + l, g_pitch, g_nlines);
    }

    m_oldsize = textsize;
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

    // draw_hinttext();

    roundedrect(160, top + 169, 50, 50, 10 | (20 << 5) | (10 << 11));
    text_drawline(font_roboto, "+", 180, top + 200);
}

static bool m_is_buttonpress = false;
static bool m_closing_animation = false;
static uint64_t m_closing_animation_start;
static bool m_closing_animation_oldy;

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

        if (tx >= 0 && tx < 120) { col = 0; }
        if (tx >= 120 && tx < 240) { col = 1; }

        if (col != -1 && row != -1) {
            if (row == 0) {
                m_type = col;
                g_pitch = 240; // TODO: this only needs to be 140px
                int start = g_top + 10;
                int lines = 40;
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

            m_prevtime = -1; // need to redraw text
            draw_hinttext();
        }

        if (ty >= 120 && ty < 240) {
            if (tx >= 0 && tx < 75) m_curr_selected = 0;
            else if (tx >= 75 && tx < 145) m_curr_selected = 1;
        }
        if (m_curr_selected != -1) m_timer_inertial.type = SCROLL_NONE;
    }

    if (e->type == UI_EVENT_TOUCH && (e->touchevent.action == TOUCHACTION_DOWN || e->touchevent.action == TOUCHACTION_UP)) {
        // roundedrect(160, g_top + 169, 50, 50, 20 | (40 << 5) | (20 << 11));
        int tx = e->touchevent.x, ty = m_timer_inertial.scroll + e->touchevent.y;
        bool pressed = tx >= 160 && tx < 160+50 && ty >= 169 && ty < 169+50;
        if (pressed && e->touchevent.action == TOUCHACTION_DOWN) m_is_buttonpress = true;
        if (e->touchevent.action == TOUCHACTION_UP) {
            int h, m;
            get_values(&h, &m);
            if (pressed && m_is_buttonpress && !(h == 0 && m == 0)) {
                m_closing_animation = true;
                m_closing_animation_oldy = 0;
                m_closing_animation_start = get_system_time() / 1000;
            }
            m_is_buttonpress = false;
        }
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
        draw_hinttext();
    }

    if (m_closing_animation) {
        float t = ((get_system_time() / 1000) - m_closing_animation_start) / 1000.0;
        t *= 8.0;
        float half = 120 - spring_ex(120, 0, t, 10, 1);
        if (half >= 119) {
            half = 120;
            g_frame_requested = false;
            m_closing_animation = false;
            g_top = g_scrolloff;
            g_startscroll_above = 0;
            g_handler = watchface_handle;


            // add the timer
            ui_timer_t t = { .type = m_type };
            get_values(&t.display_hour, &t.display_minute);
            if (m_type == 0) { // alarm
                t.hour = t.display_hour;
                t.minute = t.display_minute;
            } else if (m_type == 1) { // timer
                int dm = m_currentminute + t.hour * 60 + t.minute;
                t.hour = (dm / 60) % 24;
                t.minute = dm % 60;
            }
            timer_add(&t);
        } else g_frame_requested = true;
        int y = (int)half;

        int lines = y - m_closing_animation_oldy;
        g_pitch = 240;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = g_scrolloff + 120 - y + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            watchface_draw(g_scrolloff);
            plat_update(0, g_line, 240, g_nlines);
        }
        g_pitch = 240;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = g_scrolloff + 120 + m_closing_animation_oldy + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            watchface_draw(g_scrolloff);
            plat_update(0, g_line, 240, g_nlines);
        }
        m_closing_animation_oldy = y;
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
