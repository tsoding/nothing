#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "system/stacktrace.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/str.h"
#include "math/vec.h"
#include "game/sprite_font.h"
#include "system/log.h"

#include "./list_selector.h"


int list_selector_render(const Camera *camera,
                         const ListSelector *list_selector)
{
    trace_assert(camera);
    trace_assert(list_selector);

    for (size_t i = 0; i < list_selector->items.count; ++i) {
        const Vec2f current_position = vec_sum(
            list_selector->position,
            vec(0.0f, (float) i * ((float) FONT_CHAR_HEIGHT * list_selector->font_scale.y + list_selector->padding_bottom)));

        const char *item_text =
            list_selector->list_item_text(
                dynarray_pointer_at(
                    &list_selector->items,
                    i));

        sprite_font_render_text(
            &camera->font,
            camera->renderer,
            current_position,
            list_selector->font_scale,
            rgba(1.0f, 1.0f, 1.0f, 1.0f),
            item_text);

        if (i == list_selector->cursor) {
            SDL_Rect boundary_box = rect_for_sdl(
                sprite_font_boundary_box(
                    current_position,
                    list_selector->font_scale,
                    strlen(item_text)));
            if (SDL_SetRenderDrawColor(camera->renderer, 255, 255, 255, 255) < 0) {
                return -1;
            }

            if (SDL_RenderDrawRect(camera->renderer, &boundary_box) < 0) {
                return -1;
            }
        }
    }

    return 0;
}

Vec2f list_selector_size(const ListSelector *list_selector,
                         Vec2f font_scale,
                         float padding_bottom)
{
    trace_assert(list_selector);

    Vec2f result = vec(0.0f, 0.0f);

    for (size_t i = 0; i < list_selector->items.count; ++i) {
        const char *item_text =
            list_selector->list_item_text(
                    dynarray_pointer_at(
                        &list_selector->items,
                        i));

        Rect boundary_box = sprite_font_boundary_box(
            vec(0.0f, 0.0f),
            font_scale,
            strlen(item_text));

        result.x = fmaxf(result.x, boundary_box.w);
        result.y += boundary_box.y + padding_bottom;
    }

    return result;
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
            if (list_selector->cursor < list_selector->items.count - 1) {
                list_selector->cursor++;
            }
            break;
        case SDLK_RETURN:
            list_selector->selected_item = (int) list_selector->cursor;
            break;
        }
        break;

    case SDL_MOUSEMOTION: {
        const Vec2f mouse_pos = vec((float) event->motion.x, (float) event->motion.y);
        Vec2f position = list_selector->position;

        for (size_t i = 0; i < list_selector->items.count; ++i) {
            const char *item_text =
                list_selector->list_item_text(
                        dynarray_pointer_at(
                            &list_selector->items,
                            i));

            Rect boundary_box = sprite_font_boundary_box(
                position,
                list_selector->font_scale,
                strlen(item_text));

            if (rect_contains_point(boundary_box, mouse_pos)) {
                list_selector->cursor = i;
            }

            position.y += boundary_box.h + list_selector->padding_bottom;
        }
    } break;

    case SDL_MOUSEBUTTONDOWN: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            // check if the click position was actually inside...
            // note: make sure there's actually stuff in the list! tsoding likes
            // to remove all levels and change title to "SMOL BREAK"...
            if (list_selector->items.count == 0)
                break;

            // note: this assumes that all list items are the same height!
            // this is probably a valid assumption as long as we use a sprite font.
            float single_item_height =
                FONT_CHAR_HEIGHT * list_selector->font_scale.y + list_selector->padding_bottom;

            Vec2f position = list_selector->position;
            vec_add(&position, vec(0.0f, (float) list_selector->cursor * single_item_height));

            const char *item_text =
                list_selector->list_item_text(
                    dynarray_pointer_at(
                        &list_selector->items,
                        list_selector->cursor));

            Rect boundary_box = sprite_font_boundary_box(
                position,
                list_selector->font_scale,
                strlen(item_text));

            const Vec2f mouse_pos = vec((float) event->motion.x, (float) event->motion.y);
            if (rect_contains_point(boundary_box, mouse_pos)) {
                list_selector->selected_item = (int) list_selector->cursor;
            }
        } break;
        }
    } break;
    }

    return 0;
}
