#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

Chip8* init_chip8(void) {
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