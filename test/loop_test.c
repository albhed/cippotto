#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "../src/chip8.h"
#include "../src/display.h"
#include "../src/audio.h"
#include "../src/loop.h"

static void test_run_frame_stops_on_quit_event(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    Display* display = display_init();
    assert(display != NULL);
    Audio* audio = audio_init();
    assert(audio != NULL);

    SDL_Event quit_event;
    quit_event.type = SDL_QUIT;
    SDL_PushEvent(&quit_event);

    bool running = run_frame(chip8, display, audio, 16);
    assert(running == false);

    audio_destroy(audio);
    display_destroy(display);
    free_chip8(chip8);
}

static void test_run_frame_continues_without_quit_event(void) {
    SDL_PumpEvents();
    SDL_FlushEvent(SDL_QUIT);

    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    int load_result = load_rom(chip8, "../roms/ibm_logo.ch8");
    assert(load_result == 0);

    Display* display = display_init();
    assert(display != NULL);
    Audio* audio = audio_init();
    assert(audio != NULL);

    bool running = run_frame(chip8, display, audio, 16);
    assert(running == true);

    audio_destroy(audio);
    display_destroy(display);
    free_chip8(chip8);
}

static void test_run_frame_executes_instructions_up_to_the_program_loop(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    int load_result = load_rom(chip8, "../roms/ibm_logo.ch8");
    assert(load_result == 0);

    Display* display = display_init();
    assert(display != NULL);
    Audio* audio = audio_init();
    assert(audio != NULL);

    for (int frame = 0; frame < 20; frame++) {
        bool running = run_frame(chip8, display, audio, 16);
        assert(running == true);
    }

    assert(chip8->PC == 0x228);

    audio_destroy(audio);
    display_destroy(display);
    free_chip8(chip8);
}

static void test_run_frame_updates_timers_at_real_60hz_rate(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    int load_result = load_rom(chip8, "../roms/ibm_logo.ch8");
    assert(load_result == 0);

    Display* display = display_init();
    assert(display != NULL);
    Audio* audio = audio_init();
    assert(audio != NULL);

    chip8->delay_timer = 60;

    Uint32 start = SDL_GetTicks();
    Uint32 last_timer_update = start;

    while (chip8->delay_timer > 0) {
        Uint32 now = SDL_GetTicks();
        uint32_t elapsed_ms = now - last_timer_update;
        last_timer_update = now;

        bool running = run_frame(chip8, display, audio, elapsed_ms);
        assert(running == true);

        SDL_Delay(16);
    }

    Uint32 elapsed = SDL_GetTicks() - start;

    assert(elapsed >= 850 && elapsed <= 1250);

    audio_destroy(audio);
    display_destroy(display);
    free_chip8(chip8);
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    printf("Running main loop tests...\n");
    test_run_frame_stops_on_quit_event();
    test_run_frame_continues_without_quit_event();
    test_run_frame_executes_instructions_up_to_the_program_loop();
    test_run_frame_updates_timers_at_real_60hz_rate();

    SDL_Quit();
    printf("All main loop tests passed!\n");
    return 0;
}
