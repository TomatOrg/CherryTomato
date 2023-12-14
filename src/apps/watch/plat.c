#include "plat.h"
#include "apps/entry.h"
#include "apps/watch/ui.h"
#include "event.h"
#include <util/divmod.h>

#include "util/log.h"
#include "util/except.h"
#include "drivers/st7789/st7789.h"
#include "drivers/ft6x06/ft6x06.h"
#include "task/time.h"
#include "timer.h"

typedef void handler_t(ui_event_t *e);
extern handler_t *g_handler;

void target_touch(bool*, int*, int*);
void ui_init();

// --------------------------------------------------------
// Global variables, extern'd in plat.h
// --------------------------------------------------------

uint16_t *g_target;
int g_scrolloff = 0;
int g_line = 0;
int g_nlines = 0;
int g_pitch = 0;

// --------------------------------------------------------

__attribute__((aligned(4)))
static uint16_t m_dmaA[240 * (NLINES + 1)];

__attribute__((aligned(4)))
static uint16_t m_dmaB[240 * (NLINES + 1)];

void target_set_vertical_scrolloff(uint16_t scrolloff);
void target_blit(uint16_t *buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

void plat_update(int x, int y, int w, int h) {
    //LOG_TRACE("platupdate %d %d %d %d, %d", x, y, w, h, g_scrolloff);
    ASSERT(w == g_pitch);

    int starty = floormod(y, 320);
    int endy = floormod(y + h, 320);
    if (endy < starty) {
        target_blit(g_target, x, starty, w, 320-starty);
        target_blit(g_target + (320 - starty) * g_pitch, x, 0, w, endy);
    } else {
        target_blit(g_target, x, starty, w, h);
    }

    if (g_target == m_dmaA) {
        g_target = m_dmaB;
    } else {
        g_target = m_dmaA;
    }
}


static void watch_loop();

static int m_oldx, m_oldy;
static bool m_oldpressed;
static uint64_t m_prevframe;

void watch_main() {
    g_target = m_dmaA;
    ui_init();
    g_scrolloff = 0;

    ui_event_t uie = {.type = UI_EVENT_REDRAW};
    g_handler(&uie);

    target_touch(&m_oldpressed, &m_oldx, &m_oldy);
    m_prevframe = (get_system_time() / 1000);

    start_event_loop(watch_loop, NULL);
}

static void watch_loop() {
    // TODO: check if the timer is actually done lol
    if (timers_count > 0) {
        ui_event_t uie = {.type = UI_EVENT_REDRAW};
        g_handler = alarmdone_handle;
        g_handler(&uie);
    }

    uint64_t starttime = (get_system_time() / 1000);

    bool up = false, down = false;

    int x, y;
    bool pressed = false;
    target_touch(&pressed, &x, &y);

    if (m_oldpressed && !pressed) up = true;
    else if (!m_oldpressed && pressed) down = true;

    ui_event_t uie = {.type = UI_EVENT_FRAME};
    if (down || pressed) {
        uie.type = UI_EVENT_TOUCH;
        uie.touchevent.x = x;
        uie.touchevent.y = y;
        uie.touchevent.dt = (starttime - m_prevframe) / 1000.0;
        uie.touchevent.dx = x - m_oldx;
        uie.touchevent.dy = y - m_oldy;
    }
    if (up) {
        uie.type = UI_EVENT_TOUCH;
        uie.touchevent.x = m_oldx;
        uie.touchevent.y = m_oldy;
        uie.touchevent.dt = (starttime - m_prevframe) / 1000.0;
        uie.touchevent.dx = 0;
        uie.touchevent.dy = 0;
    }
    if (down) uie.touchevent.action = TOUCHACTION_DOWN;
    else if (up) uie.touchevent.action = TOUCHACTION_UP;
    else if (pressed) uie.touchevent.action = TOUCHACTION_CONTACT;

    g_handler(&uie);
    target_set_vertical_scrolloff(floormod(g_scrolloff, 320));

    m_oldx = x;
    m_oldy = y;
    m_oldpressed = pressed;
    m_prevframe = starttime;
}