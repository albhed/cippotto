#ifndef LOOP_H
#define LOOP_H

#include <stdbool.h>
#include <stdint.h>
#include "chip8.h"
#include "display.h"
#include "audio.h"

#define INSTRUCTIONS_PER_FRAME 11

bool run_frame(Chip8* chip8, Display* display, Audio* audio, uint32_t elapsed_ms);

#endif /* LOOP_H */
