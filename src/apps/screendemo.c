#include "util/log.h"
#include "drivers/st7789/st7789.h"
#include "drivers/ft6x06/ft6x06.h"
#include "event/delay.h"

void screendemo() {
    err_t err = NO_ERROR;
    while (1) {
        bool pressed;
        uint16_t x, y;
        CHECK_AND_RETHROW(ft6x06_touch(&pressed, &x, &y));
        if (pressed) {
            st7789_fillrect(0, x, 320-y, 8, 8);
        }
        delay(1000*16);
    }
cleanup:
    if (IS_ERROR(err)) {
        LOG_CRITICAL("Error :(");
        while (1);
    }
}