#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "./lt.h"
#include "./sound_sample.h"
#include "./error.h"

struct sound_sample_t
{
    lt_t *lt;
    Mix_Chunk *chunk;
};

sound_sample_t *create_sound_sample_from_file(const char *filepath)
{
    assert(filepath);

    lt_t *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    sound_sample_t *sound_sample = PUSH_LT(lt, malloc(sizeof(sound_sample_t)), free);
    if (sound_sample == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    sound_sample->chunk = PUSH_LT(lt, Mix_LoadWAV(filepath), Mix_FreeChunk);
    if (sound_sample->chunk == NULL) {
        throw_error(ERROR_TYPE_SDL2_MIXER);
        RETURN_LT(lt, NULL);
    }

    sound_sample->lt = lt;

    return sound_sample;
}

void destroy_sound_sample(sound_sample_t *sound_sample)
{
    assert(sound_sample);
    RETURN_LT0(sound_sample->lt);
}

int sound_sample_play(sound_sample_t *sound_sample, int channel)
{
    assert(sound_sample);
    return Mix_PlayChannel(channel, sound_sample->chunk, 1);
}
