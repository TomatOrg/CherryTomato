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
#include "back.h"
#include "timer.h"
#include <util/divmod.h>
#include "roundedrect.h"
#include "util/log.h"
#include <util/except.h>


void test_draw(int top) {

}

void draw(int x, int y, int r) {
    int startx = MAX(x - 20, 10);
    int endx = MIN(x + 20, 230);
    g_pitch = endx - startx;
    if (g_pitch <= 0) return;
    int start = MAX(y - 20, g_scrolloff + 10);
    int end = MIN(y + 20, g_scrolloff + 230);
    int lines = end - start;
    for (int l = 0; l < lines; l += NLINES) {
        g_line = start + l;
        g_nlines = MIN(lines - l, NLINES);
        memset(g_target, 0, 240 * 2 * NLINES);
        if (x - 20 >= 0 && x + 20 < 240) roundedrect(20 - r, y - r, r*2, r*2, 0xFFFF);
        for (int i = 0; i < g_nlines; i++) {
            for (int j = 0; j < g_pitch; j++) {
                int dx = (j + startx) - 120, dy = (i + g_line - g_scrolloff) - 120;
                dx *= dx; dy *= dy;
                float a = sqrtf(dx + dy) / 120;
                if (a > 1.0) a = 1.0;
                a = 1.0 - a;

                int off = i * g_pitch + j;
                uint16_t v = __builtin_bswap16(g_target[off]);
                uint8_t r = (v & 31), g = ((v >> 5) & 63), b = ((v >> 11) & 31);
                uint8_t newr = r * a;
                uint8_t newg = g * a;
                uint8_t newb = b * a;
                g_target[off] = __builtin_bswap16((newr << 0) | (newg << 5) | (newb << 11));
            }
        }
        plat_update(startx, g_line, g_pitch, g_nlines);
    }
}

static int m_startx = 0;
static int m_starty = 0;

void test_handle(ui_event_t *e) {
    bool isback = back_handle(e);
    if (isback) return;
    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_DOWN) {
        m_startx = e->touchevent.x;
        m_starty = e->touchevent.y;
    }
    for (int i = -100; i < 100; i++) {
        for (int j = -100; j < 100; j++) {
            int x = 40 * i + (e->touchevent.x - m_startx);
            int y = 40 * j + (e->touchevent.y - m_starty);
            if (j % 2) { x += 40 / 2; }
            float toCenter = sqrtf(
                pow(y - 120, 2) +
                pow(x - 120, 2)
            );
            float r = MIN(15, (240 * (0.20 - toCenter / 240 / 3)) / 2);
            draw(x, g_top + y, r);
        }
    }
}
