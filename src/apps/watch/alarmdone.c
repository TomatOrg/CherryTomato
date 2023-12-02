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
#include "ui.h"
#include "timer.h"
#include <util/divmod.h>
#include "roundedrect.h"
#include "util/log.h"
#include <util/except.h>

static ui_timer_t m_timer;
static bool m_closing_animation = false;
static uint64_t m_closing_animation_start;
static int m_closing_animation_oldy;

void alarmdone_draw(int top) {
    char string[16];
    char *mode;
    char *activebutton;
    char *secondbutton;
    if (m_timer.type == 0) {
        sprintf_(string, "%02d:%02d", m_timer.display_hour, m_timer.display_minute);
        mode = "ALARM";
        secondbutton = "Stop";
        activebutton = "Snooze";
    } else if (m_timer.type == 1) {
        sprintf_(string, "+%02d:%02d:00", m_timer.display_hour, m_timer.display_minute);
        mode = "TIMER";
        secondbutton = "Repeat";
        activebutton = "Stop";
    } else ASSERT(false);
    int size, startx;

    size = text_getlinesize(font_bebas3, mode);
    startx = (240 - size) / 2;
    text_drawline(font_bebas3, mode, startx, top + 130);

    size = text_getlinesize(font_bebas4, string);
    startx = (240 - size) / 2;
    text_drawline(font_bebas4, string, startx, top + 90);

    roundedrect(20, top + 160, 90, 60, (10) | (20 << 5) | (10 << 11));
    size = text_getlinesize(font_roboto, secondbutton);
    text_drawline(font_roboto, secondbutton, 20 + (90 - size) / 2, top + 160 + (60 + 24/2) / 2);

    roundedrect(130, top + 160, 90, 60, (5) | (15 << 5) | (20 << 11));
    size = text_getlinesize(font_roboto, activebutton);
    text_drawline(font_roboto, activebutton, 130 + (90 - size) / 2, top + 160 + (60 + 24/2) / 2);
}

void alarmdone_handle(ui_event_t *e) {
    int start = 0, lines = 0;

    if (e->type == UI_EVENT_REDRAW) {
        start = g_top;
        lines = 240;
        ASSERT(timers_count > 0);
        m_timer = timers[0];
    }

    if (!m_closing_animation) {
        g_pitch = 240;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = start + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            timer_draw(g_top + -240);
            alarmdone_draw(g_top + 0);
            messagelist_draw(g_top + 240);
            plat_update(0, g_line, 240, g_nlines);
        }
    }
    if (!m_closing_animation && e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_UP) {
        int button_num = -1;
        int tx = e->touchevent.x, ty = e->touchevent.y;
        if (tx >= 20 && tx < 20 + 90 && ty >= 160 && ty < 160 + 60) button_num = 0;
        else if (tx >= 130 && tx < 130 + 90 && ty >= 160 && ty < 160 + 60) button_num = 1;
        if (button_num != -1) m_closing_animation = true;

        bool alarm_snooze = m_timer.type == 0 && button_num == 1;
        bool timer_repeat = m_timer.type == 1 && button_num == 0;

        // remove the timer in all cases, we are gonna readd it if needed
        if (timers_count > 0) {
            timers_count--;
            memmove(timers, timers + 1, timers_count);
        }

        if (alarm_snooze) {
            m_timer.minute += 5;
            m_timer.display_minute += 5;
            if (m_timer.minute >= 60) { m_timer.minute -= 60; m_timer.hour = (m_timer.hour + 1) % 24; }
            if (m_timer.display_minute >= 60) { m_timer.display_minute -= 60; m_timer.display_hour = (m_timer.display_hour + 1) % 24; }
            timer_add(&m_timer);
        }

        if (timer_repeat) {
            m_timer.minute += m_timer.display_minute;
            m_timer.hour += m_timer.display_hour;
            if (m_timer.minute >= 60) { m_timer.minute -= 60; m_timer.hour++; }
            m_timer.hour %= 24;
            timer_add(&m_timer);
        }

        if (button_num != -1) {
            m_closing_animation = true;
            m_closing_animation_oldy = 0;
            m_closing_animation_start = get_system_time() / 1000;
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
            g_handler = watchface_handle;
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
}
