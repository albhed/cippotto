#include <assert.h>
#include <stdio.h>
#include "../src/mem.h"

void test_mem_size(void) {
    assert(MEM_SIZE == 4096);
    assert(sizeof(memory) == MEM_SIZE);
}

void test_pc(void) {
    assert(PC_START == 0x200);
}

int main(void) {
    printf("Running memory tests...\n");
    test_mem_size();
    test_pc();
    printf("All tests passed!\n");
    return 0;
}