#pragma once
#include "event.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum inertial_scroll_type {
    SCROLL_NONE,
    SCROLL_CONTACT,
    SCROLL_INERTIAL,
    SCROLL_TRANSITION,
    SCROLL_BOUNCEBACK,
} inertial_scroll_type_t;

typedef struct inertial_state {
    int starttouch;
    int startscroll;
    float velocity;
    uint64_t starttime;
    inertial_scroll_type_t type;
    int scroll;
    bool transition_isup;

    bool has_top_constraint;
    bool has_bottom_constraint;

    bool round;
} inertial_state_t;

int handle_inertial(inertial_state_t *d, ui_event_t *e);

float spring(float x0, float v0, float t);
float spring_ex(float x0, float v0, float t, float stiffness, float mass);
