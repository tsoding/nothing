#ifndef LT_ADAPTERS_H_
#define LT_ADAPTERS_H_

// libc
void fclose_lt(void* file);

// SDL
void SDL_Quit_lt(void* ignored);

// SDL_mixer
void Mix_CloseAudio_lt(void* ignored);

#endif  // LT_ADAPTERS_H_
