#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "chip8.h"
#include "err.h"

static void print_cpu_state(const Chip8* chip8) {
    printf("PC = 0x%04X\n", chip8->PC);
    for (int i = 0; i < NUM_REGISTERS; i++) {
        printf("V%X = 0x%02X\n", i, chip8->V[i]);
    }
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    if (argc < 2) {
        printf("Error: No ROM file specified\n");
        return NO_ROM_SELECTED;
    }

    Chip8* chip8 = init_chip8();

    if (chip8 == NULL) {
        printf("Error: Could not initialize chip8\n");
        return ERR_MEMORY_ALLOCATION_FAILED;
    }

    int load_rom_result = load_rom(chip8, argv[1]);
    if (load_rom_result != 0) {
        printf("Error: Could not load ROM\n");
        free_chip8(chip8);
        return load_rom_result;
    }

    print_cpu_state(chip8);

    free_chip8(chip8);
    printf("CHIP-8 emulator - setup OK");
    return 0;
}