#ifndef SOUND_MEDIUM_H_
#define SOUND_MEDIUM_H_

#include <SDL2/SDL_mixer.h>

#include "math/point.h"

typedef struct sound_medium_t sound_medium_t;

sound_medium_t *create_sound_medium(Mix_Chunk **samples,
                                    size_t samples_count,
                                    int lower_channel,
                                    int upper_channel);
void destroy_sound_medium(sound_medium_t *sound_medium);

int sound_medium_play_sound(sound_medium_t *sound_medium,
                            size_t sound_index,
                            point_t position,
                            int loops);

int sound_medium_toggle_pause(sound_medium_t *sound_medium);

#endif  // SOUND_MEDIUM_H_
