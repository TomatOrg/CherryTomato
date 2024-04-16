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
    for (;;) {
        event_loop();
    }
#endif

    return 0;
}

void putchar_(char c) {
    fputc(c, stdout);
}
