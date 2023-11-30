#pragma once
typedef struct ui_timer {
    int hour, minute;
    int type;
    int display_hour, display_minute;
} ui_timer_t;


#define TIMERS_MAX 16
extern ui_timer_t timers[TIMERS_MAX];
extern int timers_count;