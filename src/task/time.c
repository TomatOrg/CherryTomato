#include "time.h"

void udelay(uint32_t us) {
    uint64_t target = get_system_time() + us;
    while (get_system_time() < target);
}
