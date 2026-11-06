#include <string.h>
#include <stddef.h>
#include "quirks.h"

QuirkConfig parse_quirk_args(int argc, char** argv, const char** rom_path) {
    QuirkConfig config = {
        .quirk_shift_vy = false,
        .quirk_wrap = false,
        .quirk_jump_vx = false,
        .quirk_fx55_increment = false,
        .quirk_display_wait = false,
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
        } else if (strcmp(argv[i], "--quirk-display-wait") == 0) {
            config.quirk_display_wait = true;
        } else {
            *rom_path = argv[i];
        }
    }

    return config;
}

bool quirk_help_requested(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            return true;
        }
    }
    return false;
}

const char* quirk_help_text(void) {
    return
        "Usage: chip8 [options] <rom_path>\n"
        "\n"
        "Options:\n"
        "  --help, -h               Show this help message and exit\n"
        "  --quirk-shift-vy         Use VY instead of VX as shift source for 8XY6/8XYE\n"
        "  --quirk-wrap             Wrap sprites around screen edges instead of clipping\n"
        "  --quirk-jump-vx          Use VX as offset register for BNNN jump (BXNN behavior)\n"
        "  --quirk-fx55-increment   Increment I register after FX55/FX65 memory operations\n"
        "  --quirk-display-wait     Limit DXYN to one sprite draw per frame\n"
        "\n"
        "Arguments:\n"
        "  <rom_path>               Path to the CHIP-8 ROM file to load\n";
}
