#include <assert.h>
#include <stdbool.h>
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

void test_init_chip8_timers(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    assert(chip8->delay_timer == 0);
    assert(chip8->sound_timer == 0);
    free_chip8(chip8);
}

void test_init_chip8_keys(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    for (int i = 0; i < NUM_KEYS; i++) {
        assert(chip8->keys[i] == 0);
    }
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

void test_update_timers_decrements_when_above_zero(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->delay_timer = 5;
    chip8->sound_timer = 5;

    update_timers(chip8, 17);

    assert(chip8->delay_timer == 4);
    assert(chip8->sound_timer == 4);

    free_chip8(chip8);
}

void test_update_timers_never_goes_below_zero(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->delay_timer = 0;
    chip8->sound_timer = 0;

    update_timers(chip8, 1000);

    assert(chip8->delay_timer == 0);
    assert(chip8->sound_timer == 0);

    free_chip8(chip8);
}

void test_update_timers_small_elapsed_does_not_decrement_yet(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->delay_timer = 10;

    update_timers(chip8, 1);

    assert(chip8->delay_timer == 10);

    free_chip8(chip8);
}

void test_update_timers_simulated_half_second_reaches_zero(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->delay_timer = 30;

    update_timers(chip8, 500);

    assert(chip8->delay_timer == 0);

    free_chip8(chip8);
}

void test_update_timers_multiple_small_calls_reach_zero(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->delay_timer = 30;

    for (int i = 0; i < 30; i++) {
        update_timers(chip8, 17);
    }

    assert(chip8->delay_timer == 0);

    free_chip8(chip8);
}

void test_update_timers_handles_multiple_ticks_in_one_call(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->delay_timer = 10;

    update_timers(chip8, 100);

    assert(chip8->delay_timer == 4);

    free_chip8(chip8);
}

void test_update_timers_independent_of_cpu_cycles(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->delay_timer = 10;
    chip8->memory[PC_START] = 0x00;
    chip8->memory[PC_START + 1] = 0xE0;

    for (int i = 0; i < 1000; i++) {
        uint16_t opcode = fetch(chip8);
        DecodedInstruction instruction = decode(opcode);
        execute(chip8, instruction);
        chip8->PC = PC_START;
    }

    assert(chip8->delay_timer == 10);

    update_timers(chip8, 500);
    assert(chip8->delay_timer == 0);

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

void test_decode(void) {
    uint16_t opcode = 0xA2F0;
    DecodedInstruction decoded = decode(opcode);

    assert(decoded.nibble == 0xA);
    assert(decoded.x == 0x2);
    assert(decoded.y == 0xF);
    assert(decoded.n == 0x0);
    assert(decoded.nn == 0xF0);
    assert(decoded.nnn == 0x2F0);

    DecodedInstruction decoded2 = decode(0x6A3F);

    assert(decoded2.nibble == 0x6);
    assert(decoded2.x == 0xA);
    assert(decoded2.y == 0x3);
    assert(decoded2.n == 0xF);
    assert(decoded2.nn == 0x3F);
    assert(decoded2.nnn == 0xA3F);

    DecodedInstruction decoded3 = decode(0xFFFF);

    assert(decoded3.nibble == 0xF);
    assert(decoded3.x == 0xF);
    assert(decoded3.y == 0xF);
    assert(decoded3.n == 0xF);
    assert(decoded3.nn == 0xFF);
    assert(decoded3.nnn == 0xFFF);
}

void test_execute_00E0(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        chip8->display[i] = 1;
    }

    DecodedInstruction instruction = decode(0x00E0);

    int result = execute(chip8, instruction);
    assert(result == 0);

    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        assert(chip8->display[i] == 0);
    }

    free_chip8(chip8);
}

void test_execute_1NNN(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    DecodedInstruction instruction = decode(0x1234);

    int result = execute(chip8, instruction);
    assert(result == 0);

    assert(chip8->PC == 0x234);

    free_chip8(chip8);
}

void test_execute_loop_1NNN(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->memory[PC_START] = 0x12;
    chip8->memory[PC_START + 1] = 0x00;

   
    for (int i = 0; i < 10; i++) {
        int fetched = fetch(chip8);
        assert(fetched == 0x1200);
        assert(chip8->PC == PC_START + 2);
        DecodedInstruction instruction = decode(fetched);
        int result = execute(chip8, instruction);
        assert(result == 0);
        assert(chip8->PC == 0x200);
    }

    free_chip8(chip8);
}

void test_execute_6XNN(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->memory[PC_START] = 0x6A;
    chip8->memory[PC_START + 1] = 0x3F;

    int fetched = fetch(chip8);
    assert(fetched == 0x6A3F);
    assert(chip8->PC == PC_START + 2);

    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);

    for(int i = 0; i < 16; i++) {
        if (i == 0xA) {
            assert(chip8->V[i] == 0x3F);
        } else {
            assert(chip8->V[i] == 0);
        }
    }

    free_chip8(chip8);
}

