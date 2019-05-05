#include <SDL2/SDL.h>

#include "system/stacktrace.h"
#include "system/line_stream.h"
#include "system/log.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/str.h"
#include "dynarray.h"
#include "game/camera.h"
#include "./point_layer.h"

struct PointLayer
{
    Lt *lt;
    Dynarray *points;
    Dynarray *colors;
    Dynarray *ids;
};

// TODO(#837): PointLayer does not allow to edit itself

PointLayer *create_point_layer_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);

    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    PointLayer *point_layer = PUSH_LT(lt, nth_calloc(1, sizeof(PointLayer)), free);
    if (point_layer == NULL) {
        RETURN_LT(lt, NULL);
    }
    point_layer->lt = lt;

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

    const size_t n = dynarray_count(point_layer->points);
    Point *points = dynarray_data(point_layer->points);
    Color *colors = dynarray_data(point_layer->colors);

    for (size_t i = 0; i < n; ++i) {
        const Triangle t = triangle_mat3x3_product(
            equilateral_triangle(),
            mat3x3_product(
                trans_mat(points[i].x, points[i].y),
                scale_mat(10.0f)));

        if (camera_fill_triangle(camera, t, colors[i]) < 0) {
            return -1;
        }
    }

    return 0;
}

int point_layer_mouse_button(PointLayer *point_layer,
                             const SDL_MouseButtonEvent *event)
{
    trace_assert(point_layer);
    trace_assert(event);
    log_info("Clicked %d %d\n", event->x, event->y);
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
