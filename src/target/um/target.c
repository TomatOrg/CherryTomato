#include <SDL2/SDL_config.h>
#include <SDL2/SDL_mouse.h>
#include <util/log.h>
#include <util/except.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <assert.h>
#include <util/divmod.h>

static SDL_Window *win;
static SDL_Surface *winsurf;
#ifndef __EMSCRIPTEN__
static SDL_Window *win2;
static SDL_Surface *win2surf;
#endif
void target_entry(void) {
    err_t err = NO_ERROR;

    LOG_INFO("Target: SDL-based simulator");

    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
    assert(SDL_Init(SDL_INIT_EVENTS) >= 0);
    assert(SDL_Init(SDL_INIT_TIMER) >= 0);

#ifndef __EMSCRIPTEN__
    win2 = SDL_CreateWindow("CherryTomato Debug", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 240, 320, 0);
    assert(win2);
    win2surf = SDL_GetWindowSurface(win2);
    assert(win2surf);
#endif

    win = SDL_CreateWindow("CherryTomato", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 240, 240, 0);
    assert(win);
    winsurf = SDL_GetWindowSurface(win);
    assert(winsurf);


cleanup:
    if (IS_ERROR(err)) {
        LOG_CRITICAL("Failed to initialize sim :(");
        while (1);
    }
}

void target_touch(bool* pressed, int* x, int* y) {
    SDL_Event e;
    while(SDL_PollEvent(&e));

    int xx, yy;
    uint32_t state = SDL_GetMouseState(&xx, &yy);
    *pressed = !!(state & SDL_BUTTON(SDL_BUTTON_LEFT));
    *x = xx;
    *y = yy;
}

static uint16_t framebuffer[240 * 320];
static int target_scrolloff;

static void sdl_update() {
    int dmastartpos = target_scrolloff;

    SDL_LockSurface(winsurf);
    for (int i = 0; i < 240; i++) {
        for (int j = 0; j < 240; j++) {
            uint32_t *dest = winsurf->pixels + i * winsurf->pitch + j * 4;
            uint16_t src = framebuffer[((i + dmastartpos) % 320) * 240 + j];
            uint8_t r = ((src) & ((1 << 5) - 1)) * 255 / 31;
            uint8_t g = ((src >> 5) & ((1 << 6) - 1)) * 255 / 63;
            uint8_t b = ((src >> 11) & ((1 << 5) - 1)) * 255 / 31;
            *dest = r | (g << 8) | (b << 16);
        }
    }
    SDL_UnlockSurface(winsurf);
    SDL_UpdateWindowSurface(win);
#ifndef __EMSCRIPTEN__
    SDL_LockSurface(win2surf);
    for (int i = 0; i < 320; i++) {
        for (int j = 0; j < 240; j++) {
            uint32_t *dest = win2surf->pixels + i * win2surf->pitch + j * 4;
            uint16_t src = framebuffer[((i) % 320) * 240 + j];
            uint8_t r = ((src) & ((1 << 5) - 1)) * 255 / 31;
            uint8_t g = ((src >> 5) & ((1 << 6) - 1)) * 255 / 63;
            uint8_t b = ((src >> 11) & ((1 << 5) - 1)) * 255 / 31;
            *dest = r | (g << 8) | (b << 16);
        }
    }
    SDL_UnlockSurface(win2surf);
    SDL_UpdateWindowSurface(win2);
#endif
}

long long perf_stop();

void target_set_vertical_scrolloff(uint16_t scrolloff) {
    int max = 64*1000*1000 / 4 / 40;
    int cyc = perf_stop();
    if (cyc > max) {
        printf("WARNING: %d %d\n", cyc, max);
    }
    if (target_scrolloff != scrolloff) {
        target_scrolloff = scrolloff;
    }
    sdl_update();
}
void target_blit(uint16_t *buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            framebuffer[(x + j) + ((y + i) % 320) * 240] = __builtin_bswap16(buffer[i * w + j]);
        }
    }
}
