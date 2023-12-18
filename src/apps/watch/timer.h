#pragma once
typedef struct ui_timer {
    int hour, minute;
    int type;
    int display_hour, display_minute;
} ui_timer_t;


#define TIMERS_MAX 16
extern ui_timer_t g_timers[TIMERS_MAX];
extern int g_timers_count;
void timer_add(ui_timer_t* t);