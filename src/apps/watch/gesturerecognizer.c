#include "gesturerecognizer.h"
#include <util/divmod.h>

static int m_startx, m_starty;
static bool m_is_gesture_recognized = false;
static bool m_is_gesture_vertical;

void gesture_recognizer(ui_event_t *e, bool *recognized, bool *vertical, bool *tap, int *startx, int *starty) {
    bool is_tap = false;

    // i have no idea how it's supposed to work really
    // but i am going off one sentence in apple's docs
    // A pan gesture recognizer enters the UIGestureRecognizer.State.began state
    // *as soon as the required amount of initial movement is achieved*
    // I have no idea how it's calculated

    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_DOWN) {
        m_startx = e->touchevent.x;
        m_starty = e->touchevent.y;
        m_is_gesture_recognized = false;
    }

    if (!m_is_gesture_recognized && e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_CONTACT) {
        int dx = e->touchevent.x - m_startx;
        int dy = e->touchevent.y - m_starty;
        if (dx*dx + dy*dy >= 4*4) {
            m_is_gesture_recognized = true;
            m_is_gesture_vertical = ABS(dy) > ABS(dx);
            // inject fake start gesture in the vertical scroll statemachine.
            // TODO: there is a better way surely
            e->touchevent.action = TOUCHACTION_DOWN;
        }
    }

    // same as before, anything less than the margin is considered "in the same place"
    // so interpret it as a tap
    if (!m_is_gesture_recognized && e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_UP) {
        int dx = e->touchevent.x - m_startx;
        int dy = e->touchevent.y - m_starty;
        if (dx*dx + dy*dy < 4*4) { is_tap = true; }
    }

    *tap = is_tap;
    *recognized = m_is_gesture_recognized;
    *vertical = m_is_gesture_vertical;
    *startx = m_startx;
    *starty = m_starty;

    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_UP) {
        m_is_gesture_recognized = false;
    }
}