#pragma once

typedef enum ui_event_touchaction { TOUCHACTION_DOWN = 0, TOUCHACTION_UP = 1, TOUCHACTION_CONTACT = 2 } ui_event_touchaction_t;

typedef struct ui_event {
    enum { UI_EVENT_REDRAW, UI_EVENT_TOUCH, UI_EVENT_FRAME } type;
    union {
        // TODO: the touch controller supports 10 points
        // and for each point it also knows the finger, pressure and area
        struct {
            int x, y;
            float dx, dy, dt; // TODO: make it an integer number of milliseconds?
            ui_event_touchaction_t action;
        } touchevent;
    };
} ui_event_t;
