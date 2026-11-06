#include "loop.h"

bool run_frame(Chip8* chip8, Display* display, Audio* audio, uint32_t elapsed_ms) {
    bool running = display_process_events(chip8);

    chip8->sprite_drawn = false;
    for (int i = 0; i < INSTRUCTIONS_PER_FRAME && running; i++) {
        uint16_t opcode = fetch(chip8);
        DecodedInstruction instruction = decode(opcode);
        execute(chip8, instruction);

        if (chip8->quirks.quirk_display_wait && chip8->sprite_drawn) {
            break;
        }
    }

    update_timers(chip8, elapsed_ms);
    audio_update(audio, chip8->sound_timer);
    display_render(display, chip8);

    return running;
}
