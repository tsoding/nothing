#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#include "lt_adapters.h"

void fclose_lt(void* file)
{
    fclose(file);
}

void Mix_CloseAudio_lt(void* ignored)
{
    (void) ignored;
    Mix_CloseAudio();
}

void SDL_Quit_lt(void* ignored)
{
    (void) ignored;
    SDL_Quit();
}

void TTF_Quit_lt(void* ignored)
{
    (void) ignored;
    TTF_Quit();
}
