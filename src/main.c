#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "mem.h"
#include "err.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    if (argc < 2) {
        printf("Error: No ROM file specified\n");
        return NO_ROM_SELECTED;
    }

    Memory* mem = init_memory();

    if (mem == NULL) {
        printf("Error: Could not initialize memory\n");
        return ERR_MEMORY_ALLOCATION_FAILED;
    }

    int load_rom_result = load_rom(mem, argv[1]);
    if (load_rom_result!= 0) {
        printf("Error: Could not load ROM\n");
        free(mem);
        return load_rom_result;
    }

    free(mem);
    printf("CHIP-8 emulator - setup OK");
    return 0;
}