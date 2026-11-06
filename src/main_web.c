#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <SDL2/SDL.h>
#include "chip8.h"
#include "display.h"
#include "audio.h"
#include "loop.h"
#include "quirks.h"
#include "err.h"

typedef struct {
    Chip8* chip8;
    Display* display;
    Audio* audio;
    Uint32 last_timer_update;
} WebLoopContext;

static void web_loop_callback(void* arg) {
    WebLoopContext* ctx = (WebLoopContext*)arg;

    Uint32 now = SDL_GetTicks();
    uint32_t elapsed_ms = now - ctx->last_timer_update;
    ctx->last_timer_update = now;

    bool running = run_frame(ctx->chip8, ctx->display, ctx->audio, elapsed_ms);

    if (!running) {
        emscripten_cancel_main_loop();
        audio_destroy(ctx->audio);
        display_destroy(ctx->display);
        free_chip8(ctx->chip8);
        free(ctx);
        SDL_Quit();
    }
}

int main(int argc, char* argv[]) {
    if (quirk_help_requested(argc, argv)) {
        printf("%s", quirk_help_text());
        return 0;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("Error: Could not initialize SDL: %s\n", SDL_GetError());
        return ERR_SDL_INIT_FAILED;
    }

    const char* rom_path = NULL;
    QuirkConfig quirks = parse_quirk_args(argc, argv, &rom_path);

    if (rom_path == NULL) {
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

    chip8->quirks = quirks;

    int load_rom_result = load_rom(chip8, rom_path);
    if (load_rom_result != 0) {
        printf("Error: Could not load ROM\n");
        free_chip8(chip8);
        SDL_Quit();
        return load_rom_result;
    }

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

    WebLoopContext* ctx = malloc(sizeof(WebLoopContext));
    if (ctx == NULL) {
        printf("Error: Could not allocate loop context\n");
        audio_destroy(audio);
        display_destroy(display);
        free_chip8(chip8);
        SDL_Quit();
        return ERR_MEMORY_ALLOCATION_FAILED;
    }

    ctx->chip8 = chip8;
    ctx->display = display;
    ctx->audio = audio;
    ctx->last_timer_update = SDL_GetTicks();

    emscripten_set_main_loop_arg(web_loop_callback, ctx, 0, 1);

    return 0;
}
