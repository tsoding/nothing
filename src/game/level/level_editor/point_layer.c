#include <SDL2/SDL.h>

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

#define POINT_LAYER_ELEMENT_RADIUS 10.0f

typedef enum {
    POINT_LAYER_NORMAL_STATE = 0,
    POINT_LAYER_ID_EDITING_STATE
} PointLayerState;

struct PointLayer
{
    Lt *lt;
    PointLayerState state;
    Dynarray/*<Point>*/ *points;
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

PointLayer *create_point_layer_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);

    Lt *lt = create_lt();

    PointLayer *point_layer = PUSH_LT(lt, nth_calloc(1, sizeof(PointLayer)), free);
    if (point_layer == NULL) {
        RETURN_LT(lt, NULL);
    }
    point_layer->lt = lt;

    point_layer->state = POINT_LAYER_NORMAL_STATE;

    point_layer->points = PUSH_LT(lt, create_dynarray(sizeof(Point)), destroy_dynarray);
    if (point_layer->points == NULL) {
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

    size_t count = 0;
    if (sscanf(
            line_stream_next(line_stream),
            "%lu",
            &count) == EOF) {
        log_fail("Could not read amount of points");
        RETURN_LT(lt, NULL);
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
            RETURN_LT(lt, NULL);
        }
        const Color color = hexstr(color_name);
        const Point point = vec(x, y);

        dynarray_push(point_layer->colors, &color);
        dynarray_push(point_layer->points, &point);
        dynarray_push(point_layer->ids, id);
    }

    point_layer->edit_field = PUSH_LT(
        point_layer->lt,
        create_edit_field(
            vec(5.0f, 5.0f),
            rgba(0.0f, 0.0f, 0.0f, 1.0f)),
        destroy_edit_field);
    if (point_layer->edit_field == NULL) {
        RETURN_LT(point_layer->lt, NULL);
    }

    point_layer->selected = -1;

    point_layer->color_picker.color = rgba(1.0f, 0.0f, 0.0f, 1.0f);

    return point_layer;
}

void destroy_point_layer(PointLayer *point_layer)
{
    trace_assert(point_layer);
    RETURN_LT0(point_layer->lt);
}

int point_layer_render(const PointLayer *point_layer,
                       Camera *camera)
{
    trace_assert(point_layer);
    trace_assert(camera);

    const int n = (int) dynarray_count(point_layer->points);
    Point *points = dynarray_data(point_layer->points);
    Color *colors = dynarray_data(point_layer->colors);

    for (int i = 0; i < n; ++i) {
        const Triangle t = triangle_mat3x3_product(
            equilateral_triangle(),
            mat3x3_product(
                trans_mat(points[i].x, points[i].y),
                scale_mat(POINT_LAYER_ELEMENT_RADIUS)));

        if (i == point_layer->selected) {
            const Triangle t0 = triangle_mat3x3_product(
                equilateral_triangle(),
                mat3x3_product(
                    trans_mat(points[i].x, points[i].y),
                    scale_mat(15.0f)));

            if (camera_fill_triangle(camera, t0, color_invert(colors[i])) < 0) {
                return -1;
            }
        }

        if (camera_fill_triangle(camera, t, colors[i]) < 0) {
            return -1;
        }

        /* TODO(#854): The ids of PointLayer are not displayed constantly */
    }

    if (point_layer->state == POINT_LAYER_ID_EDITING_STATE) {
        /* TODO(#855): PointLayer edit field is not scaled on zoom */
        if (edit_field_render(
                point_layer->edit_field,
                camera,
                camera_point(camera, points[point_layer->selected])) < 0) {
            return -1;
        }
    }

    if (color_picker_render(&point_layer->color_picker, camera) < 0) {
        return -1;
    }


    return 0;
}


