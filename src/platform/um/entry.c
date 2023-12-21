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

#ifndef __EMSCRIPTEN__
int perf_fd;
struct perf_event_attr pe;

static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}
void perf_init() {
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    // Don't count hypervisor events.
    pe.exclude_hv = 1;
    perf_fd = perf_event_open(&pe, 0, -1, -1, 0);
}
void perf_start() {
    ioctl(perf_fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(perf_fd, PERF_EVENT_IOC_ENABLE, 0);
}
long long perf_stop() {
    long long count;
    ioctl(perf_fd, PERF_EVENT_IOC_DISABLE, 0);
    read(perf_fd, &count, sizeof(long long));
    return count;
}
#else
long long perf_stop() {

}
#endif
void start_event_loop(loop_fn_t* fn, void* arg) {
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(fn, arg, -1, true);
#else
    perf_init();
    while (true) {
        uint64_t starttime = get_system_time();
        perf_start();
        fn(arg);
        uint64_t after = get_system_time();
        uint64_t delta = after - starttime;
        if (delta < 16 * 1000) { udelay(16 * 1000 - delta); }
    }
#endif
}

// TODO: implement this
uint64_t platform_timer_get_period(void) {
    return 0;
}
