#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "./sound_medium.h"

sound_medium_t *create_sound_medium_from_folder(const char *folder_path)
{
    assert(folder_path);
    return NULL;
}

void destroy_sound_medium(sound_medium_t *sound_medium)
{
    assert(sound_medium);
}

int sound_medium_play_sound(sound_medium_t *sound_medium,
                            const char *sound_name,
                            point_t position)
{
    assert(sound_medium);
    assert(sound_name);
    (void) position;

    return 0;
}

int sound_medium_listen_sounds(sound_medium_t *sound_medium,
                               point_t position)
{
    assert(sound_medium);
    (void) position;
    return 0;
}
