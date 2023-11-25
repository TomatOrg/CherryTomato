#include "plat.h"
#include <util/divmod.h>
#include <stdint.h>

// this completely disregards the half-pixel offset in chroma
// so it's wrong, but it's fast
void thumbnail_draw(const uint8_t *restrict img, int x, int y) {
    const uint8_t *restrict luma = img;
    const uint8_t *restrict chroma = img + 32 * 32 / 2;
    int start = MAX(g_line, y);
    int end = MIN(g_line + g_nlines, y + 64);
    for (int l = start; l < end;) {
        int imgline = l - y;
        for (int i = 0; i < 8; i++) {
            uint8_t crA = chroma[imgline / 8 * 8 + i / 2], crB = chroma[imgline / 8 * 8 + MIN(i / 2 + 1, 7)];
            uint8_t crC = chroma[MIN((imgline / 8 + 1), 7) * 8 + i / 2],
                    crD = chroma[MIN((imgline / 8 + 1), 7) * 8 + MIN(i / 2 + 1, 7)];

            uint16_t reds[4];
            uint16_t blues[4];
            for (int ii = 0; ii < 4; ii++) {
                uint16_t rTop = (crA >> 4) * (4 - ii) + (crB >> 4) * ii;
                uint16_t bTop = (crA & 0xF) * (4 - ii) + (crB & 0xF) * ii;
                uint16_t rBottom = (crC >> 4) * (4 - ii) + (crD >> 4) * ii;
                uint16_t bBottom = (crC & 0xF) * (4 - ii) + (crD & 0xF) * ii;
                reds[ii] = rTop * (8 - imgline % 8) + rBottom * (imgline % 8);
                blues[ii] = bTop * (8 - imgline % 8) + bBottom * (imgline % 8);
            }

            uint16_t gg = *(uint16_t *)(luma + (imgline / 2 * 16 + i * 2)); // TODO: this is UB
            for (int ii = 0; ii < 4; ii++) {
                uint16_t g = (gg & 0xF) * (240 / 15);
                gg >>= 4;
                uint16_t r = MIN(240, MAX(0, g - (reds[ii] - 480 / 2)));
                uint16_t b = MIN(240, MAX(0, g - (blues[ii] - 480 / 2)));

                uint16_t col = (r * 31 / 240) | ((g * 63 / 240) << 5) | ((b * 31 / 240) << 11);
                g_target[g_pitch * (l - g_line) + x + i * 8 + ii * 2] = __builtin_bswap16(col);
                g_target[g_pitch * (l - g_line) + x + i * 8 + ii * 2 + 1] = __builtin_bswap16(col);
                g_target[g_pitch * (l - g_line + 1) + x + i * 8 + ii * 2] = __builtin_bswap16(col);
                g_target[g_pitch * (l - g_line + 1) + x + i * 8 + ii * 2 + 1] = __builtin_bswap16(col);
            }
        }
        // this is a neat trick
        // if the first line was odd, increase by 1 so the next line will be even
        l += 2 - (l & 1);
    }
}
