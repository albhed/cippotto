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

    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    bool running = display_process_events(chip8);
    assert(running == true);

    free_chip8(chip8);
}

static void test_display_process_events_stops_on_quit(void) {
    SDL_Event quit_event;
    quit_event.type = SDL_QUIT;
    SDL_PushEvent(&quit_event);

    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    bool running = display_process_events(chip8);
    assert(running == false);

    free_chip8(chip8);
}

static void test_display_process_events_stops_on_escape_key(void) {
    Display* display = display_init();
    assert(display != NULL);

    SDL_PumpEvents();
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

    SDL_Event key_event;
    SDL_zero(key_event);
    key_event.type = SDL_KEYDOWN;
    key_event.key.keysym.scancode = SDL_SCANCODE_ESCAPE;
    SDL_PushEvent(&key_event);

    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    bool running = display_process_events(chip8);
    assert(running == false);

    free_chip8(chip8);
    display_destroy(display);
}

static void test_display_process_events_continues_on_chip8_key(void) {
    Display* display = display_init();
    assert(display != NULL);

    SDL_PumpEvents();
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

    SDL_Event key_event;
    SDL_zero(key_event);
    key_event.type = SDL_KEYDOWN;
    key_event.key.keysym.scancode = SDL_SCANCODE_1;
    SDL_PushEvent(&key_event);

    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    bool running = display_process_events(chip8);
    assert(running == true);
    assert(chip8->keys[0x1] == 1);

    free_chip8(chip8);
    display_destroy(display);
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

static void run_cpu_cycles(Chip8* chip8, int max_cycles) {
    uint16_t prev_pc = 0xFFFF;
    for (int i = 0; i < max_cycles; i++) {
        uint16_t current_pc = chip8->PC;
        if (current_pc == prev_pc) {
            break;
        }
        prev_pc = current_pc;
        uint16_t opcode = fetch(chip8);
        DecodedInstruction instruction = decode(opcode);
        int result = execute(chip8, instruction);
        assert(result == 0);
    }
}

static void test_display_renders_ibm_logo_rom(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    int load_result = load_rom(chip8, "../roms/ibm_logo.ch8");
    assert(load_result == 0);

    run_cpu_cycles(chip8, 50);
    assert(chip8->PC == 0x228);

    Display* display = display_init();
    assert(display != NULL);

    display_render(display, chip8);

    Uint32* pixel_buffer = malloc(sizeof(Uint32) * FRAME_WIDTH * FRAME_HEIGHT);
    assert(pixel_buffer != NULL);

    SDL_Rect viewport = {0, 0, FRAME_WIDTH, FRAME_HEIGHT};
    int result = SDL_RenderReadPixels(display->renderer, &viewport, SDL_PIXELFORMAT_RGBA32,
                                       pixel_buffer, FRAME_WIDTH * sizeof(Uint32));
    assert(result == 0);

    SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
    assert(format != NULL);

    long white_pixel_count = 0;
    for (int i = 0; i < FRAME_WIDTH * FRAME_HEIGHT; i++) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixel_buffer[i], format, &r, &g, &b, &a);
        if (r == 255 && g == 255 && b == 255) {
            white_pixel_count++;
        }
    }

    int lit_logical_pixels = 0;
    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        if (chip8->display[i]) {
            lit_logical_pixels++;
        }
    }

    assert(lit_logical_pixels > 100);
    assert(white_pixel_count == (long)lit_logical_pixels * DISPLAY_SCALE * DISPLAY_SCALE);

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
    test_display_process_events_stops_on_escape_key();
    test_display_process_events_continues_on_chip8_key();
    test_display_render_draws_pixel_at_correct_position();
    test_display_renders_ibm_logo_rom();

    SDL_Quit();
    printf("All display tests passed!\n");
    return 0;
}
