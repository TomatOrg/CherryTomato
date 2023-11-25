#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "util/except.h"

/**
 * Initialize the esp32 bluetooth controller
 */
err_t esp32_init_bluetooth();
