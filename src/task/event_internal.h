#pragma once

#include <stdint.h>

#include "tpl.h"

/**
 * Bitmask of events that are pending
 */
extern uint32_t g_event_pending;

/**
 * Internal function to run the notifies waiting on the given tpl
 */
void event_dispatch_notifies(tpl_t tpl);
