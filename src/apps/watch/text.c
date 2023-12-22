#include "text.h"
#include "apps/watch/font.h"
#include "plat.h"
#include "util/log.h"
#include <stdint.h>
#include <util/divmod.h>
#include <string.h>
#include <stdio.h>

#include <util/except.h>


size_t strlen(const char *str) {
	const char *s;
	for (s = str; *s; ++s);
	return s - str;
}


static int getidx(font_info_t *chinfo, char ch) {
    font_charinfo_t *chars = (void *)chinfo->storage;
    int low = 0;
    int high = chinfo->count - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (chars[mid].codepoint == ch) return mid;
        else if (chars[mid].codepoint < ch) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}

int text_advance(font_info_t *chinfo, int chidx) {
    font_charinfo_t *ch = (void *)(chinfo->storage + chidx * sizeof(font_charinfo_t));
    return ch->advance;
}

void draw_rle(int start, int end, int rleoff, uint8_t* lines, int x, int y) {
    for (int l = start; l < end; l++) {
        int entries_read = 2 * (4 - rleoff % 4);
        uint32_t *data = (uint32_t*)(lines + rleoff - rleoff % 4);
        uint32_t bitbuffer = *data++;
        bitbuffer >>= (rleoff % 4) * 8;
        int toread = lines[l - y] * 2;
        for (int i = 0; toread > 0;) {
            uint8_t intensity = bitbuffer & 0xF;
            bitbuffer >>= 4; toread--;
            if (--entries_read == 0) { entries_read = 8; bitbuffer = *data++; }
            uint8_t count = 1;
            if (intensity >= 14) {
                count = (bitbuffer & 0xF);
                bitbuffer >>= 4;
                toread--;
                if (--entries_read == 0) { entries_read = 8; bitbuffer = *data++; }
                if (intensity == 15) for (int j = 0; j < count; j++) {
                    g_target[g_pitch * (l - g_line) + x + i++] = 0xFFFF;
                } else {
                    i += count;
                }
            } else {
                uint16_t v = __builtin_bswap16(g_target[g_pitch * (l - g_line) + x + i]);
                uint8_t r = (v & 31), g = ((v >> 5) & 63), b = ((v >> 11) & 31);
                uint8_t newr = r + (31 - r) * intensity * (256 / 13) / 256;
                uint8_t newg = g + (63 - g) * intensity * (256 / 13) / 256;
                uint8_t newb = b + (31 - b) * intensity * (256 / 13) / 256;
                g_target[g_pitch * (l - g_line) + x + i++] = __builtin_bswap16((newr << 0) | (newg << 5) | (newb << 11));
            }
        }
        rleoff += lines[l - y];
    }
}

void text_drawicon(uint8_t* data, int x, int y) {
    // TODO: UB
    int height = *(uint16_t*)(data + 2);
    uint8_t* lines = data + 4;
    int start = MAX(g_line, y);
    int end = MIN(g_line + g_nlines, y + height);
    int rleoff = height;
    for (int i = 0; i < MIN(height, start - y); i++) rleoff += lines[i];
    draw_rle(start, end, rleoff, lines, x, y);
}

int text_drawchar_internal(font_info_t *chinfo, int chidx, int x, int basey, bool descenders_only) {
    font_charinfo_t *ch = (void *)(chinfo->storage + chidx * sizeof(font_charinfo_t));
    uint8_t *atlas = (void *)(chinfo->storage + chinfo->count * sizeof(font_charinfo_t));
    uint8_t *lines = atlas + ch->startidx;
    int height = ch->height;
    int y = descenders_only ? basey : (basey - ch->top);
    x += ch->left;
    if (ch->width == 0) return ch->advance;
    int start = MAX(g_line, y);
    int bound = descenders_only ? (y + height - ch->top) : (y + height);
    int end = MIN(g_line + g_nlines, bound);
    int rleoff = height;
    for (int i = 0; i < MIN(height, start - basey + ch->top); i++) rleoff += lines[i];
    draw_rle(start, end, rleoff, lines, x, basey - ch->top);
    return ch->advance;
}

