#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <SDL2/SDL.h>

#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_FREQUENCY_HZ 440.0
#define AUDIO_AMPLITUDE 3000

typedef struct {
    SDL_AudioDeviceID device_id;
    uint32_t sample_index;
    int playing;
} Audio;

void generate_square_wave(int16_t* buffer, int num_samples, int sample_rate,
                           double freq, int16_t amplitude, uint32_t start_sample);

Audio* audio_init(void);
void audio_destroy(Audio* audio);
void audio_update(Audio* audio, uint8_t sound_timer);

#endif /* AUDIO_H */
