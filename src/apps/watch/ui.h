#pragma once
#include "event.h"

typedef void handler_t(ui_event_t *e);
typedef void drawer_t(int top);

void ui_update_scrolloff(int newscrolloff, int *start, int *lines);
extern handler_t *g_handler;
extern int g_top;
extern int g_startscroll_above;

void watchface_draw(int top);
void messagelist_draw(int top);
void timer_draw(int top);
void fullmessage_draw(int top);
void applist_draw(int top);

void alarmdone_handle(ui_event_t *e);
void watchface_handle(ui_event_t *e);
void messagelist_handle(ui_event_t *e);
void timer_handle(ui_event_t *e);
void fullmessage_handle(ui_event_t *e);
void applist_handle(ui_event_t *e);