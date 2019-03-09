#include <stdlib.h>

#include <SDL2/SDL.h>

#include "system/stacktrace.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/str.h"
#include "math/point.h"

#include "./list_selector.h"

struct ListSelector
{
    Lt *lt;
    const char **items;
    size_t count;
    Vec positiion;
    float width;
};

ListSelector *create_list_selector(const char **items,
                                   size_t count,
                                   Vec positiion,
                                   float width)
{
    trace_assert(items);

    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    ListSelector *list_selector = PUSH_LT(lt, nth_alloc(sizeof(ListSelector)), free);
    if (list_selector == NULL) {
        RETURN_LT(lt, NULL);
    }
    list_selector->lt = lt;

    list_selector->items = PUSH_LT(lt, nth_calloc(count, sizeof(const char*)), free);
    if (list_selector->items == NULL) {
        RETURN_LT(lt, NULL);
    }

    for (size_t i = 0; i < count; ++i) {
        list_selector->items[i] = PUSH_LT(lt, string_duplicate(list_selector->items[i], NULL), free);
        if (list_selector->items[i] == NULL) {
            RETURN_LT(lt, NULL);
        }
    }

    list_selector->count = count;
    list_selector->positiion = positiion;
    list_selector->width = width;

    return list_selector;
}

void destroy_list_selector(ListSelector *list_selector)
{
    trace_assert(list_selector);
    RETURN_LT0(list_selector->lt);
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
