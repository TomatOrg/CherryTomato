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
#include "util/log.h"
#include <util/except.h>
#include "back.h"

static bool m_back = false;
static bool m_return = false;
static uint64_t m_back_start = 0;
static uint64_t m_return_start = 0;
static int m_back_prev = 0;
static int m_return_prev = 0;

static int m_rubberband_prev_start = 0;
static bool m_rubberband = false;
static int m_rubberband_max = 0;

static int m_starty = 0;

static float rubberband(float x, float coeff, float dim) {
    return (1.0 - (1.0 / ((x * coeff / dim) + 1.0))) * dim;
}

bool back_handle(ui_event_t *e, drawer_t* draw, cleanup_t* cleanup) {
    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_DOWN) {
        m_starty = e->touchevent.y;
        m_rubberband_prev_start = 0;
        m_rubberband_max = 0;
    }

    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_CONTACT) {
        if (e->touchevent.y - m_starty > 4) {
            m_rubberband = m_starty <= 30;
            if (m_rubberband) {
                int y = rubberband(e->touchevent.y, 0.35, 240);
                int start = MIN(m_rubberband_prev_start, y);
                int end = MAX(m_rubberband_prev_start + 40, y + 40);
                DO_DRAW(0, g_scrolloff + start - 20, 240, end-start, {
                    draw(g_scrolloff);
                    roundedrect(100, g_scrolloff + y - 20, 40, 40, 10 | (20 << 5) | (10 << 11));
                    text_drawicon(icon_arrow_down, 100, g_scrolloff + y - 19);
                })
                m_rubberband_prev_start = y;
                m_rubberband_max = MAX(m_rubberband_max, end);
            }
        }
    }

    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_UP) {
        if (m_rubberband) {
            if (rubberband(e->touchevent.y, 0.35, 240) < 20) {
                m_return = true;
                m_return_start = get_system_time() / 1000;
            } else {
                m_back = true;
                m_back_start = get_system_time() / 1000;
            }
            m_rubberband = false;
        }
    }

    if (m_return) {
        float t = ((get_system_time() / 1000) - m_return_start) / 1000.0;
        t *= 8.0;
        float off = spring_ex(m_rubberband_max, 0, t, 10, 1);
        int y = off;
        if (y <= 1) {
            y = 0;
            m_return = false;
        }
        DO_DRAW(0, g_scrolloff + y, 240, m_return_prev - y, draw(g_scrolloff));
        m_return_prev = y;
    }

    if (m_back) {
        float t = ((get_system_time() / 1000) - m_back_start) / 1000.0;
        t *= 8.0;
        float off = 240 - spring_ex(240, 0, t, 10, 1);
        int y = off;
        if (y >= 239) {
            if (cleanup) cleanup();
            y = 240;
            m_back = false;
            g_top = g_scrolloff;
            g_handler = watchface_handle;
        }
        DO_DRAW(0, g_scrolloff + m_back_prev, 240, y - m_back_prev, watchface_draw(g_scrolloff));

        for (int i = 0; i < 240; i++)
            g_target[240*0 + i] = __builtin_bswap16(31 | (63 << 5) | (31 << 11));
        for (int i = 0; i < 240; i++)
            g_target[240*1 + i] = __builtin_bswap16(15 | (31 << 5) | (15 << 11));
        plat_update(0, g_scrolloff + y, 240, 2);
        m_back_prev = y;
    }

    return m_back || m_rubberband || m_return;
}