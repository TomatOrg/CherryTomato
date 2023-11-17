#pragma once

#include <util/except.h>

#include "event.h"

typedef struct io_token {
    // If the event is NULL blocking io is performed
    // If the event is non-NULL then non-blocking io is performed,
    // and the event will be signaled when the read request is completed.
    event_t* event;

    // the status of the request, so you can know what happened to it
    err_t error;

    // the buffer for the request
    size_t buffer_size;
    void* buffer;
} io_token_t;
