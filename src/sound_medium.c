#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "./sound_medium.h"
#include "./lt.h"
#include "./error.h"

struct sound_medium_t
{
    lt_t *lt;
    Mix_Chunk **samples;
    size_t samples_count;
};

static int mix_get_free_channel(void)
{
    for (int i = 0; i < MIX_CHANNELS; ++i) {
        if (!Mix_Playing(i)) {
            return i;
        }
    }

    return -1;
}

sound_medium_t *create_sound_medium(Mix_Chunk **samples, size_t samples_count)
{
    assert(samples);
    assert(samples_count > 0);

    lt_t *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    sound_medium_t *sound_medium = PUSH_LT(lt, malloc(sizeof(sound_medium_t)), free);
    if (sound_medium == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    sound_medium->samples = samples;
    sound_medium->samples_count = samples_count;

    sound_medium->lt = lt;

    return sound_medium;
}

void destroy_sound_medium(sound_medium_t *sound_medium)
{
    assert(sound_medium);
    RETURN_LT0(sound_medium->lt);
}

/* TODO(#134): sound_medium doesn't take into account the positions of the sound and the listener */
int sound_medium_play_sound(sound_medium_t *sound_medium,
                            size_t sound_index,
                            point_t position)
{
    assert(sound_medium);
    (void) sound_index;
    (void) position;

    if (sound_index < sound_medium->samples_count) {
        const int free_channel = mix_get_free_channel();

        if (free_channel >= 0) {
            return Mix_PlayChannel(free_channel, sound_medium->samples[sound_index], 0);
        }
    }

    return 0;
}

int sound_medium_listen_sounds(sound_medium_t *sound_medium,
                               point_t position)
{
    assert(sound_medium);
    (void) position;
    return 0;
}
