#include <stdio.h>

#include <SDL.h>

#include "dynarray.h"
#include "game/camera.h"
#include "system/log.h"
#include "system/nth_alloc.h"
#include "system/stacktrace.h"
#include "system/str.h"
#include "ui/edit_field.h"
#include "./point_layer.h"
#include "math/extrema.h"
#include "math/mat3x3.h"
#include "./color_picker.h"
#include "undo_history.h"

#define POINT_LAYER_ELEMENT_RADIUS 10.0f
#define POINT_LAYER_ID_TEXT_SIZE vec(2.0f, 2.0f)
#define POINT_LAYER_ID_TEXT_COLOR COLOR_BLACK

static int point_clipboard = 0;
static Color point_clipboard_color;

// TODO(#1140): PointLayer does not support snapping

typedef enum {
    POINT_UNDO_ADD,
    POINT_UNDO_DELETE,
    POINT_UNDO_UPDATE,
    POINT_UNDO_SWAP
} PointUndoType;

typedef struct {
    PointUndoType type;
    PointLayer *layer;
    Vec2f position;
    Color color;
    char id[ID_MAX_SIZE];
    size_t index;
    size_t index2;
} PointUndoContext;

static
PointUndoContext create_point_undo_swap_context(PointLayer *point_layer,
                                     size_t index, size_t index2)
{
    trace_assert(point_layer);
    trace_assert(index < point_layer->positions.count);
    trace_assert(index2 < point_layer->positions.count);

    PointUndoContext undo_context;
    undo_context.type = POINT_UNDO_SWAP;
    undo_context.layer = point_layer;
    undo_context.index = index;
    undo_context.index2 = index2;
    return undo_context;
}

static
PointUndoContext create_point_undo_context(PointLayer *point_layer,
                                PointUndoType type)
{
    trace_assert(type != POINT_UNDO_SWAP);

    (void) create_point_undo_swap_context;

    PointUndoContext undo_context;

    size_t index =
        type == POINT_UNDO_ADD
        ? point_layer->positions.count - 1
        : (size_t) point_layer->selection;

    undo_context.type = type;
    undo_context.layer = point_layer;
    dynarray_copy_to(&point_layer->positions, &undo_context.position, index);
    dynarray_copy_to(&point_layer->colors, &undo_context.color, index);
    dynarray_copy_to(&point_layer->ids, &undo_context.id, index);
    undo_context.index = index;

    return undo_context;
}

static
void point_layer_undo(void *context, size_t context_size)
{
    trace_assert(context);
    trace_assert(sizeof(PointUndoContext) == context_size);

    PointUndoContext *undo_context = context;
    PointLayer *point_layer = undo_context->layer;

    switch (undo_context->type) {
    case POINT_UNDO_ADD: {
        dynarray_pop(&point_layer->positions, NULL);
        dynarray_pop(&point_layer->colors, NULL);
        dynarray_pop(&point_layer->ids, NULL);
        point_layer->selection = -1;
    } break;

    case POINT_UNDO_DELETE: {
        dynarray_insert_before(&point_layer->positions, undo_context->index, &undo_context->position);
        dynarray_insert_before(&point_layer->colors, undo_context->index, &undo_context->color);
        dynarray_insert_before(&point_layer->ids, undo_context->index, &undo_context->id);
        point_layer->selection = -1;
    } break;

    case POINT_UNDO_UPDATE: {
        dynarray_replace_at(&point_layer->positions, undo_context->index, &undo_context->position);
        dynarray_replace_at(&point_layer->colors, undo_context->index, &undo_context->color);
        dynarray_replace_at(&point_layer->ids, undo_context->index, &undo_context->id);
    } break;

    case POINT_UNDO_SWAP: {
        dynarray_swap(&point_layer->positions, undo_context->index, undo_context->index2);
        dynarray_swap(&point_layer->colors, undo_context->index, undo_context->index2);
        dynarray_swap(&point_layer->ids, undo_context->index, undo_context->index2);
    } break;
    }
}

