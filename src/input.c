#include "input.h"

/*
 * Mappatura tastiera fisica (QWERTY) -> tastierino CHIP-8:
 *
 * CHIP-8          Tastiera
 * 1 2 3 C         1 2 3 4
 * 4 5 6 D   -->   Q W E R
 * 7 8 9 E         A S D F
 * A 0 B F         Z X C V
 */
static int scancode_to_chip8_key(SDL_Scancode scancode) {
    switch (scancode) {
        case SDL_SCANCODE_1: return 0x1;
        case SDL_SCANCODE_2: return 0x2;
        case SDL_SCANCODE_3: return 0x3;
        case SDL_SCANCODE_4: return 0xC;
        case SDL_SCANCODE_Q: return 0x4;
        case SDL_SCANCODE_W: return 0x5;
        case SDL_SCANCODE_E: return 0x6;
        case SDL_SCANCODE_R: return 0xD;
        case SDL_SCANCODE_A: return 0x7;
        case SDL_SCANCODE_S: return 0x8;
        case SDL_SCANCODE_D: return 0x9;
        case SDL_SCANCODE_F: return 0xE;
        case SDL_SCANCODE_Z: return 0xA;
        case SDL_SCANCODE_X: return 0x0;
        case SDL_SCANCODE_C: return 0xB;
        case SDL_SCANCODE_V: return 0xF;
        default: return -1;
    }
}

void input_handle_event(Chip8* chip8, const SDL_Event* event) {
    if (chip8 == NULL || event == NULL) {
        return;
    }

    if (event->type != SDL_KEYDOWN && event->type != SDL_KEYUP) {
        return;
    }

    int chip8_key = scancode_to_chip8_key(event->key.keysym.scancode);
    if (chip8_key < 0) {
        return;
    }

    chip8->keys[chip8_key] = (event->type == SDL_KEYDOWN) ? 1 : 0;
}
