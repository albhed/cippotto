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

static void write_four_consecutive_dxyn(Chip8* chip8) {
    chip8->V[0x0] = 0;
    chip8->V[0x1] = 0;
    chip8->I = FONTSET_START;

    for (int i = 0; i < 4; i++) {
        chip8->memory[PC_START + i * 2] = 0xD0;
        chip8->memory[PC_START + i * 2 + 1] = 0x11;
    }

    uint16_t loop_address = PC_START + 4 * 2;
    chip8->memory[loop_address] = 0x10 | ((loop_address >> 8) & 0x0F);
    chip8->memory[loop_address + 1] = loop_address & 0xFF;
}

static void test_run_frame_without_display_wait_quirk_executes_all_dxyn_in_one_frame(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    write_four_consecutive_dxyn(chip8);

    Display* display = display_init();
    assert(display != NULL);
    Audio* audio = audio_init();
    assert(audio != NULL);

    bool running = run_frame(chip8, display, audio, 16);
    assert(running == true);
    assert(chip8->PC == PC_START + 8);

    audio_destroy(audio);
    display_destroy(display);
    free_chip8(chip8);
}

static void test_run_frame_with_display_wait_quirk_executes_at_most_one_dxyn_per_frame(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    chip8->quirks.quirk_display_wait = true;
    write_four_consecutive_dxyn(chip8);

    Display* display = display_init();
    assert(display != NULL);
    Audio* audio = audio_init();
    assert(audio != NULL);

    for (int expected_instructions = 1; expected_instructions <= 4; expected_instructions++) {
        bool running = run_frame(chip8, display, audio, 16);
        assert(running == true);
        assert(chip8->PC == PC_START + expected_instructions * 2);
    }

    audio_destroy(audio);
    display_destroy(display);
    free_chip8(chip8);
}

static void write_non_dxyn_program_with_self_loop(Chip8* chip8, int n) {
    for (int i = 0; i < n; i++) {
        chip8->memory[PC_START + i * 2] = 0x60;
        chip8->memory[PC_START + i * 2 + 1] = (uint8_t)i;
    }

    uint16_t loop_address = PC_START + n * 2;
    chip8->memory[loop_address] = 0x10 | ((loop_address >> 8) & 0x0F);
    chip8->memory[loop_address + 1] = loop_address & 0xFF;
}

static void test_run_frame_display_wait_quirk_runs_non_dxyn_instructions_beyond_old_budget(void) {
    int n = INSTRUCTIONS_PER_FRAME + 5;
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    chip8->quirks.quirk_display_wait = true;
    write_non_dxyn_program_with_self_loop(chip8, n);

    Display* display = display_init();
    assert(display != NULL);
    Audio* audio = audio_init();
    assert(audio != NULL);

    bool running = run_frame(chip8, display, audio, 16);
    assert(running == true);
    assert(chip8->PC == PC_START + n * 2);

    audio_destroy(audio);
    display_destroy(display);
    free_chip8(chip8);
}

static void write_long_non_dxyn_program_then_dxyn(Chip8* chip8, int n) {
    chip8->V[0x0] = 0;
    chip8->V[0x1] = 0;
    chip8->I = FONTSET_START;

    for (int i = 0; i < n; i++) {
        chip8->memory[PC_START + i * 2] = 0x60;
        chip8->memory[PC_START + i * 2 + 1] = (uint8_t)i;
    }

    uint16_t dxyn_address = PC_START + n * 2;
    chip8->memory[dxyn_address] = 0xD0;
    chip8->memory[dxyn_address + 1] = 0x11;
}

