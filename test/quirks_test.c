#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/quirks.h"

static void test_parse_quirk_args_no_flags_uses_defaults(void) {
    char* argv[] = {"chip8", "roms/game.ch8"};
    const char* rom_path = NULL;
    QuirkConfig config = parse_quirk_args(2, argv, &rom_path);

    assert(config.quirk_shift_vy == false);
    assert(config.quirk_wrap == false);
    assert(config.quirk_jump_vx == false);
    assert(config.quirk_fx55_increment == false);
    assert(rom_path != NULL);
    assert(strcmp(rom_path, "roms/game.ch8") == 0);
}

static void test_parse_quirk_args_no_args_at_all(void) {
    char* argv[] = {"chip8"};
    const char* rom_path = NULL;
    QuirkConfig config = parse_quirk_args(1, argv, &rom_path);

    assert(config.quirk_shift_vy == false);
    assert(config.quirk_wrap == false);
    assert(config.quirk_jump_vx == false);
    assert(config.quirk_fx55_increment == false);
    assert(rom_path == NULL);
}

static void test_parse_quirk_args_shift_vy_only(void) {
    char* argv[] = {"chip8", "--quirk-shift-vy", "roms/game.ch8"};
    const char* rom_path = NULL;
    QuirkConfig config = parse_quirk_args(3, argv, &rom_path);

    assert(config.quirk_shift_vy == true);
    assert(config.quirk_wrap == false);
    assert(config.quirk_jump_vx == false);
    assert(config.quirk_fx55_increment == false);
    assert(strcmp(rom_path, "roms/game.ch8") == 0);
}

static void test_parse_quirk_args_wrap_only(void) {
    char* argv[] = {"chip8", "--quirk-wrap", "roms/game.ch8"};
    const char* rom_path = NULL;
    QuirkConfig config = parse_quirk_args(3, argv, &rom_path);

    assert(config.quirk_shift_vy == false);
    assert(config.quirk_wrap == true);
    assert(config.quirk_jump_vx == false);
    assert(config.quirk_fx55_increment == false);
    assert(strcmp(rom_path, "roms/game.ch8") == 0);
}

static void test_parse_quirk_args_jump_vx_only(void) {
    char* argv[] = {"chip8", "--quirk-jump-vx", "roms/game.ch8"};
    const char* rom_path = NULL;
    QuirkConfig config = parse_quirk_args(3, argv, &rom_path);

    assert(config.quirk_shift_vy == false);
    assert(config.quirk_wrap == false);
    assert(config.quirk_jump_vx == true);
    assert(config.quirk_fx55_increment == false);
    assert(strcmp(rom_path, "roms/game.ch8") == 0);
}

static void test_parse_quirk_args_fx55_increment_only(void) {
    char* argv[] = {"chip8", "--quirk-fx55-increment", "roms/game.ch8"};
    const char* rom_path = NULL;
    QuirkConfig config = parse_quirk_args(3, argv, &rom_path);

    assert(config.quirk_shift_vy == false);
    assert(config.quirk_wrap == false);
    assert(config.quirk_jump_vx == false);
    assert(config.quirk_fx55_increment == true);
    assert(strcmp(rom_path, "roms/game.ch8") == 0);
}

static void test_parse_quirk_args_multiple_flags(void) {
    char* argv[] = {"chip8", "--quirk-wrap", "--quirk-jump-vx", "roms/game.ch8"};
    const char* rom_path = NULL;
    QuirkConfig config = parse_quirk_args(4, argv, &rom_path);

    assert(config.quirk_shift_vy == false);
    assert(config.quirk_wrap == true);
    assert(config.quirk_jump_vx == true);
    assert(config.quirk_fx55_increment == false);
    assert(strcmp(rom_path, "roms/game.ch8") == 0);
}

static void test_parse_quirk_args_all_flags(void) {
    char* argv[] = {
        "chip8",
        "--quirk-shift-vy",
        "--quirk-wrap",
        "--quirk-jump-vx",
        "--quirk-fx55-increment",
        "roms/game.ch8"
    };
    const char* rom_path = NULL;
    QuirkConfig config = parse_quirk_args(6, argv, &rom_path);

    assert(config.quirk_shift_vy == true);
    assert(config.quirk_wrap == true);
    assert(config.quirk_jump_vx == true);
    assert(config.quirk_fx55_increment == true);
    assert(strcmp(rom_path, "roms/game.ch8") == 0);
}

static void test_parse_quirk_args_unknown_argument_treated_as_rom_path(void) {
    char* argv[] = {"chip8", "--unknown-flag"};
    const char* rom_path = NULL;
    QuirkConfig config = parse_quirk_args(2, argv, &rom_path);

    assert(config.quirk_shift_vy == false);
    assert(config.quirk_wrap == false);
    assert(config.quirk_jump_vx == false);
    assert(config.quirk_fx55_increment == false);
    assert(rom_path != NULL);
    assert(strcmp(rom_path, "--unknown-flag") == 0);
}

int main(void) {
    printf("Running quirks tests...\n");
    test_parse_quirk_args_no_flags_uses_defaults();
    test_parse_quirk_args_no_args_at_all();
    test_parse_quirk_args_shift_vy_only();
    test_parse_quirk_args_wrap_only();
    test_parse_quirk_args_jump_vx_only();
    test_parse_quirk_args_fx55_increment_only();
    test_parse_quirk_args_multiple_flags();
    test_parse_quirk_args_all_flags();
    test_parse_quirk_args_unknown_argument_treated_as_rom_path();
    printf("All quirks tests passed!\n");
    return 0;
}