void test_execute_7XNN(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->memory[PC_START] = 0x6A;
    chip8->memory[PC_START + 1] = 0x05;
    chip8->memory[PC_START + 2] = 0x7A;
    chip8->memory[PC_START + 3] = 0x03;
    chip8->V[0xF] = 0xAA;

    int fetched = fetch(chip8);
    assert(fetched == 0x6A05);
    assert(chip8->PC == PC_START + 2);

    DecodedInstruction instruction1 = decode(fetched);
    int result1 = execute(chip8, instruction1);
    assert(result1 == 0);
    assert(chip8->V[0xA] == 0x05);

    fetched = fetch(chip8);
    assert(fetched == 0x7A03);
    assert(chip8->PC == PC_START + 4);

    DecodedInstruction instruction2 = decode(fetched);
    int result2 = execute(chip8, instruction2);
    assert(result2 == 0);
    assert(chip8->V[0xA] == 0x08);
    assert(chip8->V[0xF] == 0xAA);

    free_chip8(chip8);
}

void test_execute_7XNN_overflow(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    chip8->V[0xF] = 0xAA;

    DecodedInstruction instruction1 = decode(0x6AFA);
    int result1 = execute(chip8, instruction1);
    assert(result1 == 0);
    assert(chip8->V[0xA] == 0xFA);

    DecodedInstruction instruction2 = decode(0x7A0A);
    int result2 = execute(chip8, instruction2);
    assert(result2 == 0);
    assert(chip8->V[0xA] == 0x04);
    assert(chip8->V[0xF] == 0xAA);

    free_chip8(chip8);
}

void test_execute_ANNN(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    DecodedInstruction instruction = decode(0xA2F0);

    int result = execute(chip8, instruction);
    assert(result == 0);

    assert(chip8->I == 0x2F0);

    free_chip8(chip8);
}

void test_execute_2NNN(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    chip8->memory[PC_START] = 0x22;
    chip8->memory[PC_START + 1] = 0xF0;

    int fetched = fetch(chip8);
    assert(fetched == 0x22F0);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);

    assert(chip8->SP == 1);
    StackPopResult* pop_result = stack_pop(chip8);
    assert(pop_result != NULL);
    assert(pop_result->error == 0);
    assert(pop_result->value == PC_START + 2);
    free(pop_result);

    free_chip8(chip8);
}

void test_execute_00EE(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->memory[PC_START] = 0x22;
    chip8->memory[PC_START + 1] = 0xF0;
    chip8->memory[0x02F0] = 0x00;
    chip8->memory[0x02F1] = 0xEE;

    int fetched = fetch(chip8);

    DecodedInstruction instruction1 = decode(fetched);
    int result1 = execute(chip8, instruction1);
    assert(result1 == 0);
    assert(chip8->SP == 1);

    fetched = fetch(chip8);
    DecodedInstruction instruction2 = decode(fetched);

    int result2 = execute(chip8, instruction2);
    assert(result2 == 0);
    assert(chip8->SP == 0);
    assert(chip8->PC == 0x202);

    free_chip8(chip8);
}

void test_execute_00EE_empty_stack(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);
    assert(chip8->SP == 0);

    DecodedInstruction instruction = decode(0x00EE);
    int result = execute(chip8, instruction);
    assert(result == ERR_STACK_UNDERFLOW);
    assert(chip8->SP == 0);

    free_chip8(chip8);
}

void test_execute_nested_subroutines(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->memory[PC_START] = 0x22;
    chip8->memory[PC_START + 1] = 0xF0;
    chip8->memory[0x02F0] = 0x24;
    chip8->memory[0x02F1] = 0x00;
    chip8->memory[0x0400] = 0x00;
    chip8->memory[0x0401] = 0xEE;
    chip8->memory[0x02F2] = 0x00;
    chip8->memory[0x02F3] = 0xEE;

    int fetched = fetch(chip8);
    assert(fetched == 0x22F0);

    DecodedInstruction instruction1 = decode(fetched);
    int result1 = execute(chip8, instruction1);
    assert(result1 == 0);
    assert(chip8->SP == 1);

    fetched = fetch(chip8);
    assert(fetched == 0x2400);

    DecodedInstruction instruction2 = decode(fetched);
    int result2 = execute(chip8, instruction2);
    assert(result2 == 0);
    assert(chip8->SP == 2);

    fetched = fetch(chip8);
    assert(fetched == 0x00EE);

    DecodedInstruction instruction3 = decode(fetched);
    int result3 = execute(chip8, instruction3);
    assert(chip8->stack[chip8->SP] == 0x02F2);
    assert(chip8->PC == 0x02F2);
    assert(result3 == 0);
    assert(chip8->SP == 1);

    fetched = fetch(chip8);
    assert(fetched == 0x00EE);

    DecodedInstruction instruction4 = decode(fetched);
    int result4 = execute(chip8, instruction4);
    assert(result4 == 0);
    assert(chip8->SP == 0);
    assert(chip8->PC == 0x202);

    free_chip8(chip8);
}

void test_execute_3XNN_true(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->memory[PC_START] = 0x3A;
    chip8->memory[PC_START + 1] = 0x05;

    int fetched = fetch(chip8);
    assert(fetched == 0x3A05);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->PC == PC_START + 4);

    free_chip8(chip8);
}

void test_execute_3XNN_false(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->memory[PC_START] = 0x3A;
    chip8->memory[PC_START + 1] = 0x06;

    int fetched = fetch(chip8);
    assert(fetched == 0x3A06);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->PC == PC_START + 2);

    free_chip8(chip8);
}

void test_execute_4XNN_true(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->memory[PC_START] = 0x4A;
    chip8->memory[PC_START + 1] = 0x05;

    int fetched = fetch(chip8);
    assert(fetched == 0x4A05);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->PC == PC_START + 2);

    free_chip8(chip8);
}

