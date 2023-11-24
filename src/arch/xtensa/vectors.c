#include "vectors.h"
#include "intrin.h"
#include "util/log.h"
#include "util/except.h"

#include <util/printf.h>

void kernel_exception_handler(interrupt_frame_t* frame) {
    LOG_ERROR("Got an exception %d", frame->exccause);
    ASSERT(!"Got an exception?!");
}