#define POINT_UNDO_PUSH(HISTORY, CONTEXT)                                     \
    do {                                                                \
        PointUndoContext context = (CONTEXT);                                \
        undo_history_push(                                              \
            HISTORY,                                                    \
            point_layer_undo,                                           \
            &context,                                                   \
            sizeof(context));                                           \
    } while(0)

LayerPtr point_layer_as_layer(PointLayer *point_layer)
{
    LayerPtr layer = {
        .type = LAYER_POINT,
        .ptr = point_layer
    };
    return layer;
}

PointLayer *create_point_layer(Memory *memory,
                               const char *id_name_prefix)
{
    trace_assert(memory);
    trace_assert(id_name_prefix);

    PointLayer *result = memory_alloc(memory, sizeof(PointLayer));
    memset(result, 0, sizeof(PointLayer));
    result->state = POINT_LAYER_IDLE;
    result->positions = create_dynarray(memory, sizeof(Vec2f));
    result->colors = create_dynarray(memory, sizeof(Color));
    result->ids = create_dynarray(memory, sizeof(char) * ID_MAX_SIZE);
    result->edit_field.font_size = POINT_LAYER_ID_TEXT_SIZE;
    result->edit_field.font_color = POINT_LAYER_ID_TEXT_COLOR;
    result->id_name_prefix = id_name_prefix;
    return result;
}

void point_layer_load(PointLayer *point_layer,
                      Memory *memory,
                      String *input)
{
    trace_assert(point_layer);
    trace_assert(memory);
    trace_assert(input);

    int n = atoi(string_to_cstr(memory, trim(chop_by_delim(input, '\n'))));
    char id[ENTITY_MAX_ID_SIZE];
    for (int i = 0; i < n; ++i) {
        String line = trim(chop_by_delim(input, '\n'));
        String string_id = trim(chop_word(&line));
        Vec2f point;
        point.x = strtof(string_to_cstr(memory, trim(chop_word(&line))), NULL);
        point.y = strtof(string_to_cstr(memory, trim(chop_word(&line))), NULL);
        Color color = hexs(trim(chop_word(&line)));

        memset(id, 0, ENTITY_MAX_ID_SIZE);
        memcpy(id, string_id.data, min_size_t(ENTITY_MAX_ID_SIZE - 1, string_id.count));

        dynarray_push(&point_layer->positions, &point);
        dynarray_push(&point_layer->colors, &color);
        dynarray_push(&point_layer->ids, id);
    }
}

static inline
Triangle element_shape(Vec2f position, float scale)
{
    return triangle_mat3x3_product(
        equilateral_triangle(),
        mat3x3_product(
            trans_mat_vec(position),
            scale_mat(scale)));
}

int point_layer_render(const PointLayer *point_layer,
                       const Camera *camera,
                       int active)
{
    trace_assert(point_layer);
    trace_assert(camera);

    const int n = (int)point_layer->positions.count;
    Vec2f *positions = (Vec2f *)point_layer->positions.data;
    Color *colors = (Color *)point_layer->colors.data;
    char *ids = (char *)point_layer->ids.data;

    for (int i = 0; i < n; ++i) {
        const Color color = color_scale(
            point_layer->state == POINT_LAYER_RECOLOR && i == point_layer->selection
            ? point_layer->inter_color
            : colors[i],
            rgba(1.0f, 1.0f, 1.0f, active ? 1.0f : 0.5f));

        const Vec2f position =
            point_layer->state == POINT_LAYER_MOVE && i == point_layer->selection
            ? point_layer->inter_position
            : positions[i];

        // Selection Layer
        if (active && i == point_layer->selection) {
            if (camera_fill_triangle(
                    camera,
                    element_shape(
                        position,
                        POINT_LAYER_ELEMENT_RADIUS + 5.0f),
                    color_invert(color)) < 0) {
                return -1;
            }

            if (point_layer->state != POINT_LAYER_EDIT_ID &&
                camera_render_text(
                    camera,
                    ids + ID_MAX_SIZE * i,
                    POINT_LAYER_ID_TEXT_SIZE,
                    POINT_LAYER_ID_TEXT_COLOR,
                    position) < 0) {
                return -1;
            }
        }

        if (camera_fill_triangle(
                camera,
                element_shape(
                    position,
                    POINT_LAYER_ELEMENT_RADIUS),
                color) < 0) {
            return -1;
        }
    }

    if (point_layer->state == POINT_LAYER_EDIT_ID) {
        if (edit_field_render_world(
                &point_layer->edit_field,
                camera,
                positions[point_layer->selection]) < 0) {
            return -1;
        }
    }

    if (active && color_picker_render(&point_layer->color_picker, camera) < 0) {
        return -1;
    }


    return 0;
}

