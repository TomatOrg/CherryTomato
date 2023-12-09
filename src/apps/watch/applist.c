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
#include "task/time.h"
#include "text.h"
#include "gesturerecognizer.h"
#include "roundedrect.h"
#include "ui.h"
#include "util/log.h"
#include <util/divmod.h>

inertial_state_t m_applist_inertial = {.has_top_constraint = 1, .has_bottom_constraint = 1};

static const uint16_t gray = 7 | (14 << 5) | (7 << 11);
static const uint16_t green = 10 | (45 << 5) | (2 << 11);

uint8_t *icon_array[2][3] = {
    { icon_timernew, icon_timeredit, icon_calculator },
    { icon_moon, icon_bell_crossed, icon_lightbulb }
};

void applist_draw(int top) {
    roundedrect(12, top + 20, 64, 64, gray);
    text_drawicon(icon_timernew, 12, top + 20);
    roundedrect(87, top + 20, 64, 64, gray);
    text_drawicon(icon_timeredit, 87, top + 20);
    roundedrect(162, top + 20, 64, 64, gray);
    text_drawicon(icon_calculator, 162, top + 20);

    roundedrect(12, top + 95, 64, 64, gray);
    text_drawicon(icon_moon, 12, top + 95);
    roundedrect(87, top + 95, 64, 64, gray);
    text_drawicon(icon_bell_crossed, 87, top + 95);
    roundedrect(162, top + 95, 64, 64, gray);
    text_drawicon(icon_lightbulb, 162, top + 95);

}


static drawer_t* m_new_drawer;
static handler_t* m_new_handler;

static bool m_closing_animation = false;
static uint64_t m_closing_animation_start;
static int m_closing_animation_oldy;

static bool m_state[3] = {};

void applist_handle(ui_event_t *e) {
    int start, lines;

    if (e->type == UI_EVENT_REDRAW) {
        start = g_top;
        lines = 240;
    }

    bool recognized, vertical, tap;
    int startx, starty;
    gesture_recognizer(e, &recognized, &vertical, &tap, &startx, &starty);

    bool starting_vertical = (recognized && vertical);
    bool in_scroll = m_applist_inertial.type != SCROLL_NONE;
    bool is_vertical_movement = !m_closing_animation && (starting_vertical || in_scroll);

    if (is_vertical_movement) {
        handle_inertial(&m_applist_inertial, e);
        ui_update_scrolloff(g_top + m_applist_inertial.scroll, &start, &lines);
        g_pitch = 240;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = start + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            applist_draw(g_top + 0);
            watchface_draw(g_top + 240);
            plat_update(0, g_line, 240, g_nlines);
        }

        if (m_applist_inertial.type == SCROLL_NONE && m_applist_inertial.scroll == -240) {
            g_handler = applist_handle;
            m_applist_inertial.scroll = 0;
            g_top -= 240;
        }

        if (m_applist_inertial.type == SCROLL_NONE && m_applist_inertial.scroll == 240) {
            g_handler = watchface_handle;
            m_applist_inertial.scroll = 0;
            g_top += 240;
        }
    }


    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_UP && tap) {
        int bx = (e->touchevent.x - 12) / 75;
        int by = (e->touchevent.y - 20) / 75;
        if ((bx==0 && by==0) || (bx==1 && by==0)) {
            m_new_drawer = timer_draw;
            m_new_handler = timer_handle;
            m_closing_animation = true;
            m_closing_animation_oldy = 0;
            m_closing_animation_start = get_system_time() / 1000;
        }

        if (bx==2 && by==0){
            m_new_drawer = calculator_draw;
            m_new_handler = calculator_handle;
            m_closing_animation = true;
            m_closing_animation_oldy = 0;
            m_closing_animation_start = get_system_time() / 1000;
        }

        if (by==1) {
            m_state[bx] = !m_state[bx];
            // TODO: do an animation to draw in a square
            g_pitch = 64;
            int start = g_top + 20 + by * 75;
            int lines = 64;
            for (int l = 0; l < lines; l += NLINES) {
                g_line = start + l;
                g_nlines = MIN(lines - l, NLINES);
                memset(g_target, 0, 240 * 2 * NLINES);
                roundedrect(0, g_top + 20 + by * 75, 64, 64, m_state[bx] ? green : gray);
                if (icon_array[by][bx]) text_drawicon(icon_array[by][bx], 0, g_top + 20 + by * 75);
                plat_update(12 + bx * 75, g_line, 64, g_nlines);
            }
        }

        g_pitch = 240;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = start + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            applist_draw(g_top + 0);
            watchface_draw(g_top + 240);
            plat_update(0, g_line, 240, g_nlines);
        }
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
            g_handler = m_new_handler;
        } else g_frame_requested = true;

        int y = (int)half;
        int lines = y - m_closing_animation_oldy;
        g_pitch = 240;

        for (int l = 0; l < lines; l += NLINES) {
            g_line = g_scrolloff + 120 - y + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            m_new_drawer(g_scrolloff);
            plat_update(0, g_line, 240, g_nlines);
        }
        g_pitch = 240;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = g_scrolloff + 120 + m_closing_animation_oldy + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            m_new_drawer(g_scrolloff);
            plat_update(0, g_line, 240, g_nlines);
        }
        m_closing_animation_oldy = y;
    }
}
