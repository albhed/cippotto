#include <stdlib.h>
#include "display.h"

Display* display_init(void) {
    Display* display = malloc(sizeof(Display));
    if (display == NULL) {
        return NULL;
    }

    display->window = SDL_CreateWindow(
        "CHIP-8",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        DISPLAY_WIDTH * DISPLAY_SCALE,
        DISPLAY_HEIGHT * DISPLAY_SCALE,
        SDL_WINDOW_SHOWN
    );
    if (display->window == NULL) {
        free(display);
        return NULL;
    }

    display->renderer = SDL_CreateRenderer(display->window, -1, 0);
    if (display->renderer == NULL) {
        SDL_DestroyWindow(display->window);
        free(display);
        return NULL;
    }

    return display;
}

void display_destroy(Display* display) {
    if (display == NULL) {
        return;
    }
    SDL_DestroyRenderer(display->renderer);
    SDL_DestroyWindow(display->window);
    free(display);
}

void display_render(Display* display, const Chip8* chip8) {
    SDL_SetRenderDrawColor(display->renderer, 0, 0, 0, 255);
    SDL_RenderClear(display->renderer);

    SDL_SetRenderDrawColor(display->renderer, 255, 255, 255, 255);
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            if (chip8->display[y * DISPLAY_WIDTH + x]) {
                SDL_Rect pixel = {
                    x * DISPLAY_SCALE,
                    y * DISPLAY_SCALE,
                    DISPLAY_SCALE,
                    DISPLAY_SCALE
                };
                SDL_RenderFillRect(display->renderer, &pixel);
            }
        }
    }

    SDL_RenderPresent(display->renderer);
}

bool display_process_events(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
    }
    return true;
}
