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
#include "gesturerecognizer.h"
#include "util/log.h"
#include <util/divmod.h>
#include "task/time.h"


extern int g_fullmessage_toview;

static inertial_state_t m_messagelist_inertial = {.has_top_constraint = 1, .has_bottom_constraint = 0};
static int m_hs_springstart = 0;
static bool m_hs_isgoingright = false;
static uint64_t m_hs_spring_starttime = 0;
static bool m_messagelist_anim = false;
static uint64_t m_messagelist_start = 0;
static int m_messagelist_anim_oldy = 0;
static bool m_horiz_animation;

static void messagelist_draw_x(int top, int x) {
    for (int i = 0; i < g_messages_num; i++) {
        thumbnail_draw(g_messages[i].image, x, 20 + top + 80 * i);
        text_draw_wrapped(font_roboto, &g_messages[i].cropped_info, g_messages[i].cropped, x + 64 + 10,
                          20 + top + 80 * i + 14);
    }
}

void messagelist_draw(int top) { messagelist_draw_x(top, 20); }

void messagelist_handle(ui_event_t *e) {
    int start, lines;
    bool recognized, vertical, tap;
    int startx, starty;

    gesture_recognizer(e, &recognized, &vertical, &tap, &startx, &starty);
    int horiz_drag_idx = floordiv(m_messagelist_inertial.scroll + starty - 20, 80); // TODO: check overflow

    bool horizontal_drag = !m_messagelist_anim && recognized && !vertical;
    bool is_vertical_movement = !m_messagelist_anim && !horizontal_drag && !m_horiz_animation;

    // ------------------------
    // vertical scrolling
    // ------------------------
    if (is_vertical_movement) {
        handle_inertial(&m_messagelist_inertial, e);
        ui_update_scrolloff(g_top + m_messagelist_inertial.scroll, &start, &lines);
        g_pitch = 240;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = start + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            watchface_draw(g_top + -240);
            messagelist_draw(g_top + 0);
            plat_update(0, g_line, 240, g_nlines);
        }

        if (m_messagelist_inertial.type == SCROLL_NONE && m_messagelist_inertial.scroll == -240) {
            g_handler = watchface_handle;
            m_messagelist_inertial.scroll = 0;
            g_top -= 240;
        }
    }

    // ------------------------
    // tap
    // ------------------------
    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_UP && tap) {
        if (!m_messagelist_anim) {
            g_fullmessage_toview = floordiv(m_messagelist_inertial.scroll + e->touchevent.y - 20, 80);
            if (g_fullmessage_toview >= 0 && g_fullmessage_toview < g_messages_num) {
                m_messagelist_anim = true;
                m_messagelist_start = (get_system_time() / 1000);
            }
        }
    }

    // ------------------------
    // horizontal scrolling
    // ------------------------
    if (!m_messagelist_anim && !vertical && e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_UP) {
        m_horiz_animation = true;
        int off = MIN(240, MAX(0, e->touchevent.x - startx));
        m_hs_springstart = off;
        m_hs_spring_starttime = (get_system_time() / 1000);
        m_hs_isgoingright = off >= 40;
        m_horiz_animation = true;
    }

    if ((m_horiz_animation || horizontal_drag) && !m_messagelist_anim) {
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
        g_pitch = 200;
        for (int l = 0; l < 64; l += NLINES) {
            g_line = 20 + g_top + l + horiz_drag_idx * 80;
            g_nlines = MIN(64 - l, NLINES);
            memset(g_target, 0x00, 240 * 2 * NLINES);
            messagelist_draw_x(g_top + 0, 0);
            for (int i = 0; i < NLINES; i++) {
                memmove(g_target + i * g_pitch + off, g_target + i * g_pitch, (g_pitch - off) * 2);
                memset(g_target + i * g_pitch, 0, off * 2);
            }
            plat_update(20, g_line, 200, g_nlines);
        }

        // should we delete the message?
        bool should_delete_msg = false;
        if (off >= 200 || off < 1) {
            should_delete_msg = off >= 200;
            m_horiz_animation = false;
        }

        if (should_delete_msg) {
            if (g_messages_num - horiz_drag_idx - 1 >= 0) {
                memmove(g_messages + horiz_drag_idx, g_messages + horiz_drag_idx + 1, (g_messages_num - horiz_drag_idx - 1) * sizeof(message_t));
                g_messages_num--;
            }
            // draw the rest
            // TODO: i can do a scrolloff trick here, but it's fast enough for now
            int start = 20 + horiz_drag_idx * 80;
            int numlines = 240 - (20 + horiz_drag_idx * 80 - m_messagelist_inertial.scroll);
            g_pitch = 200;
            for (int l = 0; l < numlines; l += NLINES) {
                g_line = g_top + l + start;
                g_nlines = MIN(numlines - l, NLINES);
                memset(g_target, 0x00, 240 * 2 * NLINES);
                messagelist_draw_x(g_top + 0, 0);
                plat_update(20, g_line, 200, g_nlines);
            }
        }
    }

    // ------------------------
    // showing fullmessage on tap
    // ------------------------
    if (m_messagelist_anim) {
        float t = ((get_system_time() / 1000) - m_messagelist_start) / 1000.0;
        t *= 8.0;
        float half = 120 - spring_ex(120, 0, t, 10, 1);
        if (half >= 119) {
            half = 120;
            g_frame_requested = false;
            m_messagelist_anim = false;
            g_top = g_scrolloff;
            g_startscroll_above = m_messagelist_inertial.startscroll;
            g_handler = fullmessage_handle;
        } else g_frame_requested = true;

        int y = (int)half;
        int lines = y - m_messagelist_anim_oldy;
        g_pitch = 240;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = g_scrolloff + 120 - y + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            fullmessage_draw(g_scrolloff);
            plat_update(0, g_line, 240, g_nlines);
        }
        g_pitch = 240;
        for (int l = 0; l < lines; l += NLINES) {
            g_line = g_scrolloff + 120 + m_messagelist_anim_oldy + l;
            g_nlines = MIN(lines - l, NLINES);
            memset(g_target, 0, 240 * 2 * NLINES);
            fullmessage_draw(g_scrolloff);
            plat_update(0, g_line, 240, g_nlines);
        }
        m_messagelist_anim_oldy = y;
    }
}
