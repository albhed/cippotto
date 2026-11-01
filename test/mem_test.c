#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/mem.h"
#include "../src/err.h"

void test_mem_size(void) {
    int expected_mem_size = 4096;
    Memory* mem = init_memory();
    assert(mem != NULL);
    assert(sizeof(mem->data) == expected_mem_size);
}

void test_pc(void) {
    assert(PC_START == 0x200);
}

void test_large_rom(void) {
    Memory* mem = init_memory();
    assert(mem != NULL);
    char* large_rom_path = "../roms/large.ch8";
    int result = load_rom(mem, large_rom_path);
    assert(result == ERR_ROM_TOO_LARGE);
    for (int i = PC_START; i < MEM_SIZE; i++) {
        assert(mem->data[i] == 0);
    }
    free(mem);
}

void test_load_rom_ok(void) {
    Memory* mem = init_memory();
    assert(mem != NULL);
    char* rom_path = "../roms/ok.ch8";
    int result = load_rom(mem, rom_path);
    assert(result == 0);
    for (int i = 0; i < 10; i++) {
        printf("mem->data[PC_START + %d] = %.2x\n", i, mem->data[PC_START + i]);
        assert(mem->data[PC_START + i] != 0);
    }
    free(mem);
}

int main(void) {
    printf("Running memory tests...\n");
    test_mem_size();
    test_pc();
    test_large_rom();
    test_load_rom_ok();
    printf("All tests passed!\n");
    return 0;
}