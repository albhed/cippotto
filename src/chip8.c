#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "chip8.h"
#include "err.h"

const uint8_t fontset[80] = {
    0xF0,0x90,0x90,0x90,0xF0, // 0
    0x20,0x60,0x20,0x20,0x70, // 1
    0xF0,0x10,0xF0,0x80,0xF0, // 2
    0xF0,0x10,0xF0,0x10,0xF0, // 3
    0x90,0x90,0xF0,0x10,0x10, // 4
    0xF0,0x80,0xF0,0x10,0xF0, // 5
    0xF0,0x80,0xF0,0x90,0xF0, // 6
    0xF0,0x10,0x20,0x40,0x40, // 7
    0xF0,0x90,0xF0,0x90,0xF0, // 8
    0xF0,0x90,0xF0,0x10,0xF0, // 9
    0xF0,0x90,0xF0,0x90,0x90, // A
    0xE0,0x90,0xE0,0x90,0xE0, // B
    0xF0,0x80,0x80,0x80,0xF0, // C
    0xE0,0x90,0x90,0x90,0xE0, // D
    0xF0,0x80,0xF0,0x80,0xF0, // E
    0xF0,0x80,0xF0,0x80,0x80  // F
};

static void seed_random_once(void) {
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
}

Chip8* init_chip8(void) {
    seed_random_once();

    Chip8* chip8 = (Chip8*)malloc(sizeof(Chip8));
    if (chip8 == NULL) {
        printf("Error: Could not allocate memory for Chip8 struct\n");
        return NULL;
    }

    for (size_t i = 0; i < sizeof(chip8->memory); i++) {
        chip8->memory[i] = 0;
    }

    memset(chip8->V, 0, sizeof(chip8->V));
    chip8->I = 0;
    chip8->PC = PC_START;
    chip8->SP = 0;
    memcpy(chip8->memory, fontset, 80);

    memset(chip8->display, 0, sizeof(chip8->display));
    memset(chip8->keys, 0, sizeof(chip8->keys));

    chip8->delay_timer = 0;
    chip8->sound_timer = 0;
    chip8->timer_accumulator_us = 0;

    return chip8;
}

int load_rom(Chip8* chip, const char* filepath) {
    FILE *ptr = fopen(filepath, "rb");

    if (ptr == NULL) {
        printf("Error: Could not open file %s\n", filepath);
        return ERR_FILE_NOT_FOUND;
    }

    fseek(ptr, 0, SEEK_END);
    long file_size = ftell(ptr);
    
    if (file_size < 0) {
        printf("Error: Could not determine file size for %s\n", filepath);
        fclose(ptr);
        return ERR_READING_ROM;
    }

    if (file_size > sizeof(chip->memory) - PC_START) {
        printf("Error: ROM size exceeds available memory\n");
        fclose(ptr);
        return ERR_ROM_TOO_LARGE;
    }
    fseek(ptr, 0, SEEK_SET);

    size_t byte_read = fread(&chip->memory[PC_START], sizeof(uint8_t), sizeof(chip->memory) - PC_START, ptr);
    if (byte_read != file_size) {
        printf("Error: Could not read the entire ROM file\n");
        fclose(ptr);
        return ERR_ROM_EOF;
    }
    fclose(ptr);
    return 0;
}

void free_chip8(Chip8* chip8) {
    if (chip8 == NULL) {
        return;
    }
    free(chip8);
}

int stack_push(Chip8* chip, uint16_t value) {
    if (chip->SP >= 16) {
        printf("Error: Stack overflow\n");
        return ERR_STACK_UNDERFLOW;
    }
    chip->stack[chip->SP] = value;
    chip->SP++;
    return 0;
}

StackPopResult* stack_pop(Chip8* chip) {
    StackPopResult* result = (StackPopResult*)malloc(sizeof(StackPopResult));
    if (result == NULL) {
        printf("Error: Could not allocate memory for StackPopResult\n");
        return NULL;
    }

    if (chip->SP == 0) {
        result->error = ERR_STACK_UNDERFLOW;
        return result;
    }

    chip->SP--;
    result->value = chip->stack[chip->SP];
    result->error = 0;
    return result;
}

int clear_display(Chip8* chip) {
    memset(chip->display, 0, sizeof(chip->display));
    return 0;
}

