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


static bool m_animation = false;
static uint64_t m_animation_start = false;
static int m_animation_oldy = 0;
static drawer_t* m_drawing_function = NULL;
static handler_t* m_handling_function = NULL;

void transition_start(drawer_t* d, handler_t* h, int newscrolloff) {
    m_animation = true;
    m_animation_oldy = 0;
    m_animation_start = get_system_time() / 1000;
    m_drawing_function = d;
    m_handling_function = h;
    g_startscroll_above = newscrolloff;
}

bool transition_do() {
    bool animation_done = false;
    if (m_animation) {
        float t = ((get_system_time() / 1000) - m_animation_start) / 1000.0;
        t *= 8.0;
        float half = 120 - spring_ex(120, 0, t, 10, 1);
        animation_done = half >= 119;
        if (animation_done) {
            half = 120;
            m_animation = false;
            g_top = g_scrolloff;
            g_handler = m_handling_function;
        }
        int y = (int)half;
        int height = y - m_animation_oldy;

        DO_DRAW(0, g_scrolloff + 120 - y, 240, height, m_drawing_function(g_scrolloff));
        DO_DRAW(0, g_scrolloff + 120 + m_animation_oldy, 240, height, m_drawing_function(g_scrolloff));
        m_animation_oldy = y;
    }
    return m_animation || animation_done;
}