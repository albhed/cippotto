#ifndef MEM_H
#define MEM_H

#include <stdint.h>

#define MEM_SIZE 4096
#define PC_START 0x200
#define FONTSET_START 0x000

typedef struct {
    uint8_t data[MEM_SIZE];
} Memory;

Memory* init_memory(void);
int load_rom(Memory* mem, const char* filepath);

#endif /* MEM_H */