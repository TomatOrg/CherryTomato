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

/*void draw(int x, int y, int r) {
    int startx = MAX(x - 20, 10);
    int endx = MIN(x + 20, 230);
    g_pitch = endx - startx;
    if (g_pitch <= 0) return;
    int start = MAX(y - 20, g_scrolloff + 10);
    int end = MIN(y + 20, g_scrolloff + 230);
    int lines = end - start;
    for (int l = 0; l < lines; l++) {
        g_line = start + l;
        g_nlines = 1;
        memset(g_target, 0, 240 * 2);
        int off = startx - (x-20);
        if (g_line >= y - r && g_line < y + r) for (int i = 0; i < r*2-off; i++) { g_target[i] = 0xFFFF; }
        plat_update(startx, g_line, g_pitch, g_nlines);
    }
}*/


int current_field = 0;

#include "../icons.h"

void draw(int x, int y, int r) {
    int startx = MAX(x - 24, 10);
    int endx = MIN(x + 24, 230);
    g_pitch = endx - startx;
    if (g_pitch <= 0) return;
    int start = MAX(y - 24, g_scrolloff + 10);
    int end = MIN(y + 24, g_scrolloff + 230);
    int lines = end - start;
    int recip = r != 0 ? (256 * 36 / (r * 2)) : 0;
    for (int l = 0; l < lines; l++) {
        g_line = start + l;
        if ((g_line & 1) != current_field) continue;
        g_nlines = 1;
        memset(g_target, 0, 240 * 2);
        int off = startx - (x-24);
        if (g_line >= y - r && g_line < y + r)
            for (int i = off; i < r*2; i++) {
                int ix = (i * recip) >> 8;
                int iy = ((g_line - (y - r)) * recip) >> 8;
                uint16_t col = ((uint16_t*)_music)[iy * 36 + ix];
                g_target[i-off] = __builtin_bswap16(col);
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

    int xoff = e->touchevent.x - m_startx;
    int yoff = e->touchevent.y - m_starty;
    for (int i = -yoff / 48 - 1; i < (-yoff + 240) / 48 + 1; i++) {
        for (int j = -xoff / 48 - 1; j < (-xoff + 240) / 48 + 1; j++) {
            int y = 48 * i + yoff;
            int x = 48 * j + xoff;
            if (i % 2) { x += 48 / 2; }
            float toCenter = sqrtf((y-120)*(y-120) + (x-120)*(x-120));
            float r = MIN(18, (240 * (0.30 - toCenter / 240 / 2)) / 2);
            draw(x, g_top + y, r);
        }
    }
    current_field = 1 - current_field;
}
