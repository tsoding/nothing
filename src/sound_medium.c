#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "./sound_medium.h"
#include "./lt.h"
#include "./error.h"

struct sound_medium_t
{
    lt_t *lt;
};

sound_medium_t *create_sound_medium_from_folder(const char *folder_path)
{
    assert(folder_path);

    lt_t *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    sound_medium_t *sound_medium = PUSH_LT(lt, malloc(sizeof(sound_medium_t)), free);
    if (sound_medium == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    sound_medium->lt = lt;

    return sound_medium;
}

void destroy_sound_medium(sound_medium_t *sound_medium)
{
    assert(sound_medium);
    RETURN_LT0(sound_medium->lt);
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
