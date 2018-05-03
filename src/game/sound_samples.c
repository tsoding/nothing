#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "math/pi.h"
#include "sound_samples.h"
#include "system/error.h"
#include "system/lt.h"

struct sound_samples_t
{
    lt_t *lt;
    Mix_Chunk **samples;
    size_t samples_count;
    int paused;
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

sound_samples_t *create_sound_samples(Mix_Chunk **samples,
                                      size_t samples_count)
{
    assert(samples);
    assert(samples_count > 0);

    lt_t *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    sound_samples_t *sound_samples = PUSH_LT(lt, malloc(sizeof(sound_samples_t)), free);
    if (sound_samples == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    sound_samples->samples = samples;
    sound_samples->samples_count = samples_count;
    sound_samples->paused = 0;

    sound_samples->lt = lt;

    return sound_samples;
}

void destroy_sound_samples(sound_samples_t *sound_samples)
{
    assert(sound_samples);
    RETURN_LT0(sound_samples->lt);
}

int sound_samples_play_sound(sound_samples_t *sound_samples,
                            size_t sound_index,
                            int loops)
{
    assert(sound_samples);

    if (sound_index < sound_samples->samples_count) {
        const int free_channel = mix_get_free_channel();

        printf("Found free channel: %d\n", free_channel);

        if (free_channel >= 0) {
            return Mix_PlayChannel(free_channel, sound_samples->samples[sound_index], loops);
        }
    }

    return 0;
}

int sound_samples_toggle_pause(sound_samples_t *sound_samples)
{
    assert(sound_samples);

    if (sound_samples->paused) {
        Mix_Resume(-1);
    } else {
        Mix_Pause(-1);
    }

    sound_samples->paused = !sound_samples->paused;

    return 0;
}