int text_drawchar(font_info_t *chinfo, int chidx, int x, int basey) {
    return text_drawchar_internal(chinfo, chidx, x, basey, false);
}

int text_getlinesize(font_info_t *chinfo, const char *str) {
    int len = strlen(str);
    int x = 0;
    for (int i = 0; i < len; i++) { x += text_advance(chinfo, getidx(chinfo, str[i])); }
    return x;
}

void text_drawline(font_info_t *chinfo, const char *str, int x, int basey) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) { x += text_drawchar(chinfo, getidx(chinfo, str[i]), x, basey); }
}

static void text_draw_wrapped_internal(font_info_t *chinfo, const char *str, int textstart, int textpos,
                                                  int *xpos, int basey, bool descenders_only) {
    for (int i = textstart; i < textpos; i++) {
        if (str[i] != '\n' && str[i] != 0) { // TODO: this is a bit of a hack
            int idx = getidx(chinfo, str[i]);
            *xpos += text_drawchar_internal(chinfo, idx, *xpos, basey, descenders_only);
        }
    }
}

// TODO: rewrite all of this
void text_wrap(font_info_t *chinfo, const char *str, int wrapat, text_wrapped_t *dest) {
    int textlen = strlen(str);
    dest->length = textlen;
    int xorig = 0;
    int textstart = 0;
    int textpos = 0;
    int xstart = xorig;
    int xpos = xorig;

    int basey = 0;
    dest->starts[0] = 0;
    bool was_newline = false;
    while (textpos < textlen) {
        bool fits = true;
        while (textpos < textlen && str[textpos] != ' ' && str[textpos] != '\n') {
            int adv = text_advance(chinfo, getidx(chinfo, str[textpos]));
            if (xpos + adv >= wrapat) {
                fits = false;
                break;
            }
            xpos += adv;
            textpos++;
        }
        was_newline = str[textpos] == '\n';
        if (fits) {
            if (str[textpos] != 0 && str[textpos] != '\n') textpos++;
            xpos = xstart;
            dest->starts[basey] = MIN(dest->starts[basey], textstart);
            for (int i = textstart; i < textpos; i++) xpos += text_advance(chinfo, getidx(chinfo, str[i]));
            textstart = textpos;
            xstart = xpos;
        } else {
            xstart = xpos = xorig;
            basey += 1;
            for (int i = textstart; i < textpos; i++) xpos += text_advance(chinfo, getidx(chinfo, str[i]));
            dest->starts[basey] = textstart;
        }
        if (was_newline) {
            textpos++;
            textstart++;
            xstart = xpos = xorig;
            basey += 1;
            dest->starts[basey] = textstart;
        }
    }
    if (!was_newline) basey++;
    dest->numlines = basey;
}

// TODO: rewrite this
int text_draw_wrapped(font_info_t *chinfo, text_wrapped_t *msg, const char *str, int x, int basey) {
    int xpos = x;
    int textlines = msg->numlines;
    int textline = floordiv(g_line - (basey - 22), 22);
    if (textline < -1) return 0; // TODO: shouldnt happen, ASSERT if it does
    if (textline > textlines + 1) return 0;
    while (true) {
        int end;
        if (basey + (textline - 1) * 22 > g_line + g_nlines) break;
        if ((textline + 1) == textlines) end = msg->length;
        else end = msg->starts[textline + 1];
        if (textline < textlines)
            text_draw_wrapped_internal(chinfo, str, msg->starts[textline], end, &xpos,
                                                  basey + textline * 22, false);
        if (textline > 0 && textline <= textlines) {
            xpos = x;
            int prevline = textline - 1;
            if ((prevline + 1) == textlines) end = msg->length;
            else end = msg->starts[prevline + 1];
            if (basey + prevline * 22 <= g_line) {
                    text_draw_wrapped_internal(chinfo, str, msg->starts[prevline], end, &xpos,
                        basey + prevline * 22, true);
            }
        }
        xpos = x;
        textline++;
    }
    return basey + 22 - 14;
}
