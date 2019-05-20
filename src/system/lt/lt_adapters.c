#include <SDL2/SDL.h>
#include <stdio.h>

#include "lt_adapters.h"

void SDL_Quit_lt(void* ignored)
{
    (void) ignored;
    SDL_Quit();
}