static void test_run_frame_display_wait_quirk_completes_long_non_dxyn_logic_before_dxyn_in_one_frame(void) {
    int n = 100;
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    chip8->quirks.quirk_display_wait = true;
    write_long_non_dxyn_program_then_dxyn(chip8, n);

    Display* display = display_init();
    assert(display != NULL);
    Audio* audio = audio_init();
    assert(audio != NULL);

    bool running = run_frame(chip8, display, audio, 16);
    assert(running == true);
    assert(chip8->PC == PC_START + (n + 1) * 2);
    assert(chip8->sprite_drawn == true);

    audio_destroy(audio);
    display_destroy(display);
    free_chip8(chip8);
}

static void write_program_filling_memory_without_dxyn(Chip8* chip8) {
    int count = (MEM_SIZE - PC_START) / 2;
    for (int i = 0; i < count; i++) {
        chip8->memory[PC_START + i * 2] = 0x60;
        chip8->memory[PC_START + i * 2 + 1] = (uint8_t)i;
    }
}

static void test_run_frame_display_wait_quirk_caps_non_dxyn_instructions_when_no_dxyn_is_found(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    chip8->quirks.quirk_display_wait = true;
    write_program_filling_memory_without_dxyn(chip8);

    Display* display = display_init();
    assert(display != NULL);
    Audio* audio = audio_init();
    assert(audio != NULL);

    bool running = run_frame(chip8, display, audio, 16);
    assert(running == true);
    assert(chip8->PC == PC_START + INSTRUCTIONS_PER_FRAME_DISPLAY_WAIT_CAP * 2);

    audio_destroy(audio);
    display_destroy(display);
    free_chip8(chip8);
}

static void test_run_frame_display_wait_quirk_reaches_same_pc_and_display_with_more_frames(void) {
    Chip8* chip8_default = init_chip8();
    assert(chip8_default != NULL);
    int load_result_default = load_rom(chip8_default, "../roms/ibm_logo.ch8");
    assert(load_result_default == 0);

    Display* display = display_init();
    assert(display != NULL);
    Audio* audio = audio_init();
    assert(audio != NULL);

    int frames_without_quirk = 0;
    while (chip8_default->PC != 0x228 && frames_without_quirk < 1000) {
        bool running = run_frame(chip8_default, display, audio, 16);
        assert(running == true);
        frames_without_quirk++;
    }
    assert(chip8_default->PC == 0x228);

    Chip8* chip8_quirk = init_chip8();
    assert(chip8_quirk != NULL);
    chip8_quirk->quirks.quirk_display_wait = true;
    int load_result_quirk = load_rom(chip8_quirk, "../roms/ibm_logo.ch8");
    assert(load_result_quirk == 0);

    int frames_used = 0;
    int frames_with_draw = 0;
    while (chip8_quirk->PC != 0x228 && frames_used < 1000) {
        bool running = run_frame(chip8_quirk, display, audio, 16);
        assert(running == true);
        frames_used++;

        if (chip8_quirk->sprite_drawn) {
            frames_with_draw++;
        }
    }

    assert(chip8_quirk->PC == 0x228);
    assert(frames_used > frames_without_quirk);
    assert(frames_with_draw >= 6);

    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        assert(chip8_default->display[i] == chip8_quirk->display[i]);
    }

    audio_destroy(audio);
    display_destroy(display);
    free_chip8(chip8_default);
    free_chip8(chip8_quirk);
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
    test_run_frame_without_display_wait_quirk_executes_all_dxyn_in_one_frame();
    test_run_frame_with_display_wait_quirk_executes_at_most_one_dxyn_per_frame();
    test_run_frame_display_wait_quirk_runs_non_dxyn_instructions_beyond_old_budget();
    test_run_frame_display_wait_quirk_completes_long_non_dxyn_logic_before_dxyn_in_one_frame();
    test_run_frame_display_wait_quirk_caps_non_dxyn_instructions_when_no_dxyn_is_found();
    test_run_frame_display_wait_quirk_reaches_same_pc_and_display_with_more_frames();
    test_run_frame_updates_timers_at_real_60hz_rate();

    SDL_Quit();
    printf("All main loop tests passed!\n");
    return 0;
}
