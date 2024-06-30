#include <stdint.h>
#include <util/log.h>
#include <apps/entry.h>
#include <target/target.h>
#include <util/printf.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "util/alloc.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

static void event_loop() {
    target_loop();
}

/**
 * The memory we will give the allocator, to emulate the worst
 * hardware (pinetime)
 */
static char m_memory[64 * 1024];

int main() {
    LOG_INFO("~~~ Cherry Tomato (Usermode simulator) ~~~");

    // add the range first thing
    mem_add_range(m_memory, sizeof(m_memory));

    // call the target
    target_entry();

    // setup everything
    cherry_tomato_entry();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(event_loop, NULL, -1, true);
#else
    for (;;) {
        event_loop();
    }
#endif

    return 0;
}

void putchar_(char c) {
    fputc(c, stdout);
}
