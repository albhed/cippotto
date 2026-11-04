#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "../src/chip8.h"
#include "../src/display.h"

#define FRAME_WIDTH (DISPLAY_WIDTH * DISPLAY_SCALE)
#define FRAME_HEIGHT (DISPLAY_HEIGHT * DISPLAY_SCALE)

static void test_display_init_creates_window_and_renderer(void) {
    Display* display = display_init();
    assert(display != NULL);
    assert(display->window != NULL);
    assert(display->renderer != NULL);
    display_destroy(display);
}

static void test_display_process_events_continue_when_no_event(void) {
    SDL_PumpEvents();
    SDL_FlushEvent(SDL_QUIT);

    bool running = display_process_events();
    assert(running == true);
}

static void test_display_process_events_stops_on_quit(void) {
    SDL_Event quit_event;
    quit_event.type = SDL_QUIT;
    SDL_PushEvent(&quit_event);

    bool running = display_process_events();
    assert(running == false);
}

static void test_display_render_draws_pixel_at_correct_position(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    int px = 5, py = 3;
    chip8->display[py * DISPLAY_WIDTH + px] = 1;

    Display* display = display_init();
    assert(display != NULL);

    display_render(display, chip8);

    Uint32* pixel_buffer = malloc(sizeof(Uint32) * FRAME_WIDTH * FRAME_HEIGHT);
    assert(pixel_buffer != NULL);

    SDL_Rect viewport = {0, 0, FRAME_WIDTH, FRAME_HEIGHT};
    int result = SDL_RenderReadPixels(display->renderer, &viewport, SDL_PIXELFORMAT_RGBA32,
                                       pixel_buffer, FRAME_WIDTH * sizeof(Uint32));
    assert(result == 0);

    int lit_x = px * DISPLAY_SCALE + DISPLAY_SCALE / 2;
    int lit_y = py * DISPLAY_SCALE + DISPLAY_SCALE / 2;
    Uint32 lit_pixel = pixel_buffer[lit_y * FRAME_WIDTH + lit_x];

    int dark_x = (px + 1) * DISPLAY_SCALE + DISPLAY_SCALE / 2;
    int dark_y = py * DISPLAY_SCALE + DISPLAY_SCALE / 2;
    Uint32 dark_pixel = pixel_buffer[dark_y * FRAME_WIDTH + dark_x];

    SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
    assert(format != NULL);

    Uint8 r, g, b, a;
    SDL_GetRGBA(lit_pixel, format, &r, &g, &b, &a);
    assert(r == 255 && g == 255 && b == 255);

    SDL_GetRGBA(dark_pixel, format, &r, &g, &b, &a);
    assert(r == 0 && g == 0 && b == 0);

    SDL_FreeFormat(format);
    free(pixel_buffer);
    display_destroy(display);
    free_chip8(chip8);
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    printf("Running display tests...\n");
    test_display_init_creates_window_and_renderer();
    test_display_process_events_continue_when_no_event();
    test_display_process_events_stops_on_quit();
    test_display_render_draws_pixel_at_correct_position();

    SDL_Quit();
    printf("All display tests passed!\n");
    return 0;
}