void test_execute_4XNN_false(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->memory[PC_START] = 0x4A;
    chip8->memory[PC_START + 1] = 0x06;

    int fetched = fetch(chip8);
    assert(fetched == 0x4A06);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->PC == PC_START + 4);

    free_chip8(chip8);
}

void test_execute_5XY0_true(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->V[0xB] = 0x05;
    chip8->memory[PC_START] = 0x5A;
    chip8->memory[PC_START + 1] = 0xB0;

    int fetched = fetch(chip8);
    assert(fetched == 0x5AB0);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->PC == PC_START + 4);

    free_chip8(chip8);
}

void test_execute_5XY0_false(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->V[0xB] = 0x06;
    chip8->memory[PC_START] = 0x5A;
    chip8->memory[PC_START + 1] = 0xB0;

    int fetched = fetch(chip8);
    assert(fetched == 0x5AB0);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->PC == PC_START + 2);

    free_chip8(chip8);
}

void test_execute_9XY0_true(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->V[0xB] = 0x06;
    chip8->memory[PC_START] = 0x9A;
    chip8->memory[PC_START + 1] = 0xB0;

    int fetched = fetch(chip8);
    assert(fetched == 0x9AB0);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->PC == PC_START + 4);

    free_chip8(chip8);
}

void test_execute_9XY0_false(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->V[0xB] = 0x05;
    chip8->memory[PC_START] = 0x9A;
    chip8->memory[PC_START + 1] = 0xB0;

    int fetched = fetch(chip8);
    assert(fetched == 0x9AB0);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->PC == PC_START + 2);

    free_chip8(chip8);
}

void test_execute_3XNN_program_skips_forbidden_instruction(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->memory[PC_START] = 0x3A;
    chip8->memory[PC_START + 1] = 0x05;
    chip8->memory[PC_START + 2] = 0x6A;
    chip8->memory[PC_START + 3] = 0xFF;
    chip8->memory[PC_START + 4] = 0x6B;
    chip8->memory[PC_START + 5] = 0x01;

    int fetched = fetch(chip8);
    assert(fetched == 0x3A05);
    DecodedInstruction instruction1 = decode(fetched);
    int result1 = execute(chip8, instruction1);
    assert(result1 == 0);
    assert(chip8->PC == PC_START + 4);

    fetched = fetch(chip8);
    assert(fetched == 0x6B01);
    assert(chip8->PC == PC_START + 6);
    DecodedInstruction instruction2 = decode(fetched);
    int result2 = execute(chip8, instruction2);
    assert(result2 == 0);

    assert(chip8->V[0xA] == 0x05);
    assert(chip8->V[0xB] == 0x01);

    free_chip8(chip8);
}

void test_execute_3XNN_program_executes_forbidden_instruction_when_false(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->memory[PC_START] = 0x3A;
    chip8->memory[PC_START + 1] = 0x06;
    chip8->memory[PC_START + 2] = 0x6A;
    chip8->memory[PC_START + 3] = 0xFF;

    int fetched = fetch(chip8);
    assert(fetched == 0x3A06);
    DecodedInstruction instruction1 = decode(fetched);
    int result1 = execute(chip8, instruction1);
    assert(result1 == 0);
    assert(chip8->PC == PC_START + 2);

    fetched = fetch(chip8);
    assert(fetched == 0x6AFF);
    assert(chip8->PC == PC_START + 4);
    DecodedInstruction instruction2 = decode(fetched);
    int result2 = execute(chip8, instruction2);
    assert(result2 == 0);

    assert(chip8->V[0xA] == 0xFF);

    free_chip8(chip8);
}

void test_execute_4XNN_program_skips_forbidden_instruction(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->memory[PC_START] = 0x4A;
    chip8->memory[PC_START + 1] = 0x06;
    chip8->memory[PC_START + 2] = 0x6A;
    chip8->memory[PC_START + 3] = 0xFF;
    chip8->memory[PC_START + 4] = 0x6B;
    chip8->memory[PC_START + 5] = 0x01;

    int fetched = fetch(chip8);
    assert(fetched == 0x4A06);
    DecodedInstruction instruction1 = decode(fetched);
    int result1 = execute(chip8, instruction1);
    assert(result1 == 0);
    assert(chip8->PC == PC_START + 4);

    fetched = fetch(chip8);
    assert(fetched == 0x6B01);
    assert(chip8->PC == PC_START + 6);
    DecodedInstruction instruction2 = decode(fetched);
    int result2 = execute(chip8, instruction2);
    assert(result2 == 0);

    assert(chip8->V[0xA] == 0x05);
    assert(chip8->V[0xB] == 0x01);

    free_chip8(chip8);
}

void test_execute_4XNN_program_executes_forbidden_instruction_when_false(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->memory[PC_START] = 0x4A;
    chip8->memory[PC_START + 1] = 0x05;
    chip8->memory[PC_START + 2] = 0x6A;
    chip8->memory[PC_START + 3] = 0xFF;

    int fetched = fetch(chip8);
    assert(fetched == 0x4A05);
    DecodedInstruction instruction1 = decode(fetched);
    int result1 = execute(chip8, instruction1);
    assert(result1 == 0);
    assert(chip8->PC == PC_START + 2);

    fetched = fetch(chip8);
    assert(fetched == 0x6AFF);
    assert(chip8->PC == PC_START + 4);
    DecodedInstruction instruction2 = decode(fetched);
    int result2 = execute(chip8, instruction2);
    assert(result2 == 0);

    assert(chip8->V[0xA] == 0xFF);

    free_chip8(chip8);
}

