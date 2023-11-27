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
    roundedrect(20, top + 20, 50, 50, 0xFFFF);
    roundedrect(95, top + 20, 50, 50, 0xFFFF);
    roundedrect(170, top + 20, 50, 50, 0xFFFF);
}

static int m_context_menu_oldheight;
static uint64_t m_context_menu_starttime;
static bool m_context_menu_in_anim;

void contextmenu_draw() {
    roundedrect_round(0, g_top + 45, 100, 100, 10, (7) | (14 << 5) | (7 << 11));
    // pretend the + is an icon for now
    text_drawline(font_roboto, "+  Alarm", 10, g_top + 45 + 25);
    text_drawline(font_roboto, "+  Timer", 10, g_top + 45 + 25 + 30);
    text_drawline(font_roboto, "/   Edit", 10, g_top + 45 + 25 + 30 + 30);
}

void applist_handle(ui_event_t *e) {
    int start, lines;

    handle_inertial(&m_applist_inertial, e);
    ui_update_scrolloff(g_top + m_applist_inertial.scroll, &start, &lines);
    if (e->type == UI_EVENT_REDRAW) {
        start = g_top;
        lines = 240;
    }

    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_UP) {
        if (!m_context_menu_in_anim) {
            m_context_menu_in_anim = true;
            m_context_menu_oldheight = 0;
            m_context_menu_starttime = get_system_time() / 1000;
        }
    }

    if (m_context_menu_in_anim) {
        g_pitch = 100;
        float elapsed = ((get_system_time() / 1000) - m_context_menu_starttime) / 1000.0;
        int height = 100 - spring_ex(100, 0, elapsed, 1500, 1);

        int start = g_top + 45 + m_context_menu_oldheight; // g_top + 45 + m_context_menu_oldheight;
        int lines = height - m_context_menu_oldheight;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = start + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            contextmenu_draw();
            plat_update(30, g_line, 100, g_nlines);
        }
        m_context_menu_oldheight = height;

        if (height >= 99) m_context_menu_in_anim = false;

    }

    g_pitch = 240;
    for (int l = 0; l < lines; l += NLINES) {
        g_line = start + l;
        g_nlines = MIN(lines - l, NLINES);
        memset(g_target, 0, 240 * 2 * NLINES);
        timer_draw(g_top + -240);
        applist_draw(g_top + 0);
        messagelist_draw(g_top + 240);
        plat_update(0, g_line, 240, g_nlines);
    }

    if (m_applist_inertial.type == SCROLL_NONE && m_applist_inertial.scroll == -240) {
        g_handler = timer_handle;
        m_applist_inertial.scroll = 0;
        g_top -= 240;
    }

    if (m_applist_inertial.type == SCROLL_NONE && m_applist_inertial.scroll == 240) {
        g_handler = messagelist_handle;
        m_applist_inertial.scroll = 0;
        g_top += 240;
    }
}
