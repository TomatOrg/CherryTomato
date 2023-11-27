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
#include "text.h"
#include "thumbnail.h"
#include "ui.h"
#include <util/divmod.h>
#include "roundedrect.h"

void alarmdone_draw(int top) {
    int size, startx;

    size = text_getlinesize(font_bebas3, "TIMER");
    startx = (240 - size) / 2;
    text_drawline(font_bebas3, "TIMER", startx, 130);


    size = text_getlinesize(font_bebas2, "5:00");
    startx = (240 - size) / 2;
    text_drawline(font_bebas2, "5:00", startx, 90);

    roundedrect(20, 160, 90, 60, (15) | (30 << 5) | (15 << 11));
    roundedrect(130, 160, 90, 60, (5) | (50 << 5) | (10 << 11));
}

void alarmdone_handle(ui_event_t *e) {
    int start = 0, lines = 0;

    if (e->type == UI_EVENT_REDRAW) {
        start = g_top;
        lines = 240;
    }

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