static
int point_layer_element_at(const PointLayer *point_layer,
                           Vec2f position)
{
    trace_assert(point_layer);

    int n = (int) point_layer->positions.count;
    Vec2f *positions = (Vec2f *)point_layer->positions.data;

    for (int i = n - 1; i >= 0; --i) {
        if (vec_length(vec_sub(positions[i], position)) < POINT_LAYER_ELEMENT_RADIUS) {
            return i;
        }
    }

    return -1;
}

static
int point_layer_add_element(PointLayer *point_layer,
                            Vec2f position,
                            Color color,
                            UndoHistory *undo_history)
{
    trace_assert(point_layer);
    trace_assert(undo_history);

    char id[ID_MAX_SIZE];
    snprintf(id, ID_MAX_SIZE, "%s_%d",
             point_layer->id_name_prefix,
             point_layer->id_name_counter++);

    dynarray_push(&point_layer->positions, &position);
    dynarray_push(&point_layer->colors, &color);
    dynarray_push(&point_layer->ids, id);

    POINT_UNDO_PUSH(
        undo_history,
        create_point_undo_context(point_layer, POINT_UNDO_ADD));

    return 0;
}

static
void point_layer_swap_elements(PointLayer *point_layer,
                               size_t a, size_t b,
                               UndoHistory *undo_history)
{
    trace_assert(point_layer);
    trace_assert(undo_history);
    trace_assert(a < point_layer->positions.count);
    trace_assert(b < point_layer->positions.count);

    dynarray_swap(&point_layer->positions, a, b);
    dynarray_swap(&point_layer->colors, a, b);
    dynarray_swap(&point_layer->ids, a, b);

    POINT_UNDO_PUSH(
        undo_history,
        create_point_undo_swap_context(point_layer, a, b));
}

static
void point_layer_delete_nth_element(PointLayer *point_layer,
                                    size_t i,
                                    UndoHistory *undo_history)
{
    trace_assert(point_layer);

    POINT_UNDO_PUSH(
        undo_history,
        create_point_undo_context(
            point_layer,
            POINT_UNDO_DELETE));

    dynarray_delete_at(&point_layer->positions, i);
    dynarray_delete_at(&point_layer->colors, i);
    dynarray_delete_at(&point_layer->ids, i);
}

