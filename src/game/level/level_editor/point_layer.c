#include <stdio.h>

#include <SDL.h>

#include "dynarray.h"
#include "game/camera.h"
#include "system/line_stream.h"
#include "system/log.h"
#include "system/lt.h"
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

static int clipboard = 0;
static Color clipboard_color;

typedef enum {
    POINT_LAYER_IDLE = 0,
    POINT_LAYER_EDIT_ID,
    POINT_LAYER_MOVE,
    POINT_LAYER_RECOLOR
} PointLayerState;

struct PointLayer
{
    Lt *lt;
    PointLayerState state;
    Dynarray/*<Point>*/ *positions;
    Dynarray/*<Color>*/ *colors;
    Dynarray/*<char[ID_MAX_SIZE]>*/ *ids;
    int selected;
    ColorPicker color_picker;

    Point inter_position;
    Color inter_color;
    Edit_field *edit_field;
};

typedef enum {
    UNDO_ADD,
    UNDO_DELETE,
    UNDO_UPDATE
} UndoType;

typedef struct {
    UndoType type;
    PointLayer *layer;
    Point position;
    Color color;
    char id[ID_MAX_SIZE];
    size_t index;
} UndoContext;

static
UndoContext point_layer_create_undo_context(PointLayer *point_layer,
                                            UndoType type)
{
    UndoContext undo_context;

    size_t index =
        type == UNDO_ADD
        ? dynarray_count(point_layer->positions) - 1
        : (size_t) point_layer->selected;

    undo_context.type = type;
    undo_context.layer = point_layer;
    dynarray_copy_to(point_layer->positions, &undo_context.position, index);
    dynarray_copy_to(point_layer->colors, &undo_context.color, index);
    dynarray_copy_to(point_layer->ids, &undo_context.id, index);
    undo_context.index = index;

    return undo_context;
}

static
void point_layer_undo(void *context, size_t context_size)
{
    trace_assert(context);
    trace_assert(sizeof(UndoContext) == context_size);

    UndoContext *undo_context = context;
    PointLayer *point_layer = undo_context->layer;

    switch (undo_context->type) {
    case UNDO_ADD: {
        dynarray_pop(point_layer->positions, NULL);
        dynarray_pop(point_layer->colors, NULL);
        dynarray_pop(point_layer->ids, NULL);
        point_layer->selected = -1;
    } break;

    case UNDO_DELETE: {
        dynarray_insert_before(point_layer->positions, undo_context->index, &undo_context->position);
        dynarray_insert_before(point_layer->colors, undo_context->index, &undo_context->color);
        dynarray_insert_before(point_layer->ids, undo_context->index, &undo_context->id);
        point_layer->selected = -1;
    } break;

    case UNDO_UPDATE: {
        dynarray_replace_at(point_layer->positions, undo_context->index, &undo_context->position);
        dynarray_replace_at(point_layer->colors, undo_context->index, &undo_context->color);
        dynarray_replace_at(point_layer->ids, undo_context->index, &undo_context->id);
    } break;
    }
}

