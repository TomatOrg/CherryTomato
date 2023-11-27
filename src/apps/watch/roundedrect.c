#include "plat.h"
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "util/defs.h"
#include "util/log.h"
#include "roundedrect.h"

uint16_t gammablend_with_black(uint16_t col, float fract) {
    float a = powf(fract, 1 / 2.0);
    if (a < 0) a = 0;
    else if (a > 1) a = 1;
    uint16_t r5 = ((col >> 0) & ((1 << 5) - 1)) * a;
    uint16_t g6 = ((col >> 5) & ((1 << 6) - 1)) * a;
    uint16_t b5 = ((col >> 11) & ((1 << 5) - 1)) * a;
    uint16_t outlinecol = (r5 << 0) | (g6 << 5) | (b5 << 11);
    return outlinecol;
}

// TODO: extremely unoptimized
void roundedrect(int topx, int topy, int w, int h, uint16_t col) {
    int r = MIN(w / 2, h / 2);
    int extendx = w - 2 * r;
    int cx = topx + w / 2;
    for (int j = 0; j < g_nlines; j++) {
        int deg45 = floorf(r * (1 / sqrtf(2)));
        int a = 0;
        int l = cx;
        bool is_circle = false;
        int line = g_line + j;
        int ll = line - topy;
        if (ll < r) {
            is_circle = true;
            a = -ll + r;
        } else if (ll >= (h - r)) {
            is_circle = true;
            a = (h - ll) - r;
        }

        if (is_circle) {
            if (a >= -deg45 && a <= deg45) {
                float b = sqrtf(r * r - a * a);
                int bint = floorf(b);
                float fract = b - bint;
                bint += extendx / 2;
                g_target[j * g_pitch + l - bint] = __builtin_bswap16(gammablend_with_black(col, fract));
                for (int i = -bint + 1; i < bint; i++) g_target[j * g_pitch + l + i] = __builtin_bswap16(col);
                g_target[j * g_pitch + l + bint] = __builtin_bswap16(gammablend_with_black(col, fract));
            } else if (a > -r && a < r) {
                for (int i = -deg45; i <= deg45; i++) {
                    float b = sqrtf(a * a + i * i);
                    float fract = r - b;
                    int o = i >= 0 ? (i + extendx / 2) : (i - extendx / 2);
                    g_target[j * g_pitch + l + o] = __builtin_bswap16(gammablend_with_black(col, fract));
                }
                for (int i = -extendx / 2; i < extendx / 2; i++) g_target[j * g_pitch + l + i] = __builtin_bswap16(col);
            }
        } else {
            for (int i = 1; i < w; i++) g_target[j * g_pitch + topx + i] = __builtin_bswap16(col);
        }
    }
}