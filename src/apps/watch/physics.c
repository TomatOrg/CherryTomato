#include "physics.h"
#include "event.h"
#include <util/divmod.h>
#include <math.h>
#include <stdbool.h>
#include <target/target.h>

int handle_inertial(inertial_state_t *d, ui_event_t *e) {
    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_DOWN) {
        d->starttouch = e->touchevent.y;
        d->startscroll = d->scroll;
        d->type = SCROLL_CONTACT;
        d->velocity = 0;
    }
    // rubberband
    if (e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_CONTACT) {
            d->scroll = d->startscroll - (e->touchevent.y - d->starttouch);
            // do a moving average on unevenly spaced data
            // i am not entirely sure the formula is correct or stable
            float velocity = -e->touchevent.dy / e->touchevent.dt;
            float span = 0.01;
            float alpha = 1 - expf(-e->touchevent.dt / span);
            d->velocity = (1 - alpha) * d->velocity + alpha * velocity;
    }

    if (d->type == SCROLL_CONTACT && e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_UP) {
        if (d->round || fabsf(d->velocity) > 1) {
            d->type = SCROLL_INERTIAL;
            d->startscroll = d->scroll;
            d->starttime = target_get_current_tick();
        } else {
            d->type = SCROLL_NONE;
        }
    }

    // TODO. have a separate flag controlling spring transitions
    if (!d->round && d->type != SCROLL_BOUNCEBACK) {
        if (d->type != SCROLL_TRANSITION && ((e->type == UI_EVENT_TOUCH && e->touchevent.action == TOUCHACTION_UP) ||
                                      d->type == SCROLL_INERTIAL)) {
            if (d->has_top_constraint && d->scroll < 0) {
                d->starttime = target_get_current_tick();
                d->startscroll = d->scroll + 240;
                d->transition_isup = true;
                d->type = SCROLL_TRANSITION;
            }
            if (d->has_bottom_constraint && d->scroll > 0) {
                d->starttime = target_get_current_tick();
                d->startscroll = 240 - d->scroll;
                d->transition_isup = false;
                d->type = SCROLL_TRANSITION;
            }
        }
        if (d->type == SCROLL_TRANSITION) {
            float t = (target_get_current_tick() - d->starttime) / 1000.0;
            int s = spring(d->startscroll, 0, t);
            if (d->transition_isup) {
                d->scroll = s - 240;
                if (d->scroll < -239) {
                    d->scroll = -240;
                    d->type = SCROLL_NONE;
                }
            } else {
                d->scroll = 240 - s;
                if (d->scroll > 239) {
                    d->scroll = 240;
                    d->type = SCROLL_NONE;
                }
            }
        }
    }

    if (d->type == SCROLL_INERTIAL) {
        float t = (target_get_current_tick() - d->starttime) / 1000.0;

        if (d->round) t *= 5.0;

        float lim1 = -1 / (1000 * logf(0.994));
        float target = d->startscroll + lim1 * d->velocity;

        if (d->round) {
            target = (target / 40.0);
            // if i am going downwards, round to the number above
            if (d->velocity > 0) target = ceilf(target);
            else target = floorf(target);
            target *= 40;
        }

        if (d->round && fabsf(d->velocity) <= 150) d->velocity = copysignf(150, d->velocity);
        float lim2 = (target - d->startscroll) / d->velocity;
        float scro = lim2 * (1 - expf(-t / lim2));
        float off = d->startscroll + scro * d->velocity;
        d->scroll = off;

        if (d->has_top_constraint && d->scroll < 0) {
            d->type = SCROLL_BOUNCEBACK;
            d->starttime = target_get_current_tick();
            d->startscroll = d->scroll;
            d->velocity = d->velocity * powf(0.995, 1000 * t);
        } else if (fabsf(off - target) < 0.75) {
            d->scroll = target;
            d->type = SCROLL_NONE;
        }
    }

    if (d->type == SCROLL_BOUNCEBACK) {
        float t = (target_get_current_tick() - d->starttime) / 1000.0;
        float off = spring_ex(d->startscroll, d->velocity, t, 500, 1);
        d->scroll = off;
        if (fabsf(off) < 0.75) {
            d->scroll = 0;
            d->type = SCROLL_NONE;
        }
    }

    return d->scroll;
}

float spring_ex(float x0, float v0, float t, float stiffness, float mass) {
    float damping_ratio = 1;
    float damping = 2 * damping_ratio * sqrtf(stiffness * mass);
    float beta = damping / (2 * mass);
    float C1 = x0;
    float C2 = v0 + beta * x0;
    float x = (C1 + C2 * t) * expf(-beta * t);
    return x;
}

float spring(float x0, float v0, float t) {
    return spring_ex(x0, v0, t, 1000, 1);
}