void test_execute_5XY0_program_skips_forbidden_instruction(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->V[0xB] = 0x05;
    chip8->memory[PC_START] = 0x5A;
    chip8->memory[PC_START + 1] = 0xB0;
    chip8->memory[PC_START + 2] = 0x6A;
    chip8->memory[PC_START + 3] = 0xFF;
    chip8->memory[PC_START + 4] = 0x6C;
    chip8->memory[PC_START + 5] = 0x01;

    int fetched = fetch(chip8);
    assert(fetched == 0x5AB0);
    DecodedInstruction instruction1 = decode(fetched);
    int result1 = execute(chip8, instruction1);
    assert(result1 == 0);
    assert(chip8->PC == PC_START + 4);

    fetched = fetch(chip8);
    assert(fetched == 0x6C01);
    assert(chip8->PC == PC_START + 6);
    DecodedInstruction instruction2 = decode(fetched);
    int result2 = execute(chip8, instruction2);
    assert(result2 == 0);

    assert(chip8->V[0xA] == 0x05);
    assert(chip8->V[0xC] == 0x01);

    free_chip8(chip8);
}

void test_execute_5XY0_program_executes_forbidden_instruction_when_false(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->V[0xB] = 0x06;
    chip8->memory[PC_START] = 0x5A;
    chip8->memory[PC_START + 1] = 0xB0;
    chip8->memory[PC_START + 2] = 0x6A;
    chip8->memory[PC_START + 3] = 0xFF;

    int fetched = fetch(chip8);
    assert(fetched == 0x5AB0);
    DecodedInstruction instruction1 = decode(fetched);
    int result1 = execute(chip8, instruction1);
    assert(result1 == 0);
    assert(chip8->PC == PC_START + 2);

    fetched = fetch(chip8);
    assert(fetched == 0x6AFF);
    assert(chip8->PC == PC_START + 4);
    DecodedInstruction instruction2 = decode(fetched);
    int result2 = execute(chip8, instruction2);
    assert(result2 == 0);

    assert(chip8->V[0xA] == 0xFF);

    free_chip8(chip8);
}

void test_execute_9XY0_program_skips_forbidden_instruction(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->V[0xB] = 0x06;
    chip8->memory[PC_START] = 0x9A;
    chip8->memory[PC_START + 1] = 0xB0;
    chip8->memory[PC_START + 2] = 0x6A;
    chip8->memory[PC_START + 3] = 0xFF;
    chip8->memory[PC_START + 4] = 0x6C;
    chip8->memory[PC_START + 5] = 0x01;

    int fetched = fetch(chip8);
    assert(fetched == 0x9AB0);
    DecodedInstruction instruction1 = decode(fetched);
    int result1 = execute(chip8, instruction1);
    assert(result1 == 0);
    assert(chip8->PC == PC_START + 4);

    fetched = fetch(chip8);
    assert(fetched == 0x6C01);
    assert(chip8->PC == PC_START + 6);
    DecodedInstruction instruction2 = decode(fetched);
    int result2 = execute(chip8, instruction2);
    assert(result2 == 0);

    assert(chip8->V[0xA] == 0x05);
    assert(chip8->V[0xC] == 0x01);

    free_chip8(chip8);
}

void test_execute_9XY0_program_executes_forbidden_instruction_when_false(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->V[0xB] = 0x05;
    chip8->memory[PC_START] = 0x9A;
    chip8->memory[PC_START + 1] = 0xB0;
    chip8->memory[PC_START + 2] = 0x6A;
    chip8->memory[PC_START + 3] = 0xFF;

    int fetched = fetch(chip8);
    assert(fetched == 0x9AB0);
    DecodedInstruction instruction1 = decode(fetched);
    int result1 = execute(chip8, instruction1);
    assert(result1 == 0);
    assert(chip8->PC == PC_START + 2);

    fetched = fetch(chip8);
    assert(fetched == 0x6AFF);
    assert(chip8->PC == PC_START + 4);
    DecodedInstruction instruction2 = decode(fetched);
    int result2 = execute(chip8, instruction2);
    assert(result2 == 0);

    assert(chip8->V[0xA] == 0xFF);

    free_chip8(chip8);
}

void test_execute_8XY0(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->V[0xB] = 0x09;

    DecodedInstruction instruction = decode(0x8AB0);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xA] == 0x09);
    assert(chip8->V[0xB] == 0x09);

    free_chip8(chip8);
}

void test_execute_8XY1(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x0F;
    chip8->V[0xB] = 0xF0;

    DecodedInstruction instruction = decode(0x8AB1);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xA] == 0xFF);

    free_chip8(chip8);
}

void test_execute_8XY2(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x0F;
    chip8->V[0xB] = 0xFC;

    DecodedInstruction instruction = decode(0x8AB2);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xA] == 0x0C);

    free_chip8(chip8);
}

void test_execute_8XY3(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x0F;
    chip8->V[0xB] = 0xFF;

    DecodedInstruction instruction = decode(0x8AB3);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xA] == 0xF0);

    free_chip8(chip8);
}

void test_execute_8XY4_no_carry(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x01;
    chip8->V[0xB] = 0x02;

    DecodedInstruction instruction = decode(0x8AB4);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xA] == 0x03);
    assert(chip8->V[0xF] == 0x00);

    free_chip8(chip8);
}

