#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <util/printf.h>

#include "event.h"
#include "font.h"
#include "messagestore.h"
#include "text.h"
#include "thumbnail.h"
#include "ui.h"
#include <util/divmod.h>

void ui_init() {
    g_handler = watchface_handle;
    g_messages_num = 10;
    for (int i = 0; i < g_messages_num; i++) {
        for (int j = 0; j < 576; j++) g_messages[i].image[j] = j * i * 27;
        sprintf_(g_messages[i].sender, "Name Surname");
        sprintf_(g_messages[i].timestamp, "13:37");
        sprintf_(g_messages[i].cropped, (i % 2) ? "%d: Lorem ipsum dolor sit amet..." : "%d: Never gonna give you up...",
                i);
        int fullidx = sprintf_(g_messages[i].full, "%d: this is a long message. ", i);
        for (int j = 0; j < 10; j++)
            fullidx += sprintf_(g_messages[i].full + fullidx, "Extra long line for testing %d\n", j);
        text_wrap(font_roboto, g_messages[i].cropped, 128, &g_messages[i].cropped_info);
        text_wrap(font_roboto, g_messages[i].full, 200, &g_messages[i].full_info);
    }
}
