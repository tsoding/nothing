#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"

static error_type_t current_error_type = ERROR_TYPE_OK;

error_type_t current_error(void)
{
    return current_error_type;
}

void throw_error(error_type_t error_type)
{
    assert(0 <= error_type && error_type < ERROR_TYPE_N);
    current_error_type = error_type;
}

void reset_error(void)
{
    current_error_type = ERROR_TYPE_OK;
}

void print_current_error_msg(const char *user_prefix)
{
    print_error_msg(current_error_type, user_prefix);
    current_error_type = ERROR_TYPE_OK;
}

void print_error_msg(error_type_t error_type, const char *user_prefix)
{
    switch (error_type) {
    case ERROR_TYPE_OK:
    case ERROR_TYPE_N:
        break;

    case ERROR_TYPE_LIBC:
        fprintf(stderr, "libc error: %s: %s\n", user_prefix, strerror(errno));
        break;

    case ERROR_TYPE_SDL2:
        fprintf(stderr, "SDL2 error: %s: %s\n", user_prefix, SDL_GetError());
        break;

    case ERROR_TYPE_SDL2_MIXER:
        fprintf(stderr, "SDL2_mixer error: %s: %s\n", user_prefix, Mix_GetError());
        break;
    }
}