void test_execute_8XY4_carry(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0xFF;
    chip8->V[0xB] = 0x02;

    DecodedInstruction instruction = decode(0x8AB4);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xA] == 0x01);
    assert(chip8->V[0xF] == 0x01);

    free_chip8(chip8);
}

void test_execute_8XY5_no_borrow(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->V[0xB] = 0x03;

    DecodedInstruction instruction = decode(0x8AB5);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xA] == 0x02);
    assert(chip8->V[0xF] == 0x01);

    free_chip8(chip8);
}

void test_execute_8XY5_borrow(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x03;
    chip8->V[0xB] = 0x05;

    DecodedInstruction instruction = decode(0x8AB5);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xA] == 0xFE);
    assert(chip8->V[0xF] == 0x00);

    free_chip8(chip8);
}

void test_execute_8XY6_flag_one(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;

    DecodedInstruction instruction = decode(0x8AB6);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xA] == 0x02);
    assert(chip8->V[0xF] == 0x01);

    free_chip8(chip8);
}

void test_execute_8XY6_flag_zero(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x04;

    DecodedInstruction instruction = decode(0x8AB6);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xA] == 0x02);
    assert(chip8->V[0xF] == 0x00);

    free_chip8(chip8);
}

void test_execute_8XY7_no_borrow(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x03;
    chip8->V[0xB] = 0x05;

    DecodedInstruction instruction = decode(0x8AB7);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xA] == 0x02);
    assert(chip8->V[0xF] == 0x01);

    free_chip8(chip8);
}

void test_execute_8XY7_borrow(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x05;
    chip8->V[0xB] = 0x03;

    DecodedInstruction instruction = decode(0x8AB7);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xA] == 0xFE);
    assert(chip8->V[0xF] == 0x00);

    free_chip8(chip8);
}

void test_execute_8XYE_flag_one(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x81;

    DecodedInstruction instruction = decode(0x8ABE);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xA] == 0x02);
    assert(chip8->V[0xF] == 0x01);

    free_chip8(chip8);
}

void test_execute_8XYE_flag_zero(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x01;

    DecodedInstruction instruction = decode(0x8ABE);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xA] == 0x02);
    assert(chip8->V[0xF] == 0x00);

    free_chip8(chip8);
}

void test_execute_BNNN(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0x0] = 0x05;

    DecodedInstruction instruction = decode(0xB300);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->PC == 0x305);

    free_chip8(chip8);
}

void test_execute_CXNN_full_mask_produces_varied_values(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    bool seen[256] = { false };
    int distinct_count = 0;

    for (int i = 0; i < 1000; i++) {
        DecodedInstruction instruction = decode(0xCAFF);
        int result = execute(chip8, instruction);
        assert(result == 0);
        assert(chip8->V[0xA] <= 0xFF);
        if (!seen[chip8->V[0xA]]) {
            seen[chip8->V[0xA]] = true;
            distinct_count++;
        }
    }

    assert(distinct_count >= 10);

    free_chip8(chip8);
}

void test_execute_CXNN_partial_mask(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    for (int i = 0; i < 1000; i++) {
        DecodedInstruction instruction = decode(0xCA0F);
        int result = execute(chip8, instruction);
        assert(result == 0);
        assert(chip8->V[0xA] <= 0x0F);
    }

    free_chip8(chip8);
}

void test_execute_CXNN_zero_mask(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    for (int i = 0; i < 1000; i++) {
        DecodedInstruction instruction = decode(0xCA00);
        int result = execute(chip8, instruction);
        assert(result == 0);
        assert(chip8->V[0xA] == 0x00);
    }

    free_chip8(chip8);
}

void test_execute_DXYN_draws_font_zero(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->I = FONTSET_START;
    chip8->V[0x0] = 10;
    chip8->V[0x1] = 5;

    DecodedInstruction instruction = decode(0xD015);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xF] == 0);

    print_display(chip8);

    for (int row = 0; row < 5; row++) {
        uint8_t sprite_byte = fontset[row];
        for (int col = 0; col < 8; col++) {
            uint8_t expected = (sprite_byte >> (7 - col)) & 0x1;
            uint8_t actual = chip8->display[(5 + row) * DISPLAY_WIDTH + (10 + col)];
            assert(actual == expected);
        }
    }

    free_chip8(chip8);
}

void test_execute_DXYN_xor_double_draw_clears_screen(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->memory[0x300] = 0xFF;
    chip8->memory[0x301] = 0x81;
    chip8->memory[0x302] = 0x81;
    chip8->memory[0x303] = 0xFF;
    chip8->I = 0x300;
    chip8->V[0x2] = 20;
    chip8->V[0x3] = 10;

    DecodedInstruction instruction = decode(0xD234);

    int result1 = execute(chip8, instruction);
    assert(result1 == 0);
    assert(chip8->V[0xF] == 0);

    int result2 = execute(chip8, instruction);
    assert(result2 == 0);
    assert(chip8->V[0xF] == 1);

    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        assert(chip8->display[i] == 0);
    }

    free_chip8(chip8);
}

void test_execute_DXYN_no_collision(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->memory[0x300] = 0xFF;
    chip8->I = 0x300;
    chip8->V[0x0] = 0;
    chip8->V[0x1] = 0;

    DecodedInstruction instruction = decode(0xD011);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xF] == 0);

    free_chip8(chip8);
}

void test_execute_DXYN_collision(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->display[0] = 1;
    chip8->memory[0x300] = 0xFF;
    chip8->I = 0x300;
    chip8->V[0x0] = 0;
    chip8->V[0x1] = 0;

    DecodedInstruction instruction = decode(0xD011);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xF] == 1);
    assert(chip8->display[0] == 0);

    free_chip8(chip8);
}

