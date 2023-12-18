#pragma once
#include "event.h"
#include <stdbool.h>

#define DO_DRAW(startx, starty, width, height, draw) \
    {   \
        g_pitch = width; \
        for (int l = 0; l < height; l += NLINES) { \
            g_line = starty + l; \
            g_nlines = MIN(height - l, NLINES); \
            memset(g_target, 0, 240 * 2 * NLINES); \
            draw; \
            plat_update(startx, g_line, g_pitch, g_nlines); \
        } \
    }

typedef void handler_t(ui_event_t *e);
typedef void drawer_t(int top);
typedef void cleanup_t();

void ui_update_scrolloff(int newscrolloff, int *start, int *lines);
extern handler_t *g_handler;
extern int g_top;
extern int g_startscroll_above;

void watchface_draw(int top);
void messagelist_draw(int top);
void timer_draw(int top);
void fullmessage_draw(int top);
void applist_draw(int top);
void calculator_draw(int top);
void timerlist_draw(int top);

void alarmdone_handle(ui_event_t *e);
void watchface_handle(ui_event_t *e);
void messagelist_handle(ui_event_t *e);
void timer_handle(ui_event_t *e);
void fullmessage_handle(ui_event_t *e);
void applist_handle(ui_event_t *e);
void calculator_handle(ui_event_t *e);
void timerlist_handle(ui_event_t *e);

void transition_start(drawer_t* d, handler_t* h, int newscrolloff);
bool transition_do(cleanup_t* cleanup);