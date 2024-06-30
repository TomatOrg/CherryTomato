#include "perf.h"

#include <linux/perf_event.h>
#include <syscall.h>

#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

static int m_perf_fd;
static struct perf_event_attr m_perf_event;

static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

void perf_init() {
    memset(&m_perf_event, 0, sizeof(struct perf_event_attr));
    m_perf_event.type = PERF_TYPE_HARDWARE;
    m_perf_event.size = sizeof(struct perf_event_attr);
    m_perf_event.config = PERF_COUNT_HW_INSTRUCTIONS;
    m_perf_event.disabled = 1;
    m_perf_event.exclude_kernel = 1;
    // Don't count hypervisor events.
    m_perf_event.exclude_hv = 1;
    m_perf_fd = perf_event_open(&m_perf_event, 0, -1, -1, 0);
}

void perf_start() {
    ioctl(m_perf_fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(m_perf_fd, PERF_EVENT_IOC_ENABLE, 0);
}

long long perf_stop() {
    long long count;
    ioctl(m_perf_fd, PERF_EVENT_IOC_DISABLE, 0);
    ssize_t a = read(m_perf_fd, &count, sizeof(long long));
    (void)a;
    return count;
}
