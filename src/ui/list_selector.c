#include <stdlib.h>

#include <SDL2/SDL.h>

#include "system/stacktrace.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/str.h"
#include "math/point.h"
#include "game/sprite_font.h"

#include "./list_selector.h"

struct ListSelector
{
    Lt *lt;
    const Sprite_font *sprite_font;
    const char **items;
    size_t count;
};

ListSelector *create_list_selector(const Sprite_font *sprite_font,
                                   const char *items[],
                                   size_t count)
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

    list_selector->sprite_font = sprite_font;

    list_selector->items = PUSH_LT(lt, nth_calloc(count, sizeof(const char*)), free);
    if (list_selector->items == NULL) {
        RETURN_LT(lt, NULL);
    }

    for (size_t i = 0; i < count; ++i) {
        list_selector->items[i] = PUSH_LT(lt, string_duplicate(items[i], NULL), free);
        if (list_selector->items[i] == NULL) {
            RETURN_LT(lt, NULL);
        }
    }

    list_selector->count = count;

    return list_selector;
}

void destroy_list_selector(ListSelector *list_selector)
{
    trace_assert(list_selector);
    RETURN_LT0(list_selector->lt);
}

int list_selector_render(const ListSelector *list_selector,
                         SDL_Renderer *renderer,
                         Vec position,
                         float width)
{
    trace_assert(list_selector);
    trace_assert(renderer);
    (void) width;

    // TODO: ListSelector.width is ignored in rendering

    for (size_t i = 0; i < list_selector->count; ++i) {
        if (sprite_font_render_text(
                list_selector->sprite_font,
                renderer,
                vec_sum(
                    position,
                    vec(0.0f, (float) (i * FONT_CHAR_HEIGHT))),
                vec(1.0f, 1.0f),
                rgba(1.0f, 1.0f, 1.0f, 1.0f),
                list_selector->items[i]) < 0) {
            return -1;
        }
    }

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
