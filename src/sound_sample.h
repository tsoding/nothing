#ifndef SOUND_SAMPLE_H_
#define SOUND_SAMPLE_H_

typedef struct sound_sample_t sound_sample_t;

sound_sample_t *create_sound_sample_from_file(const char *filepath);
void destroy_sound_sample(sound_sample_t *sound_sample);
int sound_sample_play(sound_sample_t *sound_sample, int channel);

#endif  // SOUND_SAMPLE_H_
