#include "plat.h"
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "util/defs.h"
#include "util/except.h"
#include "util/log.h"
#include "roundedrect.h"

void blend_with_black(int off, uint16_t col, float fract) {
    float a = fract; // TODO: this is not physically correct, need to do blending in linear space
    if (a < 0) a = 0;
    else if (a > 1) a = 1;
    uint16_t v = __builtin_bswap16(g_target[off]);
    uint8_t cr = (col & 31), cg = ((col >> 5) & 63), cb = ((col >> 11) & 31);
    uint8_t r = (v & 31), g = ((v >> 5) & 63), b = ((v >> 11) & 31);
    uint8_t newr = r + (cr - r) * a;
    uint8_t newg = g + (cg - g) * a;
    uint8_t newb = b + (cb - b) * a;

    g_target[off] = __builtin_bswap16((newr << 0) | (newg << 5) | (newb << 11));
}

// TODO: extremely unoptimized
void roundedrect_round(int topx, int topy, int w, int h, int r, uint16_t col) {
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
                blend_with_black(j * g_pitch + l - bint, col, fract);
                for (int i = -bint + 1; i < bint; i++) g_target[j * g_pitch + l + i] = __builtin_bswap16(col);
                blend_with_black(j * g_pitch + l + bint, col, fract);
            } else if (a > -r && a < r) {
                for (int i = -deg45; i <= deg45; i++) {
                    float b = sqrtf(a * a + i * i);
                    float fract = r - b;
                    int o = i >= 0 ? (i + extendx / 2) : (i - extendx / 2);
                    blend_with_black(j * g_pitch + l + o, col, fract);
                }
                for (int i = -extendx / 2; i < extendx / 2; i++) g_target[j * g_pitch + l + i] = __builtin_bswap16(col);
            }
        } else {
            int max = w % 2 ? (w - 1) : w;
            for (int i = 1; i < max; i++) g_target[j * g_pitch + topx + i] = __builtin_bswap16(col);
        }
    }
}

// TODO: extremely unoptimized
void roundedrect(int topx, int topy, int w, int h, uint16_t col) {
    roundedrect_round(topx, topy, w, h, MIN(w / 2, h / 2), col);
}