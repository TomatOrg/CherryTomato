#include <stdint.h>
#include <util/log.h>
#include <apps/entry.h>
#include <target/target.h>
#include <util/printf.h>
#include <stdio.h>
#include <SDL2/SDL.h>

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

// TODO: implement this
uint64_t platform_timer_get_period(void) {
    return 0;
}

uint64_t get_system_time(void) {
    return SDL_GetTicks64() * 1000;
}