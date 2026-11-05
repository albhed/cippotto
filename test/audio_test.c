#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "../src/chip8.h"
#include "../src/audio.h"

#define TEST_AMPLITUDE 1000

static void test_generate_square_wave_alternates_at_expected_rate(void) {
    int sample_rate = 8000;
    double freq = 1000.0;
    int samples_per_half_cycle = 4;
    int num_samples = 16;
    int16_t buffer[16];

    generate_square_wave(buffer, num_samples, sample_rate, freq, TEST_AMPLITUDE, 0);

    for (int i = 0; i < num_samples; i++) {
        int half_cycle = i / samples_per_half_cycle;
        int16_t expected = (half_cycle % 2 == 0) ? TEST_AMPLITUDE : -TEST_AMPLITUDE;
        assert(buffer[i] == expected);
    }
}

static void test_generate_square_wave_is_continuous_across_calls(void) {
    int sample_rate = 8000;
    double freq = 1000.0;
    int total_samples = 16;
    int16_t full_buffer[16];
    int16_t chunk_a[8];
    int16_t chunk_b[8];

    generate_square_wave(full_buffer, total_samples, sample_rate, freq, TEST_AMPLITUDE, 0);
    generate_square_wave(chunk_a, 8, sample_rate, freq, TEST_AMPLITUDE, 0);
    generate_square_wave(chunk_b, 8, sample_rate, freq, TEST_AMPLITUDE, 8);

    for (int i = 0; i < 8; i++) {
        assert(chunk_a[i] == full_buffer[i]);
        assert(chunk_b[i] == full_buffer[8 + i]);
    }
}

static void test_audio_init_destroy_opens_device_without_error(void) {
    SDL_ClearError();

    Audio* audio = audio_init();
    assert(audio != NULL);
    assert(audio->device_id != 0);
    assert(audio->playing == 0);

    audio_destroy(audio);
    assert(strlen(SDL_GetError()) == 0);
}

static void test_audio_init_destroy_multiple_times_without_leaks_or_errors(void) {
    for (int i = 0; i < 5; i++) {
        SDL_ClearError();

        Audio* audio = audio_init();
        assert(audio != NULL);
        assert(audio->device_id != 0);

        audio_destroy(audio);
        assert(strlen(SDL_GetError()) == 0);
    }
}

static void test_audio_update_starts_and_stops_playback(void) {
    Audio* audio = audio_init();
    assert(audio != NULL);

    assert(SDL_GetAudioDeviceStatus(audio->device_id) == SDL_AUDIO_PAUSED);

    audio_update(audio, 5);
    assert(audio->playing == 1);
    assert(SDL_GetAudioDeviceStatus(audio->device_id) == SDL_AUDIO_PLAYING);

    audio_update(audio, 0);
    assert(audio->playing == 0);
    assert(SDL_GetAudioDeviceStatus(audio->device_id) == SDL_AUDIO_PAUSED);

    audio_destroy(audio);
}

static void test_audio_update_ignores_null_audio(void) {
    audio_update(NULL, 10);
}

static void test_audio_update_integrates_with_sound_timer_countdown(void) {
    Chip8* chip8 = init_chip8();
    assert(chip8 != NULL);

    chip8->V[0] = 10;
    DecodedInstruction set_sound_timer = {0xF, 0x0, 0x0, 0x0, 0x18, 0x0};
    int exec_result = execute(chip8, set_sound_timer);
    assert(exec_result == 0);
    assert(chip8->sound_timer == 10);

    Audio* audio = audio_init();
    assert(audio != NULL);

    audio_update(audio, chip8->sound_timer);
    assert(audio->playing == 1);
    assert(SDL_GetAudioDeviceStatus(audio->device_id) == SDL_AUDIO_PLAYING);

    update_timers(chip8, 1000);
    assert(chip8->sound_timer == 0);

    audio_update(audio, chip8->sound_timer);
    assert(audio->playing == 0);
    assert(SDL_GetAudioDeviceStatus(audio->device_id) == SDL_AUDIO_PAUSED);

    audio_destroy(audio);
    free_chip8(chip8);
}

int main(void) {
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    printf("Running audio tests...\n");
    test_generate_square_wave_alternates_at_expected_rate();
    test_generate_square_wave_is_continuous_across_calls();
    test_audio_init_destroy_opens_device_without_error();
    test_audio_init_destroy_multiple_times_without_leaks_or_errors();
    test_audio_update_starts_and_stops_playback();
    test_audio_update_ignores_null_audio();
    test_audio_update_integrates_with_sound_timer_countdown();

    SDL_Quit();
    printf("All audio tests passed!\n");
    return 0;
}
