#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/chip8.h"
#include "../src/err.h"

extern const uint8_t fontset[80];

void test_init_chip8_pc(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    assert(chip8->PC == PC_START);
    free_chip8(chip8);
}

void test_init_chip8_registers(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    for (int i = 0; i < 16; i++) {
        assert(chip8->V[i] == 0);
    }
    free_chip8(chip8);
}

void test_init_chip8_index_register(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    assert(chip8->I == 0);
    free_chip8(chip8);
}

void test_init_chip8_memory_linked(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    assert(chip8->memory != NULL);
    assert(chip8->memory[FONTSET_START] == 0xF0);
    free_chip8(chip8);
}

void test_mem_size(void) {
    int expected_mem_size = 4096;
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    assert(sizeof(chip8->memory) == expected_mem_size);
}

void test_pc(void) {
    assert(PC_START == 0x200);
}

void test_large_rom(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    char* large_rom_path = "../roms/large.ch8";
    int result = load_rom(chip8, large_rom_path);
    assert(result == ERR_ROM_TOO_LARGE);
    for (int i = PC_START; i < MEM_SIZE; i++) {
        assert(chip8->memory[i] == 0);
    }
    free_chip8(chip8);
}

void test_load_rom_ok(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    char* rom_path = "../roms/ok.ch8";
    int result = load_rom(chip8, rom_path);
    assert(result == 0);
    for (int i = 0; i < 10; i++) {
        printf("chip8->memory[PC_START + %d] = %.2x\n", i, chip8->memory[PC_START + i]);
        assert(chip8->memory[PC_START + i] != 0);
    }
    free_chip8(chip8);
}

void test_font(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    
    for (int i = 0; i < 80; i++) {
        assert(chip8->memory[FONTSET_START + i] == fontset[i]);
    }
    free_chip8(chip8);
}

void test_stack_push_pop(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    for (uint16_t i = 0; i < 16; i++) {
        int push_result = stack_push(chip8, i);
        assert(push_result == 0);
    }

    for (int i = 15; i >= 0; i--) {
        StackPopResult* pop_result = stack_pop(chip8);
        assert(pop_result != NULL);
        assert(pop_result->error == 0);
        assert(pop_result->value == (uint16_t)i);
        free(pop_result);
    }

    StackPopResult* pop_empty_result = stack_pop(chip8);
    assert(pop_empty_result != NULL);
    assert(pop_empty_result->error == ERR_STACK_UNDERFLOW);
    free(pop_empty_result);

    free_chip8(chip8);
}

void print_display(const Chip8* chip8) {
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            printf("%c", chip8->display[y * DISPLAY_WIDTH + x] ? 'X' : '.');
        }
        printf("\n");
    }
}

void test_clear_display(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->display[0 * 64 + 0] = 1;
    chip8->display[31 * 64 + 0] = 1;
    print_display(chip8);

    int clear_result = clear_display(chip8);
    assert(clear_result == 0);
    printf("Display cleared.\n");
    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        assert(chip8->display[i] == 0);
    }

    print_display(chip8);

    free_chip8(chip8);
}

void test_fetch(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->memory[PC_START] = 0x12;
    chip8->memory[PC_START + 1] = 0x34;
    chip8->memory[PC_START + 2] = 0x56;
    chip8->memory[PC_START + 3] = 0x78;


    uint16_t instruction = fetch(chip8);
    assert(instruction == 0x1234);
    assert(chip8->PC == PC_START + 2);

    uint16_t instruction2 = fetch(chip8);
    assert(instruction2 == 0x5678);
    assert(chip8->PC == PC_START + 4);

    free_chip8(chip8);
}

int main(void) {
    printf("Running tests...\n");
    test_init_chip8_pc();
    test_init_chip8_registers();
    test_init_chip8_index_register();
    test_init_chip8_memory_linked();
    test_mem_size();
    test_pc();
    test_large_rom();
    test_load_rom_ok();
    test_font();
    test_stack_push_pop();
    test_clear_display();
    test_fetch();
    printf("All tests passed!\n");
    return 0;
}