static int point_layer_mouse_button(PointLayer *point_layer,
                                    const SDL_MouseButtonEvent *event,
                                    const Camera *camera)
{
    trace_assert(point_layer);
    trace_assert(event);

    bool selected = false;
    if (color_picker_mouse_button(
            &point_layer->color_picker,
            event,
            &selected) < 0) {
        return -1;
    }

    if (!selected &&
        point_layer->state == POINT_LAYER_NORMAL_STATE &&
        event->type == SDL_MOUSEBUTTONDOWN &&
        event->button == SDL_BUTTON_LEFT) {
        const int n = (int) dynarray_count(point_layer->points);
        const Point *points = dynarray_data(point_layer->points);
        const Point point = camera_map_screen(camera, event->x, event->y);

        for (int i = 0; i < n; ++i) {
            if (vec_length(vec_sub(points[i], point)) < POINT_LAYER_ELEMENT_RADIUS) {
                point_layer->selected = i;
                return 0;
            }
        }

        char id[ID_MAX_SIZE];

        for (size_t i = 0; i < ID_MAX_SIZE - 1; ++i) {
            id[i] = (char) ('a' + rand() % ('z' - 'a' + 1));
        }
        id[ID_MAX_SIZE - 1] = '\0';

        dynarray_push(point_layer->points, &point);
        dynarray_push(point_layer->colors, &point_layer->color_picker.color);
        dynarray_push(point_layer->ids, id);
    }

    return 0;
}

static
int point_layer_keyboard(PointLayer *point_layer,
                         const SDL_KeyboardEvent *key)
{
    trace_assert(point_layer);
    trace_assert(key);

    switch(point_layer->state) {
    case POINT_LAYER_NORMAL_STATE: {
        if (key->type == SDL_KEYDOWN) {
            switch (key->keysym.sym) {
            case SDLK_DELETE: {
                if (0 <= point_layer->selected && point_layer->selected < (int) dynarray_count(point_layer->points)) {
                    dynarray_delete_at(point_layer->points, (size_t) point_layer->selected);
                    dynarray_delete_at(point_layer->colors, (size_t) point_layer->selected);
                    dynarray_delete_at(point_layer->ids, (size_t) point_layer->selected);
                }
                point_layer->selected = -1;
            } break;

            case SDLK_F2: {
                if (point_layer->selected >= 0) {
                    char *ids = dynarray_data(point_layer->ids);
                    point_layer->state = POINT_LAYER_ID_EDITING_STATE;
                    edit_field_replace(
                        point_layer->edit_field,
                        ids + ID_MAX_SIZE * point_layer->selected);
                    SDL_StartTextInput();
                }
            } break;

            default: {}
            }
        }
    } break;

    case POINT_LAYER_ID_EDITING_STATE: {
        if (edit_field_keyboard(point_layer->edit_field, key) < 0) {
            return -1;
        }

        if (key->type == SDL_KEYDOWN) {
            switch(key->keysym.sym) {
            case SDLK_RETURN: {
                char *ids = dynarray_data(point_layer->ids);
                const char *text = edit_field_as_text(point_layer->edit_field);
                size_t n = max_size_t(strlen(text), ID_MAX_SIZE - 1);
                memcpy(ids + point_layer->selected * ID_MAX_SIZE, text, n);
                *(ids + point_layer->selected * ID_MAX_SIZE + n) = '\0';
                point_layer->state = POINT_LAYER_NORMAL_STATE;
                SDL_StopTextInput();
            } break;

            case SDLK_ESCAPE: {
                point_layer->state = POINT_LAYER_NORMAL_STATE;
                SDL_StopTextInput();
            } break;
            }
        }
    } break;
    }


    return 0;
}

static
int point_layer_text_input(PointLayer *point_layer,
                           const SDL_TextInputEvent *text_input)
{
    trace_assert(point_layer);
    trace_assert(text_input);

    if (point_layer->state == POINT_LAYER_ID_EDITING_STATE) {
        /* TODO(#856): Special development keybindings interfere with id editing field */
        return edit_field_text_input(point_layer->edit_field, text_input);
    }

    return 0;
}

int point_layer_event(PointLayer *point_layer,
                      const SDL_Event *event,
                      const Camera *camera)
{
    trace_assert(point_layer);
    trace_assert(event);
    trace_assert(camera);

    switch(event->type) {
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        return point_layer_mouse_button(
            point_layer,
            &event->button,
            camera);

    case SDL_KEYDOWN:
    case SDL_KEYUP:
        return point_layer_keyboard(
            point_layer,
            &event->key);

    case SDL_TEXTINPUT:
        return point_layer_text_input(
            point_layer,
            &event->text);
    }

    return 0;
}

size_t point_layer_count(const PointLayer *point_layer)
{
    trace_assert(point_layer);
    return dynarray_count(point_layer->points);
}

const Point *point_layer_points(const PointLayer *point_layer)
{
    trace_assert(point_layer);
    return dynarray_data(point_layer->points);
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