void update_timers(Chip8* chip, uint32_t elapsed_ms) {
    chip->timer_accumulator_us += elapsed_ms * 1000u;

    while (chip->timer_accumulator_us >= TIMER_INTERVAL_US) {
        chip->timer_accumulator_us -= TIMER_INTERVAL_US;

        if (chip->delay_timer > 0) {
            chip->delay_timer--;
        }
        if (chip->sound_timer > 0) {
            chip->sound_timer--;
        }
    }
}

uint16_t fetch(Chip8* chip) {
    uint16_t opcode = (chip->memory[chip->PC] << 8) | chip->memory[chip->PC + 1];
    chip->PC += 2;
    return opcode;
}

DecodedInstruction decode(uint16_t opcode) {
    DecodedInstruction instruction;
    instruction.nibble = (opcode & 0xF000) >> 12;
    instruction.x = (opcode & 0x0F00) >> 8;
    instruction.y = (opcode & 0x00F0) >> 4;
    instruction.n = opcode & 0x000F;
    instruction.nn = opcode & 0x00FF;
    instruction.nnn = opcode & 0x0FFF;
    return instruction;
}

int execute(Chip8* chip, DecodedInstruction instruction) {
    switch (instruction.nibble) {
        case 0x0:
            switch(instruction.nn) {
                case 0xE0:
                    return clear_display(chip);
                case 0xEE: {
                    StackPopResult* pop_result = stack_pop(chip);
                    int pop_error = pop_result->error;
                    if (pop_error != 0) {
                        free(pop_result);
                        return pop_error;
                    }
                    chip->PC = pop_result->value;
                    free(pop_result);
                    break;
                }
                default:
                    printf("Error: Unimplemented opcode: 0x%X\n", instruction.nibble);
                    return UNIMPLEMENTED_INSTRUCTION;
            }
            break;
        case 0x1:
            chip->PC = instruction.nnn;
            break;
        case 0x2:
            int push_result = stack_push(chip, chip->PC);
            if (push_result != 0) {
                return push_result;
            }
            chip->PC = instruction.nnn;
            break;
        case 0x3:
            if (chip->V[instruction.x] == instruction.nn) {
                chip->PC += 2;
            }
            break;
        case 0x4:
            if (chip->V[instruction.x] != instruction.nn) {
                chip->PC += 2;
            }
            break;
        case 0x5:
            if (chip->V[instruction.x] == chip->V[instruction.y]) {
                chip->PC += 2;
            }
            break;
        case 0x6:
            chip->V[instruction.x] = instruction.nn;
            break;
        case 0x7:
            chip->V[instruction.x] += instruction.nn;
            break;
        case 0x9:
            if (chip->V[instruction.x] != chip->V[instruction.y]) {
                chip->PC += 2;
            }
            break;
        case 0x8: {
            uint8_t vx = chip->V[instruction.x];
            uint8_t vy = chip->V[instruction.y];
            switch (instruction.n) {
                case 0x0:
                    chip->V[instruction.x] = vy;
                    break;
                case 0x1:
                    chip->V[instruction.x] = vx | vy;
                    break;
                case 0x2:
                    chip->V[instruction.x] = vx & vy;
                    break;
                case 0x3:
                    chip->V[instruction.x] = vx ^ vy;
                    break;
                case 0x4: {
                    uint16_t sum = (uint16_t)vx + (uint16_t)vy;
                    chip->V[instruction.x] = (uint8_t)sum;
                    chip->V[0xF] = (sum > 0xFF) ? 1 : 0;
                    break;
                }
                case 0x5:
                    chip->V[instruction.x] = (uint8_t)(vx - vy);
                    chip->V[0xF] = (vx >= vy) ? 1 : 0;
                    break;
                case 0x6:
                    chip->V[instruction.x] = vx >> 1;
                    chip->V[0xF] = vx & 0x1;
                    break;
                case 0x7:
                    chip->V[instruction.x] = (uint8_t)(vy - vx);
                    chip->V[0xF] = (vy >= vx) ? 1 : 0;
                    break;
                case 0xE:
                    chip->V[instruction.x] = (uint8_t)(vx << 1);
                    chip->V[0xF] = (vx & 0x80) ? 1 : 0;
                    break;
                default:
                    printf("Error: Unimplemented opcode: 0x%X\n", instruction.nibble);
                    return UNIMPLEMENTED_INSTRUCTION;
            }
            break;
        }
        case 0xA:
            chip->I = instruction.nnn;
            break;
        case 0xB:
            chip->PC = instruction.nnn + chip->V[0x0];
            break;
        case 0xC:
            chip->V[instruction.x] = (uint8_t)(rand() & instruction.nn);
            break;
        case 0xD: {
            uint8_t origin_x = chip->V[instruction.x];
            uint8_t origin_y = chip->V[instruction.y];
            uint8_t collision = 0;

            for (int row = 0; row < instruction.n; row++) {
                if (chip->I + row >= MEM_SIZE) {
                    continue;
                }
                uint8_t sprite_byte = chip->memory[chip->I + row];
                int pixel_y = origin_y + row;

                if (pixel_y >= DISPLAY_HEIGHT) {
                    continue;
                }
                for (int col = 0; col < 8; col++) {
                    int pixel_x = origin_x + col;
                    if (pixel_x >= DISPLAY_WIDTH) {
                        continue;
                    }
                    uint8_t sprite_pixel = (sprite_byte >> (7 - col)) & 0x1;
                    if (sprite_pixel == 0) {
                        continue;
                    }
                    int display_index = pixel_y * DISPLAY_WIDTH + pixel_x;
                    if (chip->display[display_index]) {
                        collision = 1;
                    }
                    chip->display[display_index] ^= 1;
                }
            }

            chip->V[0xF] = collision;
            break;
        }
        case 0xE: {
            uint8_t vx = chip->V[instruction.x];
            uint8_t key_pressed = (vx < NUM_KEYS) && chip->keys[vx];
            switch (instruction.nn) {
                case 0x9E:
                    if (key_pressed) {
                        chip->PC += 2;
                    }
                    break;
                case 0xA1:
                    if (!key_pressed) {
                        chip->PC += 2;
                    }
                    break;
                default:
                    printf("Error: Unimplemented opcode: 0x%X\n", instruction.nibble);
                    return UNIMPLEMENTED_INSTRUCTION;
            }
            break;
        }
        case 0xF:
            switch (instruction.nn) {
                case 0x0A: {
                    int pressed_key = -1;
                    for (int i = 0; i < NUM_KEYS; i++) {
                        if (chip->keys[i]) {
                            pressed_key = i;
                            break;
                        }
                    }
                    if (pressed_key < 0) {
                        chip->PC -= 2;
                    } else {
                        chip->V[instruction.x] = (uint8_t)pressed_key;
                    }
                    break;
                }
                case 0x07:
                    chip->V[instruction.x] = chip->delay_timer;
                    break;
                case 0x15:
                    chip->delay_timer = chip->V[instruction.x];
                    break;
                case 0x18:
                    chip->sound_timer = chip->V[instruction.x];
                    break;
                case 0x1E:
                    chip->I = (uint16_t)(chip->I + chip->V[instruction.x]);
                    break;
                case 0x29:
                    chip->I = FONTSET_START + (uint16_t)(chip->V[instruction.x] * FONT_CHAR_SIZE);
                    break;
                case 0x33: {
                    uint8_t vx = chip->V[instruction.x];
                    uint8_t hundreds = vx / 100;
                    uint8_t tens = (vx / 10) % 10;
                    uint8_t ones = vx % 10;

                    if (chip->I + 2 < MEM_SIZE) {
                        chip->memory[chip->I] = hundreds;
                        chip->memory[chip->I + 1] = tens;
                        chip->memory[chip->I + 2] = ones;
                    }
                    break;
                }
                case 0x55:
                    for (int i = 0; i <= instruction.x; i++) {
                        if (chip->I + i >= MEM_SIZE) {
                            break;
                        }
                        chip->memory[chip->I + i] = chip->V[i];
                    }
                    break;
                case 0x65:
                    for (int i = 0; i <= instruction.x; i++) {
                        if (chip->I + i >= MEM_SIZE) {
                            break;
                        }
                        chip->V[i] = chip->memory[chip->I + i];
                    }
                    break;
                default:
                    printf("Error: Unimplemented opcode: 0x%X\n", instruction.nibble);
                    return UNIMPLEMENTED_INSTRUCTION;
            }
            break;
        default:
            printf("Error: Unimplemented opcode: 0x%X\n", instruction.nibble);
            return UNIMPLEMENTED_INSTRUCTION;
    }
    return 0;
}