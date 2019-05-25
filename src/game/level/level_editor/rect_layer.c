#include "game/camera.h"
#include "system/lt.h"
#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "system/log.h"
#include "math/rect.h"
#include "color.h"
#include "rect_layer.h"
#include "dynarray.h"
#include "system/line_stream.h"
#include "proto_rect.h"
#include "color_picker.h"

struct RectLayer {
    Lt *lt;
    Dynarray *rects;
    Dynarray *colors;
    ProtoRect proto_rect;
    ColorPicker color_picker;
};

LayerPtr rect_layer_as_layer(RectLayer *rect_layer)
{
    LayerPtr layer = {
        .type = LAYER_RECT,
        .ptr = rect_layer
    };
    return layer;
}

RectLayer *create_rect_layer(void)
{
    Lt *lt = create_lt();

    RectLayer *layer = PUSH_LT(lt, nth_calloc(1, sizeof(RectLayer)), free);
    if (layer == NULL) {
        RETURN_LT(lt, NULL);
    }
    layer->lt = lt;

    layer->rects = PUSH_LT(
        lt,
        create_dynarray(sizeof(Rect)),
        destroy_dynarray);
    if (layer->rects == NULL) {
        RETURN_LT(lt, NULL);
    }

    layer->colors = PUSH_LT(
        lt,
        create_dynarray(sizeof(Color)),
        destroy_dynarray);
    if (layer->colors == NULL) {
        RETURN_LT(lt, NULL);
    }

    layer->color_picker.color = rgba(1.0f, 0.0f, 0.0f, 1.0f);
    layer->proto_rect.color_current = &layer->color_picker.color;
    layer->proto_rect.layer_current = layer;

    return layer;
}

RectLayer *create_rect_layer_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);

    RectLayer *layer = create_rect_layer();
    if (layer == NULL) {
        return NULL;
    }

    const char *line = line_stream_next(line_stream);
    if (line == NULL) {
        RETURN_LT(layer->lt, NULL);
    }

    size_t count = 0;
    if (sscanf(line, "%lu", &count) < 0) {
        RETURN_LT(layer->lt, NULL);
    }

    for (size_t i = 0; i < count; ++i) {
        line = line_stream_next(line_stream);
        if (line == NULL) {
            RETURN_LT(layer->lt, NULL);
        }

        char hex[7];
        Rect rect;

        if (sscanf(line, "%f%f%f%f%6s\n",
                   &rect.x, &rect.y,
                   &rect.w, &rect.h,
                   hex) < 0) {
            RETURN_LT(layer->lt, NULL);
        }

        if (rect_layer_add_rect(layer, rect, hexstr(hex)) < 0) {
            RETURN_LT(layer->lt, NULL);
        }
    }

    return layer;
}

void destroy_rect_layer(RectLayer *layer)
{
    trace_assert(layer);
    RETURN_LT0(layer->lt);
}

int rect_layer_render(const RectLayer *layer, Camera *camera, float fa)
{
    trace_assert(layer);
    trace_assert(camera);

    const size_t n = dynarray_count(layer->rects);
    Rect *rects = dynarray_data(layer->rects);
    Color *colors = dynarray_data(layer->colors);

    for (size_t i = 0; i < n; ++i) {
        if (camera_fill_rect(
                camera,
                rects[i],
                color_scale(
                    colors[i],
                    rgba(1.0f, 1.0f, 1.0f, fa))) < 0) {
            return -1;
        }
    }

    if (proto_rect_render(&layer->proto_rect, camera) < 0) {
        return -1;
    }

    if (color_picker_render(&layer->color_picker, camera) < 0) {
        return -1;
    }

    return 0;
}

int rect_layer_event(RectLayer *layer, const SDL_Event *event, const Camera *camera)
{
    trace_assert(layer);
    trace_assert(event);

    switch(event->type) {
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP: {
        bool selected = false;
        if (color_picker_mouse_button(
                &layer->color_picker,
                &event->button,
                &selected) < 0) {
            return -1;
        }

        if (!selected && proto_rect_mouse_button(
                &layer->proto_rect,
                &event->button,
                camera) < 0) {
            return -1;
        }

    } break;

    case SDL_MOUSEMOTION: {
        if (proto_rect_mouse_motion(&layer->proto_rect, &event->motion, camera) < 0) {
            return -1;
        }
    } break;
    }

    return 0;
}

int rect_layer_add_rect(RectLayer *layer, Rect rect, Color color)
{
    trace_assert(layer);

    if (dynarray_push(layer->rects, &rect) < 0) {
        return -1;
    }

    if (dynarray_push(layer->colors, &color) < 0) {
        return -1;
    }

    return 0;
}

int rect_layer_delete_rect_at(RectLayer *layer, Vec position)
{
    trace_assert(layer);

    const size_t n = dynarray_count(layer->rects);
    Rect *rects = dynarray_data(layer->rects);

    for (size_t i = 0; i < n; ++i) {
        if (rect_contains_point(rects[i], position)) {
            dynarray_delete_at(layer->rects, i);
            dynarray_delete_at(layer->colors, i);
            return 0;
        }
    }

    return 0;
}

size_t rect_layer_count(const RectLayer *layer)
{
    return dynarray_count(layer->rects);
}

const Rect *rect_layer_rects(const RectLayer *layer)
{
    return dynarray_data(layer->rects);
}

const Color *rect_layer_colors(const RectLayer *layer)
{
    return dynarray_data(layer->colors);
}
