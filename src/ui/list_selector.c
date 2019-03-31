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
    size_t cursor;
    int selected_item;
    Vec position;
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
    list_selector->items = items;
    list_selector->count = count;
    list_selector->cursor = 0;
    list_selector->selected_item = -1;
    list_selector->position = vec(0.0f, 0.0f);

    return list_selector;
}

void destroy_list_selector(ListSelector *list_selector)
{
    trace_assert(list_selector);
    RETURN_LT0(list_selector->lt);
}

int list_selector_render(const ListSelector *list_selector,
                         SDL_Renderer *renderer,
                         Vec font_scale,
                         float padding_bottom)
{
    trace_assert(list_selector);
    trace_assert(renderer);

    for (size_t i = 0; i < list_selector->count; ++i) {
        const Vec current_position = vec_sum(
            list_selector->position,
            vec(0.0f, (float) i * ((float) FONT_CHAR_HEIGHT * font_scale.y + padding_bottom)));

        if (sprite_font_render_text(
                list_selector->sprite_font,
                renderer,
                current_position,
                font_scale,
                rgba(1.0f, 1.0f, 1.0f, 1.0f),
                list_selector->items[i]) < 0) {
            return -1;
        }

        if (i == list_selector->cursor) {
            SDL_Rect boundary_box = rect_for_sdl(
                sprite_font_boundary_box(
                    list_selector->sprite_font,
                    current_position,
                    font_scale,
                    list_selector->items[i]));
            if (SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255) < 0) {
                return -1;
            }

            if (SDL_RenderDrawRect(renderer, &boundary_box) < 0) {
                return -1;
            }
        }
    }

    return 0;
}

Vec list_selector_size(const ListSelector *list_selector,
                       Vec font_scale,
                       float padding_bottom)
{
    trace_assert(list_selector);

    Vec result = vec(0.0f, 0.0f);

    for (size_t i = 0; i < list_selector->count; ++i) {
        Rect boundary_box = sprite_font_boundary_box(
            list_selector->sprite_font,
            vec(0.0f, 0.0f),
            font_scale,
            list_selector->items[i]);

        result.x = fmaxf(result.x, boundary_box.w);
        result.y += boundary_box.y + padding_bottom;
    }

    return result;
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

    switch (event->type) {
    case SDL_KEYDOWN:
        switch (event->key.keysym.sym) {
        case SDLK_UP:
            if (list_selector->cursor > 0) {
                list_selector->cursor--;
            }
            break;
        case SDLK_DOWN:
            if (list_selector->cursor < list_selector->count - 1) {
                list_selector->cursor++;
            }
            break;
        case SDLK_RETURN:
            list_selector->selected_item = (int) list_selector->cursor;
            break;
        }
        break;
    // case SDL_MOUSEMOTION: {
    //     const Vec mouse_pos = vec((float)event->motion.x, (float)event->motion.y);
    //     for (size_t i = 0; i < list_selector->count; ++i) {
    //         Rect boundary_box = sprite_font_boundary_box(
    //             list_selector->sprite_font,
    //             vec(0.0f, 0.0f),
    //             font_scale,
    //             list_selector->items[i]);
    //     }
    // } break;
    }

    return 0;
}

int list_selector_selected(const ListSelector *list_selector)
{
    trace_assert(list_selector);
    return list_selector->selected_item;
}

void list_selector_clean_selection(ListSelector *list_selector)
{
    trace_assert(list_selector);
    list_selector->selected_item = -1;
}

void list_selector_move(ListSelector *list_selector, Vec position)
{
    list_selector->position = position;
}
