#include "entry.h"
#include "util/log.h"
#include "task/timer.h"

static void my_function() {
    LOG_INFO("Hello world!");
}

static timer_t m_timer = {
    .event = {
        .notify_tpl = TPL_NOTIFY,
        .notify_function = my_function
    }
};

void cherry_tomato_entry() {
    LOG_INFO("Starting up watch");

    timer_set_interval(&m_timer, 1000);
}
