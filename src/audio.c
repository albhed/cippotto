#include <stdlib.h>
#include "audio.h"

void generate_square_wave(int16_t* buffer, int num_samples, int sample_rate,
                           double freq, int16_t amplitude, uint32_t start_sample) {
    double samples_per_half_cycle = sample_rate / (2.0 * freq);

    for (int i = 0; i < num_samples; i++) {
        uint32_t sample_pos = start_sample + (uint32_t)i;
        long half_cycle_index = (long)(sample_pos / samples_per_half_cycle);
        buffer[i] = (half_cycle_index % 2 == 0) ? amplitude : (int16_t)(-amplitude);
    }
}

static void audio_callback(void* userdata, Uint8* stream, int len) {
    Audio* audio = (Audio*)userdata;
    int16_t* buffer = (int16_t*)stream;
    int num_samples = len / (int)sizeof(int16_t);

    generate_square_wave(buffer, num_samples, AUDIO_SAMPLE_RATE, AUDIO_FREQUENCY_HZ,
                          AUDIO_AMPLITUDE, audio->sample_index);
    audio->sample_index += (uint32_t)num_samples;
}

Audio* audio_init(void) {
    Audio* audio = (Audio*)malloc(sizeof(Audio));
    if (audio == NULL) {
        return NULL;
    }

    audio->sample_index = 0;
    audio->playing = 0;

    SDL_AudioSpec want;
    SDL_zero(want);
    want.freq = AUDIO_SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 2048;
    want.callback = audio_callback;
    want.userdata = audio;

    audio->device_id = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    if (audio->device_id == 0) {
        free(audio);
        return NULL;
    }

    return audio;
}

void audio_destroy(Audio* audio) {
    if (audio == NULL) {
        return;
    }
    SDL_CloseAudioDevice(audio->device_id);
    free(audio);
}

void audio_update(Audio* audio, uint8_t sound_timer) {
    if (audio == NULL) {
        return;
    }

    if (sound_timer > 0 && !audio->playing) {
        SDL_PauseAudioDevice(audio->device_id, 0);
        audio->playing = 1;
    } else if (sound_timer == 0 && audio->playing) {
        SDL_PauseAudioDevice(audio->device_id, 1);
        audio->playing = 0;
    }
}
