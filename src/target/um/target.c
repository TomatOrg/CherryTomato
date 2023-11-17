#include <util/log.h>
#include <util/except.h>

void target_entry(void) {
    err_t err = NO_ERROR;

    LOG_INFO("Target: SDL-based simulator");

cleanup:
    if (IS_ERROR(err)) {
        LOG_CRITICAL("Failed to initialize sim :(");
        while (1);
    }
}
