#include <stdlib.h>

#include <SDL2/SDL.h>

#include "system/stacktrace.h"
#include "math/point.h"
#include "./list_selector.h"

ListSelector *create_list_selector(const char **items,
                                   size_t count,
                                   Vec positiion)
{
    trace_assert(items);
    (void) count;
    (void) positiion;
    return NULL;
}

void destroy_list_selector(ListSelector *list_selector)
{
    trace_assert(list_selector);
}

int list_selector_render(const ListSelector *list_selector, SDL_Renderer *renderer)
{
    trace_assert(list_selector);
    trace_assert(renderer);
    return 0;
}

int list_selector_update(ListSelector *list_selector, float delta_time)
{
    trace_assert(list_selector);
    (void) delta_time;

    return 0;
}

int list_selector_event(ListSelector *list_selector, const SDL_Event *event)
{
    trace_assert(list_selector);
    trace_assert(event);
    return 0;
}

const char *list_selector_selected(const ListSelector *list_selector)
{
    trace_assert(list_selector);
    return NULL;
}
