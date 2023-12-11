#include <stdbool.h>
#include <stdint.h>
#include <util/printf.h>
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
#include "ui.h"
#include <util/divmod.h>
#include "roundedrect.h"
#include <util/except.h>
#include "back.h"

static bool m_back = false;
static uint64_t m_back_start = 0;
static int m_back_prev = 0;

static int m_rubberband_prev_start = 0;
static bool m_rubberband = false;

static float rubberband(float x, float coeff, float dim) {
    return (1.0 - (1.0 / ((x * coeff / dim) + 1.0))) * dim;
}

bool back_handle(ui_event_t *e) {
    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_DOWN) {
        m_rubberband = e->touchevent.y <= 20;
    }

    if (m_rubberband && e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_CONTACT) {
        g_pitch = 40;
        int y = rubberband(e->touchevent.y, 0.35, 240);
        int start = MIN(m_rubberband_prev_start, y);
        int end = MAX(m_rubberband_prev_start + 40, y + 40);
        int lines = end - start;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = g_scrolloff + l + start - 20;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            roundedrect(0, g_scrolloff + y - 20, 40, 40, 10 | (20 << 5) | (10 << 11));
            text_drawicon(icon_arrow_down, 0, g_scrolloff + y - 19);
            plat_update(100, g_line, 40, g_nlines);
        }
        m_rubberband_prev_start = y;
    }

    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_UP) {
        if (m_rubberband) {
            m_back = true;
            m_back_start = get_system_time() / 1000;
        }
    }

    if (m_back) {
        float t = ((get_system_time() / 1000) - m_back_start) / 1000.0;
        t *= 8.0;
        float off = 240 - spring_ex(240, 0, t, 10, 1);
        int y = off;
        if (y >= 239) {
            y = 240;
            m_back = false;
            g_top = g_scrolloff;
            g_handler = watchface_handle;
        }
        int lines = y - m_back_prev;
        g_pitch = 240;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = g_scrolloff + m_back_prev + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            watchface_draw(g_scrolloff);
            plat_update(0, g_line, 240, g_nlines);
        }

        for (int i = 0; i < 240; i++)
            g_target[240*0 + i] = 31 | (63 << 5) | (31 << 11);
        for (int i = 0; i < 240; i++)
            g_target[240*1 + i] = 15 | (31 << 5) | (15 << 11);
        plat_update(0, g_scrolloff + y, 240, 2);
        m_back_prev = y;
    }

    return m_back || m_rubberband;
}