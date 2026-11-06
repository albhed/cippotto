#ifndef QUIRKS_H
#define QUIRKS_H

#include <stdbool.h>

typedef struct {
    bool quirk_shift_vy;
    bool quirk_wrap;
    bool quirk_jump_vx;
    bool quirk_fx55_increment;
    bool quirk_display_wait;
} QuirkConfig;

QuirkConfig parse_quirk_args(int argc, char** argv, const char** rom_path);

bool quirk_help_requested(int argc, char** argv);
const char* quirk_help_text(void);

#endif /* QUIRKS_H */