void test_execute_DXYN_clipping_right_edge(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->memory[0x300] = 0xFF;
    chip8->I = 0x300;
    chip8->V[0x4] = 60;
    chip8->V[0x5] = 0;

    DecodedInstruction instruction = decode(0xD451);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xF] == 0);

    for (int col = 0; col < 4; col++) {
        assert(chip8->display[0 * DISPLAY_WIDTH + 60 + col] == 1);
    }

    free_chip8(chip8);
}

void test_execute_DXYN_clipping_bottom_edge(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->memory[0x300] = 0xFF;
    chip8->memory[0x301] = 0xFF;
    chip8->memory[0x302] = 0xFF;
    chip8->memory[0x303] = 0xFF;
    chip8->I = 0x300;
    chip8->V[0x6] = 0;
    chip8->V[0x7] = 31;

    DecodedInstruction instruction = decode(0xD674);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xF] == 0);

    for (int col = 0; col < 8; col++) {
        assert(chip8->display[31 * DISPLAY_WIDTH + col] == 1);
    }

    free_chip8(chip8);
}

void test_execute_DXYN_zero_rows_no_crash(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->memory[0x300] = 0xFF;
    chip8->I = 0x300;
    chip8->V[0x2] = 0;
    chip8->V[0x3] = 0;

    DecodedInstruction instruction = decode(0xD230);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xF] == 0);

    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        assert(chip8->display[i] == 0);
    }

    free_chip8(chip8);
}

void test_execute_EX9E_pressed(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x5;
    chip8->keys[0x5] = 1;
    chip8->memory[PC_START] = 0xEA;
    chip8->memory[PC_START + 1] = 0x9E;

    int fetched = fetch(chip8);
    assert(fetched == 0xEA9E);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->PC == PC_START + 4);

    free_chip8(chip8);
}

void test_execute_EX9E_not_pressed(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x5;
    chip8->keys[0x5] = 0;
    chip8->memory[PC_START] = 0xEA;
    chip8->memory[PC_START + 1] = 0x9E;

    int fetched = fetch(chip8);
    assert(fetched == 0xEA9E);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->PC == PC_START + 2);

    free_chip8(chip8);
}

void test_execute_EXA1_not_pressed(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x5;
    chip8->keys[0x5] = 0;
    chip8->memory[PC_START] = 0xEA;
    chip8->memory[PC_START + 1] = 0xA1;

    int fetched = fetch(chip8);
    assert(fetched == 0xEAA1);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->PC == PC_START + 4);

    free_chip8(chip8);
}

void test_execute_EXA1_pressed(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x5;
    chip8->keys[0x5] = 1;
    chip8->memory[PC_START] = 0xEA;
    chip8->memory[PC_START + 1] = 0xA1;

    int fetched = fetch(chip8);
    assert(fetched == 0xEAA1);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->PC == PC_START + 2);

    free_chip8(chip8);
}

void test_execute_FX0A_waits_when_no_key_pressed(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->memory[PC_START] = 0xFA;
    chip8->memory[PC_START + 1] = 0x0A;

    for (int i = 0; i < 3; i++) {
        int fetched = fetch(chip8);
        assert(fetched == 0xFA0A);
        assert(chip8->PC == PC_START + 2);
        DecodedInstruction instruction = decode(fetched);

        int result = execute(chip8, instruction);
        assert(result == 0);
        assert(chip8->PC == PC_START);
        assert(chip8->V[0xA] == 0);
    }

    free_chip8(chip8);
}

void test_execute_FX0A_resolves_on_key_press(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->memory[PC_START] = 0xFA;
    chip8->memory[PC_START + 1] = 0x0A;

    int fetched = fetch(chip8);
    DecodedInstruction instruction = decode(fetched);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->PC == PC_START);

    chip8->keys[0x7] = 1;

    fetched = fetch(chip8);
    assert(fetched == 0xFA0A);
    assert(chip8->PC == PC_START + 2);
    instruction = decode(fetched);

    result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->PC == PC_START + 2);
    assert(chip8->V[0xA] == 0x7);

    free_chip8(chip8);
}

void test_execute_FX07_reads_delay_timer(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->delay_timer = 0x2A;

    chip8->memory[PC_START] = 0xFA;
    chip8->memory[PC_START + 1] = 0x07;

    int fetched = fetch(chip8);
    assert(fetched == 0xFA07);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xA] == 0x2A);

    free_chip8(chip8);
}

void test_execute_FX15_writes_delay_timer(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x2A;

    chip8->memory[PC_START] = 0xFA;
    chip8->memory[PC_START + 1] = 0x15;

    int fetched = fetch(chip8);
    assert(fetched == 0xFA15);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->delay_timer == 0x2A);

    free_chip8(chip8);
}

void test_execute_FX18_writes_sound_timer(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0x2A;

    chip8->memory[PC_START] = 0xFA;
    chip8->memory[PC_START + 1] = 0x18;

    int fetched = fetch(chip8);
    assert(fetched == 0xFA18);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->sound_timer == 0x2A);

    free_chip8(chip8);
}

