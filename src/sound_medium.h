#ifndef SOUND_MEDIUM_H_
#define SOUND_MEDIUM_H_

#include "./point.h"

typedef struct sound_medium_t sound_medium_t;

// TODO: sound_medium does not propagate any sounds

sound_medium_t *create_sound_medium_from_folder(const char *folder_path);
void destroy_sound_medium(sound_medium_t *sound_medium);

int sound_medium_play_sound(sound_medium_t *sound_medium,
                            const char *sound_name,
                            point_t position);
int sound_medium_listen_sounds(sound_medium_t *sound_medium,
                               point_t position);

#endif  // SOUND_MEDIUM_H_
