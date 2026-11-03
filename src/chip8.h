#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define MEM_SIZE 4096
#define PC_START 0x200
#define FONTSET_START 0x000
#define NUM_REGISTERS 16
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

typedef struct {
    uint8_t memory[MEM_SIZE];
    uint8_t V[NUM_REGISTERS];
    uint16_t I;
    uint16_t PC;
    uint16_t stack[16];
    uint8_t SP;
    uint8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
} Chip8;

typedef struct {
    uint16_t value;
    int error;
} StackPopResult;

typedef struct {
    uint8_t nibble;
    uint8_t x;
    uint8_t y;
    uint8_t n;
    uint8_t nn;
    uint16_t nnn;
} DecodedInstruction;

Chip8* init_chip8(void);
void free_chip8(Chip8* chip);
int load_rom(Chip8* chip, const char* filepath);

int stack_push(Chip8* chip, uint16_t value);
StackPopResult* stack_pop(Chip8* chip);

int clear_display(Chip8* chip);
uint16_t fetch(Chip8* chip);
DecodedInstruction decode(uint16_t opcode);

#endif /* CHIP8_H */