#define UNDO_PUSH(LAYER, HISTORY, UNDO_TYPE)                            \
    do {                                                                \
        UndoContext context = point_layer_create_undo_context(LAYER, UNDO_TYPE); \
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

PointLayer *create_point_layer(void)
{
    Lt *lt = create_lt();

    PointLayer *point_layer = PUSH_LT(lt, nth_calloc(1, sizeof(PointLayer)), free);
    if (point_layer == NULL) {
        RETURN_LT(lt, NULL);
    }
    point_layer->lt = lt;

    point_layer->state = POINT_LAYER_IDLE;

    point_layer->positions = PUSH_LT(lt, create_dynarray(sizeof(Point)), destroy_dynarray);
    if (point_layer->positions == NULL) {
        RETURN_LT(lt, NULL);
    }

    point_layer->colors = PUSH_LT(lt, create_dynarray(sizeof(Color)), destroy_dynarray);
    if (point_layer->colors == NULL) {
        RETURN_LT(lt, NULL);
    }

    point_layer->ids = PUSH_LT(lt, create_dynarray(sizeof(char) * ID_MAX_SIZE), destroy_dynarray);
    if (point_layer->ids == NULL) {
        RETURN_LT(lt, NULL);
    }

    point_layer->edit_field = PUSH_LT(
        lt,
        create_edit_field(
            POINT_LAYER_ID_TEXT_SIZE,
            POINT_LAYER_ID_TEXT_COLOR),
        destroy_edit_field);
    if (point_layer->edit_field == NULL) {
        RETURN_LT(lt, NULL);
    }

    return point_layer;
}

PointLayer *create_point_layer_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);

    PointLayer *point_layer = create_point_layer();

    size_t count = 0;
    if (sscanf(
            line_stream_next(line_stream),
            "%zu",
            &count) == EOF) {
        log_fail("Could not read amount of points");
        RETURN_LT(point_layer->lt, NULL);
    }

    char color_name[7];
    char id[ID_MAX_SIZE];
    float x, y;
    for (size_t i = 0; i < count; ++i) {
        if (sscanf(
                line_stream_next(line_stream),
                "%"STRINGIFY(ID_MAX_SIZE)"s%f%f%6s",
                id, &x, &y, color_name) < 0) {
            log_fail("Could not read %dth goal\n", i);
            RETURN_LT(point_layer->lt, NULL);
        }
        const Color color = hexstr(color_name);
        const Point point = vec(x, y);

        dynarray_push(point_layer->colors, &color);
        dynarray_push(point_layer->positions, &point);
        dynarray_push(point_layer->ids, id);
    }

    point_layer->selected = -1;

    point_layer->color_picker = create_color_picker_from_rgba(COLOR_RED);

    return point_layer;
}

void destroy_point_layer(PointLayer *point_layer)
{
    trace_assert(point_layer);
    RETURN_LT0(point_layer->lt);
}

static inline
Triangle element_shape(Point position, float scale)
{
    return triangle_mat3x3_product(
        equilateral_triangle(),
        mat3x3_product(
            trans_mat_vec(position),
            scale_mat(scale)));
}

