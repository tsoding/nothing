#include <assert.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "log.h"

Log *create_log(size_t capacity)
{
    (void) capacity;
    return NULL;
}

void destroy_log(Log *log)
{
    assert(log);
}

int log_render(const Log *log,
               SDL_Renderer *renderer,
               Point position)
{
    assert(log);
    assert(renderer);
    (void) position;

    return 0;
}

int log_push_line(Log *log, const char *line)
{
    assert(log);
    assert(line);

    return 0;
}
