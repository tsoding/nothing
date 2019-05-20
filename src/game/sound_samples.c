#include <SDL2/SDL.h>
#include "system/stacktrace.h"
#include <stdio.h>
#include <stdlib.h>

#include "math/pi.h"
#include "sound_samples.h"
#include "system/log.h"
#include "system/lt.h"
#include "system/nth_alloc.h"

// TODO: Sound_samples is not implemented

struct Sound_samples
{
    Lt *lt;
    size_t samples_count;
    int paused;
};

Sound_samples *create_sound_samples(const char *sample_files[],
                                    size_t sample_files_count)
{
    trace_assert(sample_files);
    trace_assert(sample_files_count > 0);

    Lt *lt = create_lt();

    Sound_samples *sound_samples = PUSH_LT(lt, nth_calloc(1, sizeof(Sound_samples)), free);
    if (sound_samples == NULL) {
        RETURN_LT(lt, NULL);
    }
    sound_samples->lt = lt;

    sound_samples->samples_count = sample_files_count;
    sound_samples->paused = 0;

    return sound_samples;
}

void destroy_sound_samples(Sound_samples *sound_samples)
{
    trace_assert(sound_samples);
    RETURN_LT0(sound_samples->lt);
}

int sound_samples_play_sound(Sound_samples *sound_samples,
                             size_t sound_index)
{
    trace_assert(sound_samples);
    (void) sound_index;
    return 0;
}

int sound_samples_toggle_pause(Sound_samples *sound_samples)
{
    trace_assert(sound_samples);
    sound_samples->paused = !sound_samples->paused;
    return 0;
}
