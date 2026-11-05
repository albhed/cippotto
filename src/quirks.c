#include <string.h>
#include <stddef.h>
#include "quirks.h"

QuirkConfig parse_quirk_args(int argc, char** argv, const char** rom_path) {
    QuirkConfig config = {
        .quirk_shift_vy = false,
        .quirk_wrap = false,
        .quirk_jump_vx = false,
        .quirk_fx55_increment = false,
    };

    *rom_path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--quirk-shift-vy") == 0) {
            config.quirk_shift_vy = true;
        } else if (strcmp(argv[i], "--quirk-wrap") == 0) {
            config.quirk_wrap = true;
        } else if (strcmp(argv[i], "--quirk-jump-vx") == 0) {
            config.quirk_jump_vx = true;
        } else if (strcmp(argv[i], "--quirk-fx55-increment") == 0) {
            config.quirk_fx55_increment = true;
        } else {
            *rom_path = argv[i];
        }
    }

    return config;
}
