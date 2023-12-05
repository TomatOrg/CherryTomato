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
#include "thumbnail.h"
#include "roundedrect.h"
#include "ui.h"
#include <util/divmod.h>

inertial_state_t m_applist_inertial = {.has_top_constraint = 1, .has_bottom_constraint = 1};

void applist_draw(int top) {
    uint64_t gray = 11 | (22 << 5) | (11 << 11);
    roundedrect(12, top + 20, 64, 64, gray);
    roundedrect(87, top + 20, 64, 64, gray);
    roundedrect(162, top + 20, 64, 64, gray);

    roundedrect(12, top + 94, 64, 64, gray);
    roundedrect(87, top + 94, 64, 64, gray);
    roundedrect(162, top + 94, 64, 64, gray);
}


static drawer_t* m_new_drawer;
static handler_t* m_new_handler;

static bool m_closing_animation = false;
static uint64_t m_closing_animation_start;
static int m_closing_animation_oldy;

void applist_handle(ui_event_t *e) {
    int start, lines;

    if (e->type == UI_EVENT_REDRAW) {
        start = g_top;
        lines = 240;
    }

    if (!m_closing_animation) {
        handle_inertial(&m_applist_inertial, e);
        ui_update_scrolloff(g_top + m_applist_inertial.scroll, &start, &lines);

        if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_UP) {
            m_new_drawer = timer_draw;
            m_new_handler = timer_handle;
            m_closing_animation = true;
            m_closing_animation_oldy = 0;
            m_closing_animation_start = get_system_time() / 1000;
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
