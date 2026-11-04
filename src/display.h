#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "chip8.h"

#define DISPLAY_SCALE 10

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
} Display;

Display* display_init(void);
void display_destroy(Display* display);
void display_render(Display* display, const Chip8* chip8);
bool display_process_events(void);

#endif /* DISPLAY_H */
