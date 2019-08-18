#include <SDL.h>
#include "system/stacktrace.h"
#include <stdio.h>
#include <stdlib.h>

#include "math/pi.h"
#include "sound_samples.h"
#include "system/log.h"
#include "system/lt.h"
#include "system/nth_alloc.h"

// TODO(#863): Sound_samples is not implemented
// TODO(#1022): Sound_samples does not implement volume control.

struct Sound_samples
{
    Lt *lt;
    SDL_AudioDeviceID dev;
    uint8_t **audio_buf_array;
    uint32_t *audio_buf_size_array;
    size_t samples_count;
    int paused;
};

static 
int init_buffer_and_device(Sound_samples *sound_samples, 
                           const char *sample_files[]) 
{
    // TODO(#1023): init_buffer_and_device uses hard-coded audio specification
    SDL_AudioSpec destination_spec = { // stereo float32 44100Hz
        .format = AUDIO_F32,
        .channels = 2,
        .freq = 44100
    };
    // TODO(#1024): a return value by SDL_GetNumAudioDevices that is <= 0 may not indicate an error
    if (SDL_GetNumAudioDevices(0) <= 0) {
        log_fail("No audio in 2019 LULW\n");
        return -1;
    }
    
    sound_samples->audio_buf_array = PUSH_LT(sound_samples->lt, nth_calloc(sound_samples->samples_count, sizeof(uint8_t*)), free);
    if (sound_samples->audio_buf_array == NULL) {
        log_fail("Failed to allocate memory for audio buffer pointer array\n");
        return -1;
    }
    sound_samples->audio_buf_size_array = PUSH_LT(sound_samples->lt, nth_calloc(sound_samples->samples_count, sizeof(uint32_t)), free);
    if (sound_samples->audio_buf_size_array == NULL) {
        log_fail("Failed to allocate memory for audio buffer size array\n");
        return -1;
    }
    for (size_t i = 0; i < sound_samples->samples_count; ++i) {
        uint8_t *wav_buf; uint32_t wav_buf_len; SDL_AudioSpec wav_spec;

        log_info("Loading audio file %s...\n", sample_files[i]);
        if (SDL_LoadWAV(sample_files[i], &wav_spec, &wav_buf, &wav_buf_len) == NULL) {
            log_fail("Load WAV file failed: %s\n", SDL_GetError());
            return -1;
        }
        PUSH_LT(sound_samples->lt, wav_buf, SDL_FreeWAV);
        SDL_AudioCVT cvt;
        int result = SDL_BuildAudioCVT(&cvt, wav_spec.format, (uint8_t)wav_spec.channels, (int)wav_spec.freq, 
                          destination_spec.format, (uint8_t)destination_spec.channels, (int)destination_spec.freq);
        if (result < 0) {
            log_fail("SDL_BuildAudioCVT failed: %s\n", SDL_GetError());
            return -1;
        } else if (result == 0) { // no need to do conversion
            sound_samples->audio_buf_array[i] = wav_buf;
            sound_samples->audio_buf_size_array[i] = wav_buf_len;
        } else {
            cvt.len = (int)wav_buf_len;
            cvt.buf = PUSH_LT(sound_samples->lt, malloc((size_t)(cvt.len * cvt.len_mult)), free);
            if (cvt.buf == NULL) {
                log_fail("Allocating buffer for conversion failed\n");
                return -1;
            }
            memcpy(cvt.buf, wav_buf, (size_t)cvt.len);
            SDL_FreeWAV(RELEASE_LT(sound_samples->lt, wav_buf));
            if (SDL_ConvertAudio(&cvt) < 0) {
                log_fail("SDL_ConvertAudio failed: %s\n", SDL_GetError());
                return -1;
            }
            sound_samples->audio_buf_array[i] = cvt.buf;
            sound_samples->audio_buf_size_array[i] = (uint32_t)cvt.len_cvt;
        }
    }
    
    sound_samples->dev = SDL_OpenAudioDevice(NULL, 0, &destination_spec, NULL, 0);
    if (sound_samples->dev == 0) {
        log_fail("SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        log_info("The audio device may not support the hardcoded format\n");
        return -1;
    }
    log_info("Audio device ID %u opened\n", sound_samples->dev);
    return 0;
}

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
    if (init_buffer_and_device(sound_samples, sample_files) < 0) {
        log_fail("init_buffer_and_device failed\n");
        RETURN_LT(lt, NULL);
    }

    sound_samples->paused = 0;
    SDL_PauseAudioDevice(sound_samples->dev, 0);

    return sound_samples;
}

void destroy_sound_samples(Sound_samples *sound_samples)
{
    // TODO(#1025): Use a seperate callback function for audio handling and pass that into SDL_OpenAudioDevice
    trace_assert(sound_samples);
    trace_assert(sound_samples->dev);
    SDL_CloseAudioDevice(sound_samples->dev);
    RETURN_LT0(sound_samples->lt);
}

int sound_samples_play_sound(Sound_samples *sound_samples,
                             size_t sound_index)
{
    trace_assert(sound_samples);
    trace_assert(sound_index < sound_samples->samples_count);
    trace_assert(sound_samples->dev);
    SDL_ClearQueuedAudio(sound_samples->dev);
    if (SDL_QueueAudio(sound_samples->dev, sound_samples->audio_buf_array[sound_index], 
                sound_samples->audio_buf_size_array[sound_index]) < 0) {
        log_warn("Failed to queue audio data of sound index %zu to device: %s\n", sound_index, SDL_GetError());
        return 0;
    }
    SDL_PauseAudioDevice(sound_samples->dev, 0);
    return 0;
}

int sound_samples_toggle_pause(Sound_samples *sound_samples)
{
    trace_assert(sound_samples);
    sound_samples->paused = !sound_samples->paused;
    trace_assert(sound_samples->dev);
    SDL_PauseAudioDevice(sound_samples->dev, sound_samples->paused);
    return 0;
}
