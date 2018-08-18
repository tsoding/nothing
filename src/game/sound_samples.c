#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "math/pi.h"
#include "sound_samples.h"
#include "system/error.h"
#include "system/lt.h"

struct Sound_samples
{
    Lt *lt;
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

Sound_samples *create_sound_samples(const char *sample_files[],
                                      size_t sample_files_count)
{
    assert(sample_files);
    assert(sample_files_count > 0);

    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Sound_samples *sound_samples = PUSH_LT(lt, malloc(sizeof(Sound_samples)), free);
    if (sound_samples == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    sound_samples->samples = PUSH_LT(
        lt,
        malloc(sizeof(Mix_Chunk*) * sample_files_count),
        free);
    if (sound_samples->samples == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    for (size_t i = 0; i < sample_files_count; ++i) {
        sound_samples->samples[i] = PUSH_LT(
            lt,
            Mix_LoadWAV(sample_files[i]),
            Mix_FreeChunk);
        if (sound_samples->samples[i] == NULL) {
            throw_error(ERROR_TYPE_SDL2_MIXER);
            RETURN_LT(lt, NULL);
        }
    }

    sound_samples->samples_count = sample_files_count;
    sound_samples->paused = 0;

    sound_samples->lt = lt;

    return sound_samples;
}

void destroy_sound_samples(Sound_samples *sound_samples)
{
    assert(sound_samples);
    RETURN_LT0(sound_samples->lt);
}

int sound_samples_play_sound(Sound_samples *sound_samples,
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

int sound_samples_toggle_pause(Sound_samples *sound_samples)
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
