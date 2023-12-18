#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <util/printf.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "apps/watch/timer.h"
#include "event.h"
#include "font.h"
#include "physics.h"
#include "plat.h"
#include "text.h"
#include "thumbnail.h"
#include "ui.h"
#include "gesturerecognizer.h"
#include "util/log.h"
#include <util/divmod.h>
#include "task/time.h"

int g_fulltimer_toview = 0;

static inertial_state_t m_timerlist_inertial = {.has_top_constraint = 1, .has_bottom_constraint = 0};
static int m_hs_springstart = 0;
static bool m_hs_isgoingright = false;
static uint64_t m_hs_spring_starttime = 0;
static bool m_horiz_animation;
static bool m_horiz_drag_ended = false;

#define TOP_PAD 10
#define HEIGHT 40

static void timerlist_draw_x(int top, int x) {
    for (int i = 0; i < g_timers_count; i++) {
        char buffer[20];
        const char* type = g_timers[i].type == 0 ? "ALARM" : "TIMER";
        sprintf_(buffer, "%02d:%02d", g_timers[i].hour, g_timers[i].minute);
        text_drawline(font_bebas3, buffer, x, TOP_PAD + top + HEIGHT * i + HEIGHT);
        text_drawline(font_roboto, type, x + 70, TOP_PAD + top + HEIGHT * i + HEIGHT - 5);
    }
}

void timerlist_draw(int top) { timerlist_draw_x(top, 20); }

void timerlist_handle(ui_event_t *e) {
    bool transition = transition_do();
    if (transition) return;

    int start, lines;
    bool recognized, vertical, tap;
    int startx, starty;

    gesture_recognizer(e, &recognized, &vertical, &tap, &startx, &starty);
    int horiz_drag_idx = floordiv(m_timerlist_inertial.scroll + starty - TOP_PAD, HEIGHT); // TODO: check overflow

    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_DOWN && recognized) {
        m_horiz_drag_ended = false;
    }

    bool horizontal_drag = recognized && !vertical;
    bool is_vertical_movement = !horizontal_drag && !m_horiz_animation;

    // ------------------------
    // vertical scrolling
    // ------------------------
    if (is_vertical_movement) {
        handle_inertial(&m_timerlist_inertial, e);
        ui_update_scrolloff(g_top + m_timerlist_inertial.scroll, &start, &lines);

        DO_DRAW(0, start, 240, lines, {
            watchface_draw(g_top + -240);
            timerlist_draw(g_top + 0);
        });

        if (m_timerlist_inertial.type == SCROLL_NONE && m_timerlist_inertial.scroll == -240) {
            g_handler = watchface_handle;
            m_timerlist_inertial.scroll = 0;
            g_top -= 240;
        }
    }

    // ------------------------
    // tap
    // ------------------------
    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_UP && tap) {
        g_fulltimer_toview = floordiv(m_timerlist_inertial.scroll + e->touchevent.y - TOP_PAD, HEIGHT);
        if (g_fulltimer_toview >= 0 && g_fulltimer_toview < g_timers_count) {
            transition_start(timer_draw, timer_handle, m_timerlist_inertial.startscroll);
        }
    }

    // ------------------------
    // horizontal scrolling
    // ------------------------
    if (!m_horiz_drag_ended && !vertical && e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_UP) {
        m_horiz_animation = true;
        int off = MIN(240, MAX(0, e->touchevent.x - startx));
        m_hs_springstart = off;
        m_hs_spring_starttime = (get_system_time() / 1000);
        m_hs_isgoingright = off >= 40;
    }

    if ((m_horiz_animation || horizontal_drag) && !m_horiz_drag_ended) {
        int off;
        if (m_horiz_animation) {
            off = m_hs_springstart;
            float elapsed = ((get_system_time() / 1000) - m_hs_spring_starttime) / 1000.0;
            if (m_hs_isgoingright) {
                off = 240 - spring_ex(240 - off, 0, elapsed, 1500, 1);
            } else {
                off = spring_ex(off, 0, elapsed, 1500, 1);
            }
        } else {
            off = e->touchevent.x - startx;
        }

        // translate
        // TODO: this is inefficient code
        // I don't have to memmove it around, i can just clear currx-oldx cols
        // and and display the rest without translation
        off = MAX(0, MIN(off, 200));

        DO_DRAW(20, TOP_PAD + g_top + horiz_drag_idx * HEIGHT, 200, HEIGHT, {
            timerlist_draw_x(g_top + 0, 0);
            for (int i = 0; i < NLINES; i++) {
                memmove(g_target + i * g_pitch + off, g_target + i * g_pitch, (g_pitch - off) * 2);
                memset(g_target + i * g_pitch, 0, off * 2);
            }
        });

        // should we delete the timer?
        bool should_delete_msg = false;
        if (off >= 200 || off < 1) {
            should_delete_msg = off >= 200;
            m_horiz_animation = false;
            m_horiz_drag_ended = true;
        }

        if (should_delete_msg) {
            if (g_timers_count - horiz_drag_idx - 1 >= 0) {
                memmove(g_timers + horiz_drag_idx, g_timers + horiz_drag_idx + 1, (g_timers_count - horiz_drag_idx - 1) * sizeof(timer_t));
                g_timers_count--;
            }
            // draw the rest
            // TODO: i can do a scrolloff trick here, but it's fast enough for now
            int start = TOP_PAD + horiz_drag_idx * HEIGHT;
            int numlines = 240 - (TOP_PAD + horiz_drag_idx * HEIGHT - m_timerlist_inertial.scroll);
            DO_DRAW(20, g_top + start, 200, numlines, timerlist_draw_x(g_top, 0));
        }
    }
}
