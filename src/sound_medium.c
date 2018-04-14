#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "./sound_medium.h"
#include "./lt.h"
#include "./error.h"
#include "./pi.h"
#include "./algo.h"

#define SOUND_THRESHOLD_DISTANCE 1000.0f

struct sound_medium_t
{
    lt_t *lt;
    Mix_Chunk **samples;
    size_t samples_count;
    point_t *channel_positions;
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

sound_medium_t *create_sound_medium(Mix_Chunk **samples,
                                    size_t samples_count)
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

    sound_medium->channel_positions = PUSH_LT(lt, malloc(sizeof(point_t) * MIX_CHANNELS), free);
    if (sound_medium->channel_positions == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    sound_medium->samples = samples;
    sound_medium->samples_count = samples_count;
    sound_medium->paused = 0;

    sound_medium->lt = lt;

    return sound_medium;
}

void destroy_sound_medium(sound_medium_t *sound_medium)
{
    assert(sound_medium);
    RETURN_LT0(sound_medium->lt);
}

int sound_medium_play_sound(sound_medium_t *sound_medium,
                            size_t sound_index,
                            point_t position,
                            int loops)
{
    assert(sound_medium);
    (void) sound_index;
    (void) position;

    if (sound_index < sound_medium->samples_count) {
        const int free_channel = mix_get_free_channel();

        if (free_channel >= 0) {
            sound_medium->channel_positions[free_channel] = position;
            return Mix_PlayChannel(free_channel, sound_medium->samples[sound_index], loops);
        }
    }

    return 0;
}

int sound_medium_listen_sounds(sound_medium_t *sound_medium,
                               point_t position)
{
    assert(sound_medium);

    for (int i = 0; i < MIX_CHANNELS; ++i) {
        if (Mix_Playing(i)) {
            const vec_t v = vec_from_ps(position, sound_medium->channel_positions[i]);
            const Sint16 angle = (Sint16) roundf(rad_to_deg(vec_arg(v)));
            const Uint8 distance = (Uint8) roundf(MIN(vec_mag(v), SOUND_THRESHOLD_DISTANCE) / SOUND_THRESHOLD_DISTANCE * 255.0f);
            Mix_SetPosition(i, angle, distance);
        }
    }

    return 0;
}

int sound_medium_toggle_pause(sound_medium_t *sound_medium)
{
    assert(sound_medium);

    if (sound_medium->paused) {
        Mix_Resume(-1);
    } else {
        Mix_Pause(-1);
    }

    sound_medium->paused = !sound_medium->paused;

    return 0;
}
