#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"
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

Memory* init_memory(void) {
    Memory* mem = (Memory*)malloc(sizeof(Memory));
    if (mem == NULL) {
        printf("Error: Could not allocate memory for Memory struct\n");
        return NULL;
    }
    memset(mem->data, 0, sizeof(mem->data));
    memcpy(&mem->data[FONTSET_START], fontset, 80);
    return mem;
}

int load_rom(Memory* mem, const char* filepath) {
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

    if (file_size > sizeof(mem->data) - PC_START) {
        printf("Error: ROM size exceeds available memory\n");
        fclose(ptr);
        return ERR_ROM_TOO_LARGE;
    }
    fseek(ptr, 0, SEEK_SET);

    size_t byte_read = fread(&mem->data[PC_START], sizeof(uint8_t), sizeof(mem->data) - PC_START, ptr);
    if (byte_read != file_size) {
        printf("Error: Could not read the entire ROM file\n");
        fclose(ptr);
        return ERR_ROM_EOF;
    }
    fclose(ptr);
    return 0;
}