static
int point_layer_idle_event(PointLayer *point_layer,
                           const SDL_Event *event,
                           const Camera *camera,
                           UndoHistory *undo_history)
{
    trace_assert(point_layer);
    trace_assert(event);
    trace_assert(camera);

    int selected = 0;
    if (color_picker_event(
            &point_layer->color_picker,
            event,
            camera,
            &selected) < 0) {
        return -1;
    }

    if (selected) {
        if (point_layer->selection >= 0) {
            point_layer->inter_color = color_picker_rgba(&point_layer->color_picker);
            point_layer->state = POINT_LAYER_RECOLOR;
        }
        return 0;
    }

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            const Vec2f position = camera_map_screen(camera, event->button.x, event->button.y);

            point_layer->selection = point_layer_element_at(
                point_layer, position);

            if (point_layer->selection < 0) {
                point_layer_add_element(
                    point_layer,
                    position,
                    color_picker_rgba(&point_layer->color_picker),
                    undo_history);
            } else {
                Color *colors = (Color*)point_layer->colors.data;
                Vec2f *positions = (Vec2f*)point_layer->positions.data;

                point_layer->state = POINT_LAYER_MOVE;
                point_layer->color_picker =
                    create_color_picker_from_rgba(colors[point_layer->selection]);
                point_layer->inter_position = positions[point_layer->selection];
            }
        } break;
        }
    } break;

    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_UP: {
            if ((event->key.keysym.mod & KMOD_SHIFT)
                && (point_layer->selection >= 0)
                && ((size_t)(point_layer->selection + 1) < point_layer->positions.count)) {
                point_layer_swap_elements(
                    point_layer,
                    (size_t) point_layer->selection,
                    (size_t) point_layer->selection + 1,
                    undo_history);
                point_layer->selection++;
            }
        } break;

        case SDLK_DOWN: {
            if ((event->key.keysym.mod & KMOD_SHIFT)
                && (point_layer->selection > 0)
                && ((size_t) point_layer->selection < point_layer->positions.count)) {
                point_layer_swap_elements(
                    point_layer,
                    (size_t) point_layer->selection,
                    (size_t) point_layer->selection - 1,
                    undo_history);
                point_layer->selection--;
            }
        } break;

        case SDLK_DELETE: {
            if (0 <= point_layer->selection && point_layer->selection < (int) point_layer->positions.count) {
                point_layer_delete_nth_element(
                    point_layer,
                    (size_t)point_layer->selection,
                    undo_history);
                point_layer->selection = -1;
            }
        } break;

        case SDLK_F2: {
            if (point_layer->selection >= 0) {
                char *ids = (char*)point_layer->ids.data;
                point_layer->state = POINT_LAYER_EDIT_ID;
                edit_field_replace(
                    &point_layer->edit_field,
                    ids + ID_MAX_SIZE * point_layer->selection);
                SDL_StartTextInput();
            }
        } break;

        case SDLK_c: {
            if ((event->key.keysym.mod & KMOD_LCTRL) && point_layer->selection >= 0) {
                point_clipboard = 1;
                dynarray_copy_to(&point_layer->colors, &point_clipboard_color, (size_t)point_layer->selection);
            }
        } break;

        case SDLK_v: {
            if ((event->key.keysym.mod & KMOD_LCTRL) && point_clipboard) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                Vec2f position = camera_map_screen(camera, x, y);

                point_layer_add_element(
                    point_layer,
                    position,
                    point_clipboard_color,
                    undo_history);
            }
        } break;
        }
    } break;
    }

    return 0;
}

static
int point_layer_edit_id_event(PointLayer *point_layer,
                              const SDL_Event *event,
                              const Camera *camera,
                              UndoHistory *undo_history)
{
    trace_assert(point_layer);
    trace_assert(event);
    trace_assert(camera);

    switch (event->type) {
    case SDL_KEYDOWN: {
        switch(event->key.keysym.sym) {
        case SDLK_RETURN: {
            POINT_UNDO_PUSH(
                undo_history,
                create_point_undo_context(
                    point_layer,
                    POINT_UNDO_UPDATE));

            char *id = dynarray_pointer_at(&point_layer->ids, (size_t) point_layer->selection);
            const char *text = edit_field_as_text(&point_layer->edit_field);
            size_t n = min_size_t(strlen(text), ID_MAX_SIZE - 1);
            memcpy(id, text, n);
            memset(id + n, 0, ID_MAX_SIZE - n);

            point_layer->state = POINT_LAYER_IDLE;
            SDL_StopTextInput();
            return 0;
        } break;

        case SDLK_ESCAPE: {
            point_layer->state = POINT_LAYER_IDLE;
            SDL_StopTextInput();
            return 0;
        } break;
        }
    } break;
    }

    return edit_field_event(&point_layer->edit_field, event);
}

