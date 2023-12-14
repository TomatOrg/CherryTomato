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

int g_fullmessage_toview;
static inertial_state_t m_fullmessage_inertial = {.has_top_constraint = 1, .has_bottom_constraint = 0};

void fullmessage_draw(int top) {
    message_t *msg = &g_messages[g_fullmessage_toview];
    thumbnail_draw(msg->image, 20, top + 10);
    text_drawline(font_roboto, msg->sender, 20 + 64 + 10, top + 10 + 14);
    text_drawline(font_roboto, msg->timestamp, 20 + 64 + 10, top + 10 + 14 + 22);
    text_draw_wrapped(font_roboto, &msg->full_info, msg->full, 20, top + 10 + 64 + 10 + 14);
}

void fullmessage_handle(ui_event_t *e) {
    int start, lines;
    handle_inertial(&m_fullmessage_inertial, e);
    ui_update_scrolloff(g_top + m_fullmessage_inertial.scroll, &start, &lines);
    DO_DRAW(0, start, 240, lines, {
        fullmessage_draw(g_top + 0);
        // limit this so it doesn't draw over the fullmessage view
        if (start + l < g_top) { messagelist_draw(g_top - 240 - g_startscroll_above); }
    });

    if (m_fullmessage_inertial.type == SCROLL_NONE && m_fullmessage_inertial.scroll == -240) {
        g_handler = messagelist_handle;
        m_fullmessage_inertial.scroll = 0;
        g_top -= 240 + g_startscroll_above;
    }
}