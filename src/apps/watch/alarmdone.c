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
#include "timer.h"
#include <util/divmod.h>
#include "roundedrect.h"
#include "util/log.h"
#include <util/except.h>

static ui_timer_t m_timer;

void alarmdone_draw(int top) {
    char string[16];
    char *mode;
    char *activebutton;
    char *secondbutton;
    if (m_timer.type == 0) {
        sprintf_(string, "%02d:%02d", m_timer.display_hour, m_timer.display_minute);
        mode = "ALARM";
        activebutton = "Snooze";
        secondbutton = "Stop";
    } else if (m_timer.type == 1) {
        sprintf_(string, "+%02d:%02d:00", m_timer.display_hour, m_timer.display_minute);
        mode = "TIMER";
        activebutton = "Stop";
        secondbutton = "Repeat";
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
    text_drawline(font_roboto, secondbutton, 20 + (90 - size) / 2, top + 160 + (60 + 18/2) / 2);

    roundedrect(130, top + 160, 90, 60, (5) | (20 << 5) | (24 << 11));
    size = text_getlinesize(font_roboto, activebutton);
    text_drawline(font_roboto, activebutton, 130 + (90 - size) / 2, top + 160 + (60 + 18/2) / 2);
}

void alarmdone_handle(ui_event_t *e) {
    int start = 0, lines = 0;

    if (e->type == UI_EVENT_REDRAW) {
        start = g_top;
        lines = 240;
        ASSERT(timers_count > 0);
        m_timer = timers[0];
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