static
int point_layer_move_event(PointLayer *point_layer,
                           const SDL_Event *event,
                           const Camera *camera,
                           UndoHistory *undo_history)
{
    trace_assert(point_layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(point_layer->selection >= 0);

    Vec2f *positions = (Vec2f*)point_layer->positions.data;

    switch (event->type) {
    case SDL_MOUSEBUTTONUP: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            point_layer->state = POINT_LAYER_IDLE;

            const float distance = vec_length(
                vec_sub(point_layer->inter_position,
                        positions[point_layer->selection]));

            if (distance > 1e-6) {
                POINT_UNDO_PUSH(
                    undo_history,
                    create_point_undo_context(
                        point_layer,
                        POINT_UNDO_UPDATE));

                dynarray_replace_at(
                    &point_layer->positions,
                    (size_t) point_layer->selection,
                    &point_layer->inter_position);
            }
        } break;
        }
    } break;

    case SDL_MOUSEMOTION: {
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        const Vec2f mouse_pos = camera_map_screen(camera, event->motion.x, event->motion.y);
        const Vec2f point_pos = positions[point_layer->selection];

        if (!(state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL])) {
            point_layer->inter_position = mouse_pos;
        } else {
            const float dx = fabsf(point_pos.x - mouse_pos.x);
            const float dy = fabsf(point_pos.y - mouse_pos.y);

            if (dx > dy) {
                point_layer->inter_position = vec(mouse_pos.x, point_pos.y);
            } else {
                point_layer->inter_position = vec(point_pos.x, mouse_pos.y);
            }
        }
    } break;
    }

    return 0;
}

static
int point_layer_recolor_event(PointLayer *point_layer,
                              const SDL_Event *event,
                              const Camera *camera,
                              UndoHistory *undo_history)
{
    trace_assert(point_layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(undo_history);
    trace_assert(point_layer->selection >= 0);

    int selected = 0;
    if (color_picker_event(
            &point_layer->color_picker,
            event,
            camera,
            &selected) < 0) {
        return -1;
    }

    if (selected) {
        point_layer->inter_color = color_picker_rgba(&point_layer->color_picker);

        if (!color_picker_drag(&point_layer->color_picker)) {
            POINT_UNDO_PUSH(
                undo_history,
                create_point_undo_context(
                    point_layer,
                    POINT_UNDO_UPDATE));

            dynarray_replace_at(
                &point_layer->colors,
                (size_t) point_layer->selection,
                &point_layer->inter_color);

            point_layer->state = POINT_LAYER_IDLE;
        }
    }


    return 0;
}

int point_layer_event(PointLayer *point_layer,
                      const SDL_Event *event,
                      const Camera *camera,
                      UndoHistory *undo_history)
{
    trace_assert(point_layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(undo_history);

    switch (point_layer->state) {
    case POINT_LAYER_IDLE:
        return point_layer_idle_event(point_layer, event, camera, undo_history);

    case POINT_LAYER_EDIT_ID:
        return point_layer_edit_id_event(point_layer, event, camera, undo_history);

    case POINT_LAYER_MOVE:
        return point_layer_move_event(point_layer, event, camera, undo_history);

    case POINT_LAYER_RECOLOR:
        return point_layer_recolor_event(point_layer, event, camera, undo_history);
    }

    return 0;
}

size_t point_layer_count(const PointLayer *point_layer)
{
    trace_assert(point_layer);
    return point_layer->positions.count;
}

const Vec2f *point_layer_positions(const PointLayer *point_layer)
{
    trace_assert(point_layer);
    return (const Vec2f *)point_layer->positions.data;
}

const Color *point_layer_colors(const PointLayer *point_layer)
{
    trace_assert(point_layer);
    return (const Color *)point_layer->colors.data;
}

const char *point_layer_ids(const PointLayer *point_layer)
{
    trace_assert(point_layer);
    return (const char *)point_layer->ids.data;
}

int point_layer_dump_stream(const PointLayer *point_layer,
                            FILE *filedump)
{
    trace_assert(point_layer);
    trace_assert(filedump);

    size_t n = point_layer->ids.count;
    char *ids = (char *) point_layer->ids.data;
    Vec2f *positions = (Vec2f *) point_layer->positions.data;
    Color *colors = (Color *) point_layer->colors.data;

    fprintf(filedump, "%zd\n", n);
    for (size_t i = 0; i < n; ++i) {
        fprintf(filedump, "%s %f %f ",
                ids + ID_MAX_SIZE * i,
                positions[i].x, positions[i].y);
        color_hex_to_stream(colors[i], filedump);
        fprintf(filedump, "\n");
    }

    return 0;
}
