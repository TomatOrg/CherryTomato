#include <stdint.h>
#include <util/log.h>
#include <apps/entry.h>
#include <target/target.h>
#include <stdio.h>

int main() {
    LOG_INFO("~~~ Cherry Tomato (Usermode simulator) ~~~");

    // call the target
    target_entry();

    // setup the timer interrupt
    cherry_tomato_entry();
}

void putchar_(char c) {
    putchar(c);
}

// TODO: implement this
void delay(uint32_t us) {
}

// TODO: implement this
uint64_t platform_timer_get_period(void) {
    return 0;
}