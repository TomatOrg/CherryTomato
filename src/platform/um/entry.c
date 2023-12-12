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
#endif

int main() {
    LOG_INFO("~~~ Cherry Tomato (Usermode simulator) ~~~");

    // call the target
    target_entry();

    // setup the timer interrupt
    cherry_tomato_entry();

    return 0;
}

void putchar_(char c) {
    putchar(c);
}

void udelay(uint32_t us) {
    SDL_Delay(us / 1000);
}

uint64_t get_system_time(void) {
    return SDL_GetTicks64() * 1000;
}

void start_event_loop(loop_fn_t* fn, void* arg) {
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(fn, arg, -1, true);
#else
    while (true) {
        uint64_t starttime = get_system_time();
        fn(arg);
        uint64_t after = get_system_time();
        uint64_t delta = after - starttime;
        if (delta < 20 * 1000) { udelay(20 * 1000 - delta); }
    }
#endif
}

// TODO: implement this
uint64_t platform_timer_get_period(void) {
    return 0;
}
