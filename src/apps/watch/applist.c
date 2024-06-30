#include <stdbool.h>
#include <stdint.h>
#include <util/printf.h>
#include <string.h>
#include <unistd.h>

#include "event.h"
#include "font.h"
#include "messagestore.h"
#include "physics.h"
#include "text.h"
#include "gesturerecognizer.h"
#include "roundedrect.h"
#include "ui.h"
#include <util/divmod.h>

inertial_state_t m_applist_inertial = {.has_top_constraint = 1, .has_bottom_constraint = 1};

static const uint16_t gray = 7 | (14 << 5) | (7 << 11);
static const uint16_t green = 10 | (45 << 5) | (2 << 11);

uint8_t *icon_array[2][3] = {
    { icon_timernew, icon_timeredit, icon_calculator },
    { icon_moon, icon_bell_crossed, icon_lightbulb }
};

void applist_draw(int top) {
    roundedrect(12, top + 20, 64, 64, gray);
    text_drawicon(icon_timernew, 12, top + 20);
    roundedrect(87, top + 20, 64, 64, gray);
    text_drawicon(icon_timeredit, 87, top + 20);
    roundedrect(162, top + 20, 64, 64, gray);
    text_drawicon(icon_calculator, 162, top + 20);

    roundedrect(12, top + 95, 64, 64, gray);
    text_drawicon(icon_moon, 12, top + 95);
    roundedrect(87, top + 95, 64, 64, gray);
    text_drawicon(icon_bell_crossed, 87, top + 95);
    roundedrect(162, top + 95, 64, 64, gray);
    text_drawicon(icon_lightbulb, 162, top + 95);

}


static bool m_state[3] = {};

void applist_handle(ui_event_t *e) {
    bool transition = transition_do(NULL);
    if (transition) return;

    int start, lines;

    if (e->type == UI_EVENT_REDRAW) {
        start = g_top;
        lines = 240;
    }

    bool recognized, vertical, tap;
    int startx, starty;
    gesture_recognizer(e, &recognized, &vertical, &tap, &startx, &starty);

    bool starting_vertical = (recognized && vertical);
    bool in_scroll = m_applist_inertial.type != SCROLL_NONE;
    bool is_vertical_movement = (starting_vertical || in_scroll);

    if (is_vertical_movement) {
        handle_inertial(&m_applist_inertial, e);
        ui_update_scrolloff(g_top + m_applist_inertial.scroll, &start, &lines);

        DO_DRAW(0, start, 240, lines, {
            applist_draw(g_top + 0);
            watchface_draw(g_top + 240);
        });

        if (m_applist_inertial.type == SCROLL_NONE && m_applist_inertial.scroll == -240) {
            g_handler = applist_handle;
            m_applist_inertial.scroll = 0;
            g_top -= 240;
        }

        if (m_applist_inertial.type == SCROLL_NONE && m_applist_inertial.scroll == 240) {
            g_handler = watchface_handle;
            m_applist_inertial.scroll = 0;
            g_top += 240;
        }
    }


    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_UP && tap) {
        int bx = (e->touchevent.x - 12) / 75;
        int by = (e->touchevent.y - 20) / 75;
        if (bx == 0 && by == 0) transition_start(timer_draw, timer_handle, 0);
        if (bx == 1 && by == 0) transition_start(timerlist_draw, timerlist_handle, 0);
        if (bx == 2 && by == 0) transition_start(calculator_draw, calculator_handle, 0);

        if (by==1) {
            m_state[bx] = !m_state[bx];
            // TODO: do an animation to draw in a square
            DO_DRAW(12 + bx * 75, g_top + 20 + by * 75, 64, 64, {
                roundedrect(0, g_top + 20 + by * 75, 64, 64, m_state[bx] ? green : gray);
                if (icon_array[by][bx]) text_drawicon(icon_array[by][bx], 0, g_top + 20 + by * 75);
            });
        }
    }
}