void test_execute_FX15_then_update_timers_then_FX07_shows_decrement(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 10;

    chip8->memory[PC_START] = 0xFA;
    chip8->memory[PC_START + 1] = 0x15;
    chip8->memory[PC_START + 2] = 0xFB;
    chip8->memory[PC_START + 3] = 0x07;

    int fetched = fetch(chip8);
    DecodedInstruction instruction = decode(fetched);
    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->delay_timer == 10);

    update_timers(chip8, 100);

    fetched = fetch(chip8);
    instruction = decode(fetched);
    result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0xB] < 10);

    free_chip8(chip8);
}

void test_execute_FX1E_adds_vx_to_i(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->I = 0x300;
    chip8->V[0xA] = 0x10;

    chip8->memory[PC_START] = 0xFA;
    chip8->memory[PC_START + 1] = 0x1E;

    int fetched = fetch(chip8);
    assert(fetched == 0xFA1E);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->I == 0x310);

    free_chip8(chip8);
}

void test_execute_FX29_points_to_font_char_A(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0xA] = 0xA;

    chip8->memory[PC_START] = 0xFA;
    chip8->memory[PC_START + 1] = 0x29;

    int fetched = fetch(chip8);
    assert(fetched == 0xFA29);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->I == FONTSET_START + 0xA * FONT_CHAR_SIZE);

    for (int i = 0; i < FONT_CHAR_SIZE; i++) {
        assert(chip8->memory[chip8->I + i] == fontset[0xA * FONT_CHAR_SIZE + i]);
    }

    free_chip8(chip8);
}

void test_execute_FX33_bcd_decomposition(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->I = 0x300;
    chip8->V[0xA] = 234;

    chip8->memory[PC_START] = 0xFA;
    chip8->memory[PC_START + 1] = 0x33;

    int fetched = fetch(chip8);
    assert(fetched == 0xFA33);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->memory[0x300] == 2);
    assert(chip8->memory[0x301] == 3);
    assert(chip8->memory[0x302] == 4);

    free_chip8(chip8);
}

void test_execute_FX33_bcd_with_leading_zeros(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->I = 0x300;
    chip8->V[0xA] = 7;

    chip8->memory[PC_START] = 0xFA;
    chip8->memory[PC_START + 1] = 0x33;

    int fetched = fetch(chip8);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->memory[0x300] == 0);
    assert(chip8->memory[0x301] == 0);
    assert(chip8->memory[0x302] == 7);

    free_chip8(chip8);
}

void test_execute_FX55_stores_registers_to_memory(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->I = 0x300;
    chip8->V[0x0] = 0x11;
    chip8->V[0x1] = 0x22;
    chip8->V[0x2] = 0x33;
    chip8->V[0x3] = 0x44;

    chip8->memory[PC_START] = 0xF3;
    chip8->memory[PC_START + 1] = 0x55;

    int fetched = fetch(chip8);
    assert(fetched == 0xF355);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->memory[0x300] == 0x11);
    assert(chip8->memory[0x301] == 0x22);
    assert(chip8->memory[0x302] == 0x33);
    assert(chip8->memory[0x303] == 0x44);

    free_chip8(chip8);
}

void test_execute_FX65_loads_registers_from_memory(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->I = 0x300;
    chip8->memory[0x300] = 0xAA;
    chip8->memory[0x301] = 0xBB;
    chip8->memory[0x302] = 0xCC;
    chip8->memory[0x303] = 0xDD;

    chip8->memory[PC_START] = 0xF3;
    chip8->memory[PC_START + 1] = 0x65;

    int fetched = fetch(chip8);
    assert(fetched == 0xF365);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0x0] == 0xAA);
    assert(chip8->V[0x1] == 0xBB);
    assert(chip8->V[0x2] == 0xCC);
    assert(chip8->V[0x3] == 0xDD);

    free_chip8(chip8);
}

void test_execute_FX55_near_mem_size_does_not_write_out_of_bounds(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->I = MEM_SIZE - 2;
    chip8->V[0x0] = 0x11;
    chip8->V[0x1] = 0x22;
    chip8->V[0x2] = 0x33;
    chip8->V[0x3] = 0x44;

    chip8->memory[PC_START] = 0xF3;
    chip8->memory[PC_START + 1] = 0x55;

    int fetched = fetch(chip8);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->memory[MEM_SIZE - 2] == 0x11);
    assert(chip8->memory[MEM_SIZE - 1] == 0x22);

    free_chip8(chip8);
}

void test_execute_FX65_near_mem_size_does_not_read_out_of_bounds(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->I = MEM_SIZE - 2;
    chip8->memory[MEM_SIZE - 2] = 0x55;
    chip8->memory[MEM_SIZE - 1] = 0x66;
    chip8->V[0x2] = 0x99;
    chip8->V[0x3] = 0xAA;

    chip8->memory[PC_START] = 0xF3;
    chip8->memory[PC_START + 1] = 0x65;

    int fetched = fetch(chip8);
    DecodedInstruction instruction = decode(fetched);

    int result = execute(chip8, instruction);
    assert(result == 0);
    assert(chip8->V[0x0] == 0x55);
    assert(chip8->V[0x1] == 0x66);
    assert(chip8->V[0x2] == 0x99);
    assert(chip8->V[0x3] == 0xAA);

    free_chip8(chip8);
}

static void run_cpu_cycles(Chip8* chip8, int max_cycles) {
    uint16_t prev_pc = 0xFFFF;
    for (int i = 0; i < max_cycles; i++) {
        uint16_t current_pc = chip8->PC;
        if (current_pc == prev_pc) {
            break;
        }
        prev_pc = current_pc;
        uint16_t opcode = fetch(chip8);
        DecodedInstruction instruction = decode(opcode);
        int result = execute(chip8, instruction);
        assert(result == 0);
    }
}

