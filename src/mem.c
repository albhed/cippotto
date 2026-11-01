#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "err.h"

Memory* init_memory(void) {
    Memory* mem = (Memory*)malloc(sizeof(Memory));
    if (mem == NULL) {
        printf("Error: Could not allocate memory for Memory struct\n");
        return NULL;
    }
    memset(mem->data, 0, sizeof(mem->data));
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