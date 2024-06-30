#include <SDL2/SDL_config.h>
#include <SDL2/SDL_mouse.h>
#include <util/log.h>
#include <util/except.h>
#include <SDL2/SDL.h>

#include "perf.h"
#include "task/timer_internal.h"

/**
 * Main display
 */
static SDL_Window* m_win;
static SDL_Surface* m_winsurf;

/**
 * Debug display (shows full vram)
 */
#ifndef __EMSCRIPTEN__
static SDL_Window* m_win2;
static SDL_Surface* m_win2surf;
#endif

/**
 * The framebuffer, we will blit to this
 * and blit this to the screen later
 */
static uint16_t m_framebuffer[240 * 320];

/**
 * The scroll offset
 */
static int m_target_scrolloff;

/**
 * The time of the next tick
 */
static uint64_t m_next_tick;

/**
 * Blit the full framebuffer as needed
 */
static void sdl_update() {
    int dmastartpos = m_target_scrolloff;

    SDL_LockSurface(m_winsurf);
    for (int i = 0; i < 240; i++) {
        for (int j = 0; j < 240; j++) {
            uint32_t *dest = m_winsurf->pixels + i * m_winsurf->pitch + j * 4;
            uint16_t src = m_framebuffer[((i + dmastartpos) % 320) * 240 + j];
            uint8_t r = ((src) & ((1 << 5) - 1)) * 255 / 31;
            uint8_t g = ((src >> 5) & ((1 << 6) - 1)) * 255 / 63;
            uint8_t b = ((src >> 11) & ((1 << 5) - 1)) * 255 / 31;
            *dest = r | (g << 8) | (b << 16);
        }
    }
    SDL_UnlockSurface(m_winsurf);
    SDL_UpdateWindowSurface(m_win);

#ifndef __EMSCRIPTEN__
    SDL_LockSurface(m_win2surf);
    for (int i = 0; i < 320; i++) {
        for (int j = 0; j < 240; j++) {
            uint32_t *dest = m_win2surf->pixels + i * m_win2surf->pitch + j * 4;
            uint16_t src = m_framebuffer[((i) % 320) * 240 + j];
            uint8_t r = ((src) & ((1 << 5) - 1)) * 255 / 31;
            uint8_t g = ((src >> 5) & ((1 << 6) - 1)) * 255 / 63;
            uint8_t b = ((src >> 11) & ((1 << 5) - 1)) * 255 / 31;
            *dest = r | (g << 8) | (b << 16);
        }
    }
    SDL_UnlockSurface(m_win2surf);
    SDL_UpdateWindowSurface(m_win2);
#endif
}

void watch_loop();

void target_loop() {
    // do any event handling needed
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                exit(0);
            }
        }
    }

    // check for timer
    if (SDL_GetTicks64() >= m_next_tick) {
        timer_dispatch();
    }

    // update the display
    watch_loop();

    // TODO: cap at some fps
    sdl_update();

    // sleep for some time
    // TODO: something smarter somehow
    SDL_Delay(1);
}
void target_entry(void) {
    err_t err = NO_ERROR;

    LOG_INFO("Target: SDL-based simulator");

    CHECK(SDL_Init(SDL_INIT_VIDEO) >= 0);
    CHECK(SDL_Init(SDL_INIT_EVENTS) >= 0);
    CHECK(SDL_Init(SDL_INIT_TIMER) >= 0);

#ifndef __EMSCRIPTEN__
    m_win2 = SDL_CreateWindow("Cherry Tomato Debug", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 240, 320, 0);
    CHECK(m_win2 != NULL);
    m_win2surf = SDL_GetWindowSurface(m_win2);
    CHECK(m_win2surf != NULL);
#endif

    m_win = SDL_CreateWindow("Cherry Tomato", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 240, 240, 0);
    CHECK(m_win);
    m_winsurf = SDL_GetWindowSurface(m_win);
    CHECK(m_winsurf);

cleanup:
    if (IS_ERROR(err)) {
        LOG_CRITICAL("Failed to initialize sim :(");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Target agnostic functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------------------------------------------------
// Input functions
//----------------------------------------------------------------------------------------------------------------------

void target_touch(bool* pressed, int* x, int* y) {
    int xx, yy;
    uint32_t state = SDL_GetMouseState(&xx, &yy);
    *pressed = !!(state & SDL_BUTTON(SDL_BUTTON_LEFT));
    *x = xx;
    *y = yy;
}

//----------------------------------------------------------------------------------------------------------------------
// Display functions
//----------------------------------------------------------------------------------------------------------------------

void target_set_vertical_scrolloff(uint16_t scrolloff) {
    if (m_target_scrolloff != scrolloff) {
        m_target_scrolloff = scrolloff;
    }
}

void target_blit(uint16_t* buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            m_framebuffer[(x + j) + ((y + i) % 320) * 240] = __builtin_bswap16(buffer[i * w + j]);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
// Timer related
//----------------------------------------------------------------------------------------------------------------------

void target_set_next_tick(uint64_t tick) {
    m_next_tick = tick;
}

uint64_t target_get_current_tick() {
    return SDL_GetTicks64();
}
