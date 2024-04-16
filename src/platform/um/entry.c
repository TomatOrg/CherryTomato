#include <stdint.h>
#include <util/log.h>
#include <apps/entry.h>
#include <target/target.h>
#include <util/printf.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#else
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#endif

static void event_loop() {
    target_loop();
}

int main() {
    LOG_INFO("~~~ Cherry Tomato (Usermode simulator) ~~~");

    // call the target
    target_entry();

    // setup everything
    cherry_tomato_entry();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(event_loop, NULL, -1, true);
#else
    while (true) {
        event_loop();
    }
#endif

    return 0;
}

void putchar_(char c) {
    putchar(c);
}

uint64_t get_system_time(void) {
    return SDL_GetTicks64() * 1000;
}

uint64_t platform_timer_get_period(void) {
    return 0;
}
