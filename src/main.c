#include <stdio.h>
#include <SDL2/SDL.h>

int main(void) {
    SDL_Init(SDL_INIT_VIDEO);
    printf("CHIP-8 emulator - setup OK");
    return 0;
}