int point_layer_render(const PointLayer *point_layer,
                       Camera *camera,
                       int active)
{
    trace_assert(point_layer);
    trace_assert(camera);

    const int n = (int) dynarray_count(point_layer->positions);
    Point *positions = dynarray_data(point_layer->positions);
    Color *colors = dynarray_data(point_layer->colors);
    char *ids = dynarray_data(point_layer->ids);

    for (int i = 0; i < n; ++i) {
        const Color color = color_scale(
            point_layer->state == POINT_LAYER_RECOLOR && i == point_layer->selected
            ? point_layer->inter_color
            : colors[i],
            rgba(1.0f, 1.0f, 1.0f, active ? 1.0f : 0.5f));

        const Point position =
            point_layer->state == POINT_LAYER_MOVE && i == point_layer->selected
            ? point_layer->inter_position
            : positions[i];

        // Selection Layer
        if (active && i == point_layer->selected) {
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
                point_layer->edit_field,
                camera,
                positions[point_layer->selected]) < 0) {
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
                           Point position)
{
    trace_assert(point_layer);

    int n = (int) dynarray_count(point_layer->positions);
    Point *positions = dynarray_data(point_layer->positions);

    for (int i = 0; i < n; ++i) {
        if (vec_length(vec_sub(positions[i], position)) < POINT_LAYER_ELEMENT_RADIUS) {
            return i;
        }
    }

    return -1;
}

static
int point_layer_add_element(PointLayer *point_layer,
                            Point position,
                            Color color,
                            UndoHistory *undo_history)
{
    trace_assert(point_layer);
    trace_assert(undo_history);

    char id[ID_MAX_SIZE];
    for (size_t i = 0; i < ID_MAX_SIZE - 1; ++i) {
        id[i] = (char) ('a' + rand() % ('z' - 'a' + 1));
    }
    id[ID_MAX_SIZE - 1] = '\0';

    dynarray_push(point_layer->positions, &position);
    dynarray_push(point_layer->colors, &color);
    dynarray_push(point_layer->ids, id);

    UNDO_PUSH(point_layer, undo_history, UNDO_ADD);

    return 0;
}

static
void point_layer_delete_nth_element(PointLayer *point_layer,
                                    size_t i,
                                    UndoHistory *undo_history)
{
    trace_assert(point_layer);

    UNDO_PUSH(point_layer, undo_history, UNDO_DELETE);

    dynarray_delete_at(point_layer->positions, i);
    dynarray_delete_at(point_layer->colors, i);
    dynarray_delete_at(point_layer->ids, i);
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
        if (point_layer->selected >= 0) {
            point_layer->inter_color = color_picker_rgba(&point_layer->color_picker);
            point_layer->state = POINT_LAYER_RECOLOR;
        }
        return 0;
    }

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            const Point position = camera_map_screen(camera, event->button.x, event->button.y);

            point_layer->selected = point_layer_element_at(
                point_layer, position);

            if (point_layer->selected < 0) {
                point_layer_add_element(
                    point_layer,
                    position,
                    color_picker_rgba(&point_layer->color_picker),
                    undo_history);
            } else {
                Color *colors = dynarray_data(point_layer->colors);
                Point *positions = dynarray_data(point_layer->positions);

                point_layer->state = POINT_LAYER_MOVE;
                point_layer->color_picker =
                    create_color_picker_from_rgba(colors[point_layer->selected]);
                point_layer->inter_position = positions[point_layer->selected];
            }
        } break;
        }
    } break;

    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_DELETE: {
            if (0 <= point_layer->selected && point_layer->selected < (int) dynarray_count(point_layer->positions)) {
                point_layer_delete_nth_element(
                    point_layer,
                    (size_t)point_layer->selected,
                    undo_history);
                point_layer->selected = -1;
            }
        } break;

        case SDLK_F2: {
            if (point_layer->selected >= 0) {
                char *ids = dynarray_data(point_layer->ids);
                point_layer->state = POINT_LAYER_EDIT_ID;
                edit_field_replace(
                    point_layer->edit_field,
                    ids + ID_MAX_SIZE * point_layer->selected);
                SDL_StartTextInput();
            }
        } break;

        case SDLK_c: {
            if ((event->key.keysym.mod & KMOD_LCTRL) && point_layer->selected >= 0) {
                clipboard = 1;
                dynarray_copy_to(point_layer->colors, &clipboard_color, (size_t)point_layer->selected);
            }
        } break;

        case SDLK_v: {
            if ((event->key.keysym.mod & KMOD_LCTRL) && clipboard) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                Point position = camera_map_screen(camera, x, y);

                point_layer_add_element(
                    point_layer,
                    position,
                    clipboard_color,
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
            UNDO_PUSH(point_layer, undo_history, UNDO_UPDATE);

            char *id = dynarray_pointer_at(point_layer->ids, (size_t) point_layer->selected);
            const char *text = edit_field_as_text(point_layer->edit_field);
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

    return edit_field_event(point_layer->edit_field, event);
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
    trace_assert(point_layer->selected >= 0);

    switch (event->type) {
    case SDL_MOUSEBUTTONUP: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            point_layer->state = POINT_LAYER_IDLE;

            // TODO(#1014): just click (without moving) on the point creates an undo history entry
            UNDO_PUSH(point_layer, undo_history, UNDO_UPDATE);

            dynarray_replace_at(
                point_layer->positions,
                (size_t) point_layer->selected,
                &point_layer->inter_position);
        } break;
        }
    } break;

    case SDL_MOUSEMOTION: {
        point_layer->inter_position =
            camera_map_screen(camera, event->motion.x, event->motion.y);
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
    trace_assert(point_layer->selected >= 0);

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
            UNDO_PUSH(point_layer, undo_history, UNDO_UPDATE);

            dynarray_replace_at(
                point_layer->colors,
                (size_t) point_layer->selected,
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
    return dynarray_count(point_layer->positions);
}

const Point *point_layer_positions(const PointLayer *point_layer)
{
    trace_assert(point_layer);
    return dynarray_data(point_layer->positions);
}

const Color *point_layer_colors(const PointLayer *point_layer)
{
    trace_assert(point_layer);
    return dynarray_data(point_layer->colors);
}

const char *point_layer_ids(const PointLayer *point_layer)
{
    trace_assert(point_layer);
    return dynarray_data(point_layer->ids);
}

int point_layer_dump_stream(const PointLayer *point_layer,
                            FILE *filedump)
{
    trace_assert(point_layer);
    trace_assert(filedump);

    size_t n = dynarray_count(point_layer->ids);
    char *ids = dynarray_data(point_layer->ids);
    Point *positions = dynarray_data(point_layer->positions);
    Color *colors = dynarray_data(point_layer->colors);

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
