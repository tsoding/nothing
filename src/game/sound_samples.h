#ifndef SOUND_SAMPLES_H_
#define SOUND_SAMPLES_H_

#include <SDL2/SDL_mixer.h>

#include "math/point.h"

typedef struct sound_samples_t sound_samples_t;

sound_samples_t *create_sound_samples(Mix_Chunk **samples,
                                    size_t samples_count,
                                    int lower_channel,
                                    int upper_channel);
void destroy_sound_samples(sound_samples_t *sound_samples);

int sound_samples_play_sound(sound_samples_t *sound_samples,
                            size_t sound_index,
                            int loops);

int sound_samples_toggle_pause(sound_samples_t *sound_samples);

#endif  // SOUND_SAMPLES_H_
