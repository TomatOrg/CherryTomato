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

typedef enum op {
    OP_NONE = 0,
    OP_DIV,
    OP_MUL,
    OP_SUB,
    OP_ADD
} op_t;

static int m_total = 0;
static op_t m_operation;
static uint64_t m_num;
static bool m_start_new = true;

void drawbutton(const char* str, int x, int y, int top) {
    int size = text_getlinesize(font_roboto, str);
    text_drawline(font_roboto, str, x - size / 2, top + y + 18 / 2);
}

static void draw_text(int start, int top) {
    char textbuf[64];
    if (m_total != 0) {
        sprintf_(textbuf, "%d", m_total);
        text_drawline(font_roboto, textbuf, 32 - start, top + 40);
    }

    sprintf_(textbuf, "%d", m_num);
    int size = text_getlinesize(font_bebas4, textbuf);
    text_drawline(font_bebas4, textbuf, 200 - size - start, top + 60);
}

void calculator_draw(int top) {
    draw_text(0, top);

    drawbutton("7", 40, 90, top);
    drawbutton("8", 90, 90, top);
    drawbutton("9", 140, 90, top);
    drawbutton("/", 190, 90, top);

    drawbutton("4", 40, 130, top);
    drawbutton("5", 90, 130, top);
    drawbutton("6", 140, 130, top);
    drawbutton("*", 190, 130, top);

    drawbutton("1", 40, 170, top);
    drawbutton("2", 90, 170, top);
    drawbutton("3", 140, 170, top);
    drawbutton("-", 190, 170, top);

    drawbutton("0", 40, 210, top);
    drawbutton(".", 90, 210, top);
    drawbutton("=", 140, 210, top);
    drawbutton("+", 190, 210, top);
}

static uint64_t apply_op(uint64_t lhs, uint64_t rhs, op_t op) {
    switch (op) {
    case OP_NONE: return lhs;
    case OP_DIV: return lhs / rhs;
    case OP_MUL: return lhs * rhs;
    case OP_SUB: return lhs - rhs;
    case OP_ADD: return lhs + rhs;
    }
    return 0;
}

static bool m_back = false;
static uint64_t m_back_start = 0;
static int m_back_prev = 0;

static int m_rubberband_prev_start = 0;
static bool m_rubberband = false;

static float rubberband(float x, float coeff, float dim) {
    return (1.0 - (1.0 / ((x * coeff / dim) + 1.0))) * dim;
}

void calculator_handle(ui_event_t *e) {
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

        int bx = (e->touchevent.x - 40 + 25) / 50;
        int by = (e->touchevent.y - 90 + 20) / 40;

        if (bx >= 0 && bx < 3 && by >= 0 && by < 3) {
            if (m_start_new) {
                m_num = 0;
                m_start_new = false;
            }
            int number = (2 - by) * 3 + 1 + bx;
            m_num = m_num * 10 + number;
        }

        if (by == 3 && bx == 0) {
            if (m_start_new) {
                m_num = 0;
                m_start_new = false;
            }
            m_num *= 10;
        }

        if (bx == 3 && by >= 0 && by < 4) {
            m_operation = by + 1;
            if (m_start_new) {
                m_total = m_num;
                m_start_new = false;
            } else {
                m_total = apply_op(m_total, m_num, m_operation);
            }
            m_num = 0;
        }

        if (bx == 2 && by == 3) {
            m_num = apply_op(m_total, m_num, m_operation);
            m_total = 0;
            m_start_new = true;
        }

        int lines = 40;
        int start = g_top + 20;
        g_pitch = 200-32;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = start + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            draw_text(32, g_top + 0);
            plat_update(32, g_line, 200-32, g_nlines);
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
}
