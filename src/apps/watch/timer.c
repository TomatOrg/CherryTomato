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
#include "back.h"
#include "roundedrect.h"
#include "util/except.h"
#include "util/log.h"
#include "timer.h"
#include <util/divmod.h>

// ------------------------------------
// timer list
// ------------------------------------
static int m_timer_toedit = -1;
ui_timer_t g_timers[TIMERS_MAX];
int g_timers_count = 0;

void timer_add(ui_timer_t* t) {
    ASSERT(g_timers_count < 16);
    g_timers[g_timers_count] = *t;
    g_timers_count++;
}


static int m_timers_scrolloff[2] = {0, 0};
static int m_curr_selected = -1;
static inertial_state_t m_timer_inertial = {.has_top_constraint = 1, .has_bottom_constraint = 1};
static inertial_state_t m_scrollbar_inertial = {.round = true};

void draw_scrollbar(bool wraparound, int x, int timer_scrolloff, int top) {
    int n = -floordiv(-timer_scrolloff, 40);
    int o = floormod(-timer_scrolloff, 40);
    for (int i = -2; i <= 2; i++) {
        int num = wraparound ? floormod(n + i, 60) : ((n + i) % 24);
        if (wraparound || (!wraparound && num >= 0)) {
            // TODO: dont use sprintf_
            static char buf[3];
            buf[0] = num / 10 + '0';
            buf[1] = num % 10 + '0';
            buf[2] = 0;
            text_drawline(font_bebas4, buf, x, o + top + 210 + 40 * i);
        }
    }
    for (int l = 0; l < g_nlines; l++) {
        int fading = 48 - ABS((top + 240 - 48) - (g_line + l));
        if (fading < 0) fading = 0;
        if (fading > 48) fading = 48;
        // TODO: this is a surprising bottleneck, rewrite this
        for (int i = 0; i < 40; i++) {
            uint16_t px = __builtin_bswap16(g_target[g_pitch * l + x + i]);
            uint32_t r = ((px >> 0) & 31) * fading * (256/48) / 256;
            uint32_t g = ((px >> 5) & 63) * fading * (256/48) / 256;
            uint32_t b = ((px >> 11) & 31) * fading * (256/48) / 256;
            g_target[g_pitch * l + x + i] = __builtin_bswap16(r | (g << 5) | (b << 11));
        }
    }
}


static int m_type = 0;
static int m_notifstate = 0;


void timer_startedit(int toedit) {
    ASSERT(toedit >= 0 && toedit < g_timers_count);
    m_timer_toedit = toedit;
    m_type = g_timers[toedit].type;
    m_notifstate = g_timers[toedit].notifstate;
    m_timers_scrolloff[0] = g_timers[toedit].display_hour * 40;
    m_timers_scrolloff[1] = g_timers[toedit].display_minute * 40;
}

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
        int h = (dm / 60) % 24;
        if (h == 0) {
            sprintf_(str, "In %d minutes", m);
        } else {
            sprintf_(str, "In %d hours %02d minutes", h, m);
        }
    } else if (m_type == 1) {
        int dm = m_currentminute + minutes;
        int m = dm % 60;
        int h = (dm / 60) % 24;
        sprintf_(str, "At %d:%02d", h, m);
    }
    int textsize = text_getlinesize(font_roboto, str);
    int width = MAX(m_oldsize, textsize);
    int xoff = (240 - textsize) / 2;
    int real_xoff = xoff - (width - textsize) / 2;

    DO_DRAW(real_xoff, g_top + 105, width, 20,
        text_drawline(font_roboto, str, (g_pitch - textsize) / 2, g_top + 105 + 20));

    m_oldsize = textsize;
}


void timer_draw(int top) {
    int real_nlines = g_nlines;
    g_nlines = MIN(g_line + g_nlines, top + 240) - g_line;

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

    roundedrect(160, top + 169, 50, 50, 10 | (20 << 5) | (10 << 11));
    text_drawline(font_roboto, "+", 180, top + 200);

    g_nlines = real_nlines;
}

static bool m_is_buttonpress = false;
static bool m_pressed = false;
static bool m_inhibit_starting_scroll = false;

static void cleanup() {
    m_timers_scrolloff[0] = 0;
    m_timers_scrolloff[1] = 0;
    m_timer_inertial.scroll = 0;
    m_type = 0;
    m_notifstate = 0;
}

