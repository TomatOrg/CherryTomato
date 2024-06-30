#include <stdbool.h>
#include <stdint.h>
#include <util/printf.h>
#include <unistd.h>

#include "text.h"
#include <util/divmod.h>

// TODO: this structure can be optimized in various ways
// 1) switch to the DCT-based compression format
// 2) make the fields varlength instead of fixed
typedef struct message {
    int id;
    char sender[64], timestamp[16], cropped[64], full[512];
    int senderlen, timestamplen, croppedlen, fulllen;
    uint8_t image[576];

    text_wrapped_t cropped_info;
    text_wrapped_t full_info;
} message_t;

extern message_t g_messages[];
extern int g_messages_num;