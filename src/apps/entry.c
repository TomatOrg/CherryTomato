#include "entry.h"
#include "util/log.h"

void watch_main();

void cherry_tomato_entry() {
    LOG_INFO("Starting up watch");
    watch_main();
}
