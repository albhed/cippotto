#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>
#include "chip8.h"

void input_handle_event(Chip8* chip8, const SDL_Event* event);

#endif /* INPUT_H */