void timer_handle(ui_event_t *e) {
    bool isback = back_handle(e, timer_draw, cleanup);
    if (isback) return;

    bool transition = transition_do(cleanup);
    if (transition) return;

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

        if (row == 0) {
            m_inhibit_starting_scroll = true;
        }

        if (col != -1 && row != -1) {
            int start = 0, lines = 0;
            if (row == 0) {
                m_type = col;
                start = g_top + 10;
                lines = 40;
            } else if (row == 1) {
                m_notifstate = col;
                start = g_top + 60;
                lines = 30;
            }
            if (lines) DO_DRAW(0, start, 240, lines, redraw_checkboxes(g_top));
            m_prevtime = -1; // need to redraw text
            draw_hinttext();
        }

        if (ty >= 120 && ty < 240) {
            if (tx >= 0 && tx < 75) m_curr_selected = 0;
            else if (tx >= 75 && tx < 145) m_curr_selected = 1;
        }
        if (m_curr_selected != -1) {
            m_timer_inertial.type = SCROLL_NONE;
            m_timer_inertial.starttime = 0;
            m_timer_inertial.velocity = 0;
            m_timer_inertial.starttime = 0;
        }
    }

    if (e->type == UI_EVENT_TOUCH && (e->touchevent.action == TOUCHACTION_DOWN || e->touchevent.action == TOUCHACTION_UP)) {
        // roundedrect(160, g_top + 169, 50, 50, 20 | (40 << 5) | (20 << 11));
        int tx = e->touchevent.x, ty = m_timer_inertial.scroll + e->touchevent.y;
        m_pressed = tx >= 160-5 && tx < 160+50+5 && ty >= 169-10 && ty < 169+50+10;
        if (m_pressed && e->touchevent.action == TOUCHACTION_DOWN) m_is_buttonpress = true;
        if (e->touchevent.action == TOUCHACTION_UP) {
            int h, m;
            get_values(&h, &m);
            if (m_pressed && m_is_buttonpress && !(h == 0 && m == 0)) {
                transition_start(watchface_draw, watchface_handle, 0);
                // add the timer
                ui_timer_t t = { .type = m_type, .notifstate = m_notifstate };
                get_values(&t.display_hour, &t.display_minute);
                if (m_type == 0) { // alarm
                    t.hour = t.display_hour;
                    t.minute = t.display_minute;
                } else if (m_type == 1) { // timer
                    int dm = m_currentminute + t.hour * 60 + t.minute;
                    t.hour = (dm / 60) % 24;
                    t.minute = dm % 60;
                }
                if (m_timer_toedit != -1) g_timers[m_timer_toedit] = t;
                else timer_add(&t);
                m_timer_toedit = -1;
            }
            m_is_buttonpress = false;
            m_inhibit_starting_scroll = false;
        }
    }

    if (m_curr_selected == -1 && !m_pressed) {
        bool should_not_start = (m_timer_inertial.type == SCROLL_NONE && m_inhibit_starting_scroll);
        if (!should_not_start) {
            handle_inertial(&m_timer_inertial, e);
            ui_update_scrolloff(g_top + m_timer_inertial.scroll, &start, &lines);
            is_viewscroll = true;
        }
    }

    if (m_curr_selected != -1) {
        int oldscro = m_timers_scrolloff[m_curr_selected];
        m_scrollbar_inertial.scroll = oldscro;
        bool wrap = (m_curr_selected == 0);
        m_scrollbar_inertial.has_top_constraint = wrap;
        handle_inertial(&m_scrollbar_inertial, e);
        m_timers_scrolloff[m_curr_selected] = m_scrollbar_inertial.scroll;

        int startoff = 90 + 70 * (m_curr_selected - 1);
        DO_DRAW(startoff, g_top + (240 - 96), 40, 96,
            draw_scrollbar(m_curr_selected != 0, 0, m_timers_scrolloff[m_curr_selected], g_top));
        draw_hinttext();
    }

    if (is_viewscroll) {
        DO_DRAW(0, start, 240, lines, {
            watchface_draw(g_top + 240);
            timer_draw(g_top);
            applist_draw(g_top - 240);
        })

        if (m_timer_inertial.type == SCROLL_NONE && m_timer_inertial.scroll == -240) {
            cleanup();
            g_handler = applist_handle;
            g_top -= 240;
        }

        if (m_timer_inertial.type == SCROLL_NONE && m_timer_inertial.scroll == 240) {
            cleanup();
            g_handler = watchface_handle;
            g_top += 240;
        }
    }
}