void test_execute_ibm_logo_rom(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    char* rom_path = "../roms/ibm_logo.ch8";
    int load_result = load_rom(chip8, rom_path);
    assert(load_result == 0);

    run_cpu_cycles(chip8, 50);

    assert(chip8->PC == 0x228);

    printf("IBM logo framebuffer:\n");
    print_display(chip8);

    int lit_pixel_count = 0;
    int min_x = DISPLAY_WIDTH, max_x = -1, min_y = DISPLAY_HEIGHT, max_y = -1;
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            if (chip8->display[y * DISPLAY_WIDTH + x]) {
                lit_pixel_count++;
                if (x < min_x) min_x = x;
                if (x > max_x) max_x = x;
                if (y < min_y) min_y = y;
                if (y > max_y) max_y = y;
            }
        }
    }

    assert(lit_pixel_count > 100);
    assert(min_x > 0 && max_x < DISPLAY_WIDTH - 1);
    assert(min_y > 0 && max_y < DISPLAY_HEIGHT - 1);

    free_chip8(chip8);
}

int main(void) {
    printf("Running tests...\n");
    test_init_chip8_pc();
    test_init_chip8_registers();
    test_init_chip8_index_register();
    test_init_chip8_timers();
    test_init_chip8_keys();
    test_init_chip8_memory_linked();
    test_mem_size();
    test_pc();
    test_large_rom();
    test_load_rom_ok();
    test_font();
    test_stack_push_pop();
    test_clear_display();
    test_update_timers_decrements_when_above_zero();
    test_update_timers_never_goes_below_zero();
    test_update_timers_small_elapsed_does_not_decrement_yet();
    test_update_timers_simulated_half_second_reaches_zero();
    test_update_timers_multiple_small_calls_reach_zero();
    test_update_timers_handles_multiple_ticks_in_one_call();
    test_update_timers_independent_of_cpu_cycles();
    test_fetch();
    test_decode();
    test_execute_00E0();
    test_execute_1NNN();
    test_execute_loop_1NNN();
    test_execute_6XNN();
    test_execute_7XNN();
    test_execute_7XNN_overflow();
    test_execute_ANNN();
    test_execute_2NNN();
    test_execute_00EE();
    test_execute_00EE_empty_stack();
    test_execute_nested_subroutines();
    test_execute_3XNN_true();
    test_execute_3XNN_false();
    test_execute_4XNN_true();
    test_execute_4XNN_false();
    test_execute_5XY0_true();
    test_execute_5XY0_false();
    test_execute_9XY0_true();
    test_execute_9XY0_false();
    test_execute_3XNN_program_skips_forbidden_instruction();
    test_execute_3XNN_program_executes_forbidden_instruction_when_false();
    test_execute_4XNN_program_skips_forbidden_instruction();
    test_execute_4XNN_program_executes_forbidden_instruction_when_false();
    test_execute_5XY0_program_skips_forbidden_instruction();
    test_execute_5XY0_program_executes_forbidden_instruction_when_false();
    test_execute_9XY0_program_skips_forbidden_instruction();
    test_execute_9XY0_program_executes_forbidden_instruction_when_false();
    test_execute_8XY0();
    test_execute_8XY1();
    test_execute_8XY2();
    test_execute_8XY3();
    test_execute_8XY4_no_carry();
    test_execute_8XY4_carry();
    test_execute_8XY5_no_borrow();
    test_execute_8XY5_borrow();
    test_execute_8XY6_flag_one();
    test_execute_8XY6_flag_zero();
    test_execute_8XY7_no_borrow();
    test_execute_8XY7_borrow();
    test_execute_8XYE_flag_one();
    test_execute_8XYE_flag_zero();
    test_execute_BNNN();
    test_execute_CXNN_full_mask_produces_varied_values();
    test_execute_CXNN_partial_mask();
    test_execute_CXNN_zero_mask();
    test_execute_DXYN_draws_font_zero();
    test_execute_DXYN_xor_double_draw_clears_screen();
    test_execute_DXYN_no_collision();
    test_execute_DXYN_collision();
    test_execute_DXYN_clipping_right_edge();
    test_execute_DXYN_clipping_bottom_edge();
    test_execute_DXYN_zero_rows_no_crash();
    test_execute_EX9E_pressed();
    test_execute_EX9E_not_pressed();
    test_execute_EXA1_not_pressed();
    test_execute_EXA1_pressed();
    test_execute_FX0A_waits_when_no_key_pressed();
    test_execute_FX0A_resolves_on_key_press();
    test_execute_FX07_reads_delay_timer();
    test_execute_FX15_writes_delay_timer();
    test_execute_FX18_writes_sound_timer();
    test_execute_FX15_then_update_timers_then_FX07_shows_decrement();
    test_execute_FX1E_adds_vx_to_i();
    test_execute_FX29_points_to_font_char_A();
    test_execute_FX33_bcd_decomposition();
    test_execute_FX33_bcd_with_leading_zeros();
    test_execute_FX55_stores_registers_to_memory();
    test_execute_FX65_loads_registers_from_memory();
    test_execute_FX55_near_mem_size_does_not_write_out_of_bounds();
    test_execute_FX65_near_mem_size_does_not_read_out_of_bounds();
    test_execute_ibm_logo_rom();
    printf("All tests passed!\n");
    return 0;
}