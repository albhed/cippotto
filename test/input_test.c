#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "../src/chip8.h"
#include "../src/display.h"
#include "../src/input.h"

typedef struct {
    SDL_Scancode scancode;
    uint8_t chip8_key;
} KeyMapping;

static const KeyMapping key_mappings[] = {
    {SDL_SCANCODE_1, 0x1}, {SDL_SCANCODE_2, 0x2}, {SDL_SCANCODE_3, 0x3}, {SDL_SCANCODE_4, 0xC},
    {SDL_SCANCODE_Q, 0x4}, {SDL_SCANCODE_W, 0x5}, {SDL_SCANCODE_E, 0x6}, {SDL_SCANCODE_R, 0xD},
    {SDL_SCANCODE_A, 0x7}, {SDL_SCANCODE_S, 0x8}, {SDL_SCANCODE_D, 0x9}, {SDL_SCANCODE_F, 0xE},
    {SDL_SCANCODE_Z, 0xA}, {SDL_SCANCODE_X, 0x0}, {SDL_SCANCODE_C, 0xB}, {SDL_SCANCODE_V, 0xF},
};

static void push_key_event(Uint32 type, SDL_Scancode scancode) {
    SDL_Event event;
    SDL_zero(event);
    event.type = type;
    event.key.type = type;
    event.key.state = (type == SDL_KEYDOWN) ? SDL_PRESSED : SDL_RELEASED;
    event.key.keysym.scancode = scancode;
    SDL_PushEvent(&event);
}

static void test_all_mapped_keys_set_and_clear_state(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    size_t num_mappings = sizeof(key_mappings) / sizeof(key_mappings[0]);
    assert(num_mappings == NUM_KEYS);

    for (size_t i = 0; i < num_mappings; i++) {
        push_key_event(SDL_KEYDOWN, key_mappings[i].scancode);
        bool running = display_process_events(chip8);
        assert(running == true);
        assert(chip8->keys[key_mappings[i].chip8_key] == 1);

        push_key_event(SDL_KEYUP, key_mappings[i].scancode);
        running = display_process_events(chip8);
        assert(running == true);
        assert(chip8->keys[key_mappings[i].chip8_key] == 0);
    }

    free_chip8(chip8);
}

static void test_unmapped_key_does_not_crash_or_alter_state(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    uint8_t snapshot[NUM_KEYS];
    memcpy(snapshot, chip8->keys, sizeof(snapshot));

    push_key_event(SDL_KEYDOWN, SDL_SCANCODE_G);
    bool running = display_process_events(chip8);
    assert(running == true);

    push_key_event(SDL_KEYUP, SDL_SCANCODE_G);
    running = display_process_events(chip8);
    assert(running == true);

    assert(memcmp(snapshot, chip8->keys, sizeof(snapshot)) == 0);

    free_chip8(chip8);
}

static void test_handle_event_ignores_null_chip8(void) {
    SDL_Event event;
    SDL_zero(event);
    event.type = SDL_KEYDOWN;
    event.key.keysym.scancode = SDL_SCANCODE_1;

    input_handle_event(NULL, &event);
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    printf("Running input tests...\n");
    test_all_mapped_keys_set_and_clear_state();
    test_unmapped_key_does_not_crash_or_alter_state();
    test_handle_event_ignores_null_chip8();

    SDL_Quit();
    printf("All input tests passed!\n");
    return 0;
}
