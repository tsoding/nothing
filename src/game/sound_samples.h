#ifndef SOUND_SAMPLES_H_
#define SOUND_SAMPLES_H_

#include "math/point.h"

typedef struct Sound_samples Sound_samples;

Sound_samples *create_sound_samples(const char *sample_files[],
                                      size_t sample_files_count);
void destroy_sound_samples(Sound_samples *sound_samples);

int sound_samples_play_sound(Sound_samples *sound_samples,
                             size_t sound_index);

int sound_samples_toggle_pause(Sound_samples *sound_samples);

#endif  // SOUND_SAMPLES_H_
