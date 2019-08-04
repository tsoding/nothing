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
#include "./color_picker.h"
#include "undo_history.h"

#define POINT_LAYER_ELEMENT_RADIUS 10.0f
#define POINT_LAYER_ID_TEXT_SIZE vec(2.0f, 2.0f)
#define POINT_LAYER_ID_TEXT_COLOR COLOR_BLACK

// TODO(#1002): PointLayer does not fully support UndoHistory

typedef enum {
    POINT_LAYER_IDLE = 0,
    POINT_LAYER_EDIT_ID,
    POINT_LAYER_MOVE
} PointLayerState;

struct PointLayer
{
    Lt *lt;
    PointLayerState state;
    Dynarray/*<Point>*/ *positions;
    Dynarray/*<Color>*/ *colors;
    Dynarray/*<char[ID_MAX_SIZE]>*/ *ids;
    Edit_field *edit_field;
    int selected;
    ColorPicker color_picker;
};

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

    point_layer->color_picker = create_color_picker_from_rgba(rgba(1.0f, 0.0f, 0.0f, 1.0f));

    return point_layer;
}

void destroy_point_layer(PointLayer *point_layer)
{
    trace_assert(point_layer);
    RETURN_LT0(point_layer->lt);
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
        const Triangle t = triangle_mat3x3_product(
            equilateral_triangle(),
            mat3x3_product(
                trans_mat(positions[i].x, positions[i].y),
                scale_mat(POINT_LAYER_ELEMENT_RADIUS)));

        const Color color = color_scale(
            colors[i],
            rgba(1.0f, 1.0f, 1.0f, active ? 1.0f : 0.5f));

        if (i == point_layer->selected) {
            const Triangle t0 = triangle_mat3x3_product(
                equilateral_triangle(),
                mat3x3_product(
                    trans_mat(positions[i].x, positions[i].y),
                    scale_mat(15.0f)));

            if (camera_fill_triangle(camera, t0, color_invert(color)) < 0) {
                return -1;
            }

            if (point_layer->state != POINT_LAYER_EDIT_ID &&
                camera_render_text(
                    camera,
                    ids + ID_MAX_SIZE * i,
                    POINT_LAYER_ID_TEXT_SIZE,
                    POINT_LAYER_ID_TEXT_COLOR,
                    positions[i]) < 0) {
                return -1;
            }
        }

        if (camera_fill_triangle(camera, t, color) < 0) {
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
void point_layer_pop_element(void *layer, Context context)
{
    trace_assert(layer);
    (void) context;

    PointLayer *point_layer = layer;

    dynarray_pop(point_layer->positions, NULL);
    dynarray_pop(point_layer->colors, NULL);
    dynarray_pop(point_layer->ids, NULL);
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

    Action action =  {
        .revert = point_layer_pop_element,
        .layer = point_layer
    };
    undo_history_push(undo_history, action);

    return 0;
}

static
void point_layer_delete_nth_element(PointLayer *point_layer,
                                    size_t i)
{
    trace_assert(point_layer);
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
            &selected,
            undo_history) < 0) {
        return -1;
    }

    if (selected) {
        if (point_layer->selected >= 0) {
            Color *colors = dynarray_data(point_layer->colors);
            colors[point_layer->selected] =
                color_picker_rgba(&point_layer->color_picker);
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
                point_layer->state = POINT_LAYER_MOVE;
                point_layer->color_picker =
                    create_color_picker_from_rgba(colors[point_layer->selected]);
            }
        } break;
        }
    } break;

    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_DELETE: {
            if (0 <= point_layer->selected && point_layer->selected < (int) dynarray_count(point_layer->positions)) {
                point_layer_delete_nth_element(point_layer, (size_t)point_layer->selected);
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
        }
    } break;
    }

    return 0;
}

static
int point_layer_edit_id_event(PointLayer *point_layer,
                              const SDL_Event *event,
                              const Camera *camera)
{
    trace_assert(point_layer);
    trace_assert(event);
    trace_assert(camera);

    switch (event->type) {
    case SDL_KEYDOWN: {
        switch(event->key.keysym.sym) {
        case SDLK_RETURN: {
            char *ids = dynarray_data(point_layer->ids);
            const char *text = edit_field_as_text(point_layer->edit_field);
            size_t n = max_size_t(strlen(text), ID_MAX_SIZE - 1);
            memcpy(ids + point_layer->selected * ID_MAX_SIZE, text, n);
            *(ids + point_layer->selected * ID_MAX_SIZE + n) = '\0';
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
                           const Camera *camera)
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
        } break;
        }
    } break;

    case SDL_MOUSEMOTION: {
        Point *positions = dynarray_data(point_layer->positions);
        positions[point_layer->selected] =
            camera_map_screen(camera, event->motion.x, event->motion.y);
    } break;
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
        return point_layer_edit_id_event(point_layer, event, camera);

    case POINT_LAYER_MOVE:
        return point_layer_move_event(point_layer, event, camera);
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
