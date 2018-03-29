#ifndef SOUND_MEDIUM_H_
#define SOUND_MEDIUM_H_

#include "./point.h"

typedef struct sound_medium_t sound_medium_t;
typedef struct Mix_Chunk Mix_Chunk;

sound_medium_t *create_sound_medium(Mix_Chunk **samples, size_t samples_count);
void destroy_sound_medium(sound_medium_t *sound_medium);

int sound_medium_play_sound(sound_medium_t *sound_medium,
                            size_t sound_index,
                            point_t position);
int sound_medium_listen_sounds(sound_medium_t *sound_medium,
                               point_t position);

#endif  // SOUND_MEDIUM_H_
