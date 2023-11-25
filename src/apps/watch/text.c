#include "text.h"
#include "plat.h"
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
    int idx = -1;
    font_charinfo_t *chars = (void *)chinfo->storage;
    for (int j = 0; j < chinfo->count; j++) {
        if (chars[j].codepoint == ch) {
            idx = j;
            break;
        }
    }
    ASSERT(idx != -1);
    return idx;
}

int text_advance(font_info_t *chinfo, int chidx) {
    font_charinfo_t *ch = (void *)(chinfo->storage + chidx * sizeof(font_charinfo_t));
    return ch->advance;
}

int text_drawchar(font_info_t *chinfo, int chidx, int x, int basey) {
    font_charinfo_t *ch = (void *)(chinfo->storage + chidx * sizeof(font_charinfo_t));
    uint8_t *atlas = (void *)(chinfo->storage + chinfo->count * sizeof(font_charinfo_t));
    uint8_t *lines = atlas + ch->startidx;

    int height = ch->height;
    int width = ch->width;
    int y = basey - ch->top;
    x += ch->left;

    if (ch->width == 0) return ch->advance;

    int start = MAX(g_line, y);
    int end = MIN(g_line + g_nlines, y + height);

    int rleoff = height;
    for (int i = 0; i < start - basey + ch->top; i++) rleoff += lines[i];

    for (int l = start; l < end; l++) {
        int entries_read = 2 * (4 - rleoff % 4);
        uint32_t *data = (uint32_t*)(lines + rleoff - rleoff % 4);
        uint32_t bitbuffer = *data++;
        bitbuffer >>= (rleoff % 4) * 8;

        for (int i = 0; i < width;) {
            uint8_t intensity;
            {
                intensity = bitbuffer & 0xF;
                bitbuffer >>= 4;
                if (--entries_read == 0) {
                    entries_read = 8;
                    bitbuffer = *data++;
                }
            }
            uint8_t count = 1;
            if (intensity == 0 || intensity == 15) {
                count = bitbuffer & 0xF;
                bitbuffer >>= 4;
                if (--entries_read == 0) {
                    entries_read = 8;
                    bitbuffer = *data++;
                }
            }
            for (int j = 0; j < count; j++) {
                uint8_t newr = intensity << 1;
                uint8_t newg = intensity << 2;
                uint8_t newb = intensity << 1;
                g_target[g_pitch * (l - g_line) + x + i++] = (newr << 0) | (newg << 5) | (newb << 11);
            }
        }
        rleoff += lines[l - basey + ch->top];
    }
    return ch->advance;
}

void text_drawline(font_info_t *chinfo, const char *str, int x, int basey) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) { x += text_drawchar(chinfo, getidx(chinfo, str[i]), x, basey); }
}

static void text_draw_wrapped_internal(font_info_t *chinfo, const char *str, int textstart, int textpos,
                                                  int *xpos, int basey) {
    for (int i = textstart; i < textpos; i++) {
        if (str[i] != '\n' && str[i] != 0) { // TODO: this is a bit of a hack
            int idx = getidx(chinfo, str[i]);
            *xpos += text_drawchar(chinfo, idx, *xpos, basey);
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
                                                  basey + textline * 22);
        if (textline > 0) {
            xpos = x;
            int prevline = textline - 1;
            if ((prevline + 1) == textlines) end = msg->length;
            else end = msg->starts[prevline + 1];
            text_draw_wrapped_internal(chinfo, str, msg->starts[prevline], end, &xpos,
                                                  basey + prevline * 22);
        }
        xpos = x;
        textline++;
    }
    return basey + 22 - 14;
}