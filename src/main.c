#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "chip8.h"
#include "display.h"
#include "audio.h"
#include "loop.h"
#include "err.h"

#define TARGET_FRAME_MS 16

static void print_cpu_state(const Chip8* chip8) {
    printf("PC = 0x%04X\n", chip8->PC);
    for (int i = 0; i < NUM_REGISTERS; i++) {
        printf("V%X = 0x%02X\n", i, chip8->V[i]);
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("Error: Could not initialize SDL: %s\n", SDL_GetError());
        return ERR_SDL_INIT_FAILED;
    }

    if (argc < 2) {
        printf("Error: No ROM file specified\n");
        SDL_Quit();
        return NO_ROM_SELECTED;
    }

    Chip8* chip8 = init_chip8();

    if (chip8 == NULL) {
        printf("Error: Could not initialize chip8\n");
        SDL_Quit();
        return ERR_MEMORY_ALLOCATION_FAILED;
    }

    int load_rom_result = load_rom(chip8, argv[1]);
    if (load_rom_result != 0) {
        printf("Error: Could not load ROM\n");
        free_chip8(chip8);
        SDL_Quit();
        return load_rom_result;
    }

    print_cpu_state(chip8);

    Display* display = display_init();
    if (display == NULL) {
        printf("Error: Could not initialize display\n");
        free_chip8(chip8);
        SDL_Quit();
        return ERR_DISPLAY_INIT_FAILED;
    }

    Audio* audio = audio_init();
    if (audio == NULL) {
        printf("Error: Could not initialize audio\n");
        display_destroy(display);
        free_chip8(chip8);
        SDL_Quit();
        return ERR_AUDIO_INIT_FAILED;
    }

    bool running = true;
    Uint32 last_timer_update = SDL_GetTicks();
    Uint32 last_frame = SDL_GetTicks();

    while (running) {
        Uint32 now = SDL_GetTicks();
        uint32_t elapsed_ms = now - last_timer_update;
        last_timer_update = now;

        running = run_frame(chip8, display, audio, elapsed_ms);

        Uint32 frame_time = SDL_GetTicks() - last_frame;
        if (frame_time < TARGET_FRAME_MS) {
            SDL_Delay(TARGET_FRAME_MS - frame_time);
        }
        last_frame = SDL_GetTicks();
    }

    audio_destroy(audio);
    display_destroy(display);
    free_chip8(chip8);
    SDL_Quit();
    printf("CHIP-8 emulator - shutting down\n");
    return 0;
}