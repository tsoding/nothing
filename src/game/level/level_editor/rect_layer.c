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
#include "color_picker.h"
#include "system/str.h"

#define RECT_LAYER_ID_MAX_SIZE 36
#define RECT_LAYER_SELECTION_THICCNESS 5.0f
#define PROTO_AREA_THRESHOLD 10.0

// TODO(#942): RectLayer does not allow to move rectangle arround
typedef enum {
    RECT_LAYER_IDLE = 0,
    RECT_LAYER_PROTO,
    RECT_LAYER_RESIZE
} RectLayerState;

/* TODO(#886): RectLayer does not allow to modify ids of Rects */
struct RectLayer {
    Lt *lt;
    RectLayerState state;
    Dynarray *ids;
    Dynarray *rects;
    Dynarray *colors;
    ColorPicker color_picker;
    Vec proto_begin;
    Vec proto_end;
    int selection;
};

static int rect_layer_rect_at(RectLayer *layer, Vec position)
{
    trace_assert(layer);

    const size_t n = dynarray_count(layer->rects);
    Rect *rects = dynarray_data(layer->rects);

    for (size_t i = 0; i < n; ++i) {
        if (rect_contains_point(rects[i], position)) {
            return (int) i;
        }
    }

    return -1;
}

static int rect_layer_delete_rect_at(RectLayer *layer, size_t i)
{
    trace_assert(layer);

    dynarray_delete_at(layer->rects, i);
    dynarray_delete_at(layer->colors, i);
    dynarray_delete_at(layer->ids, i);

    return 0;
}

static Rect rect_layer_resize_anchor(const RectLayer *layer, size_t i)
{
    Rect *rects = dynarray_data(layer->rects);
    return rect(rects[i].x + rects[i].w,
                rects[i].y + rects[i].h,
                RECT_LAYER_SELECTION_THICCNESS * 2.0f,
                RECT_LAYER_SELECTION_THICCNESS * 2.0f);
}

static int rect_layer_add_rect(RectLayer *layer, Rect rect, Color color)
{
    trace_assert(layer);

    if (dynarray_push(layer->rects, &rect) < 0) {
        return -1;
    }

    if (dynarray_push(layer->colors, &color) < 0) {
        return -1;
    }

    char id[RECT_LAYER_ID_MAX_SIZE];
    for (size_t i = 0; i < RECT_LAYER_ID_MAX_SIZE - 1; ++i) {
        id[i] = (char) ('a' + rand() % ('z' - 'a' + 1));
    }
    id[RECT_LAYER_ID_MAX_SIZE - 1] = '\0';

    if (dynarray_push(layer->ids, id)) {
        return -1;
    }

    return 0;
}


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

    layer->ids = PUSH_LT(
        lt,
        create_dynarray(sizeof(char) * RECT_LAYER_ID_MAX_SIZE),
        destroy_dynarray);
    if (layer->ids == NULL) {
        RETURN_LT(lt, NULL);
    }

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

    layer->color_picker = create_color_picker_from_rgba(rgba(1.0f, 0.0f, 0.0f, 1.0f));
    layer->selection = -1;

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
    if (sscanf(line, "%zu", &count) < 0) {
        RETURN_LT(layer->lt, NULL);
    }

    for (size_t i = 0; i < count; ++i) {
        line = line_stream_next(line_stream);
        if (line == NULL) {
            RETURN_LT(layer->lt, NULL);
        }

        char hex[7];
        Rect rect;
        char id[RECT_LAYER_ID_MAX_SIZE];

        if (sscanf(line,
                   "%"STRINGIFY(RECT_LAYER_ID_MAX_SIZE)"s%f%f%f%f%6s\n",
                   id,
                   &rect.x, &rect.y,
                   &rect.w, &rect.h,
                   hex) < 0) {
            RETURN_LT(layer->lt, NULL);
        }

        Color color = hexstr(hex);

        dynarray_push(layer->rects, &rect);
        dynarray_push(layer->ids, id);
        dynarray_push(layer->colors, &color);
    }

    return layer;
}

void destroy_rect_layer(RectLayer *layer)
{
    trace_assert(layer);
    RETURN_LT0(layer->lt);
}

int rect_layer_render(const RectLayer *layer, Camera *camera, int active)
{
    trace_assert(layer);
    trace_assert(camera);

    const size_t n = dynarray_count(layer->rects);
    Rect *rects = dynarray_data(layer->rects);
    Color *colors = dynarray_data(layer->colors);

    for (size_t i = 0; i < n; ++i) {
        if (layer->selection == (int) i) {
            if (active) {
                const Color color = color_invert(colors[i]);

                if (camera_fill_rect(
                        camera,
                        // TODO: thiccness of RectLayer selection should be probably based on zoom
                        rect_scale(rects[i], RECT_LAYER_SELECTION_THICCNESS),
                        color) < 0) {
                    return -1;
                }

                if (camera_fill_rect(
                        camera,
                        rect_layer_resize_anchor(layer, i),
                        color) < 0) {
                    return -1;
                }
            }
        }

        if (camera_fill_rect(
                camera,
                rects[i],
                color_scale(
                    colors[i],
                    rgba(1.0f, 1.0f, 1.0f, active ? 1.0f : 0.5f))) < 0) {
            return -1;
        }

    }

    const Color color = color_picker_rgba(&layer->color_picker);
    if (layer->state == RECT_LAYER_PROTO) {
        if (camera_fill_rect(camera, rect_from_points(layer->proto_begin, layer->proto_end), color) < 0) {
            return -1;
        }
    }

    if (active && color_picker_render(&layer->color_picker, camera) < 0) {
        return -1;
    }

    return 0;
}

int rect_layer_event(RectLayer *layer, const SDL_Event *event, const Camera *camera)
{
    trace_assert(layer);
    trace_assert(event);

    int selected = 0;
    if (color_picker_event(&layer->color_picker, event, &selected) < 0) {
        return -1;
    }

    if (selected) {
        return 0;
    }

    const Color color = color_picker_rgba(&layer->color_picker);
    if (layer->state == RECT_LAYER_PROTO) {
        switch (event->type) {
        case SDL_MOUSEBUTTONUP: {
            switch (event->button.button) {
            case SDL_BUTTON_LEFT: {
                const Rect real_rect =
                    rect_from_points(
                        layer->proto_begin,
                        layer->proto_end);
                const float area = real_rect.w * real_rect.h;

                if (area >= PROTO_AREA_THRESHOLD) {
                    rect_layer_add_rect(layer, real_rect, color);
                } else {
                    log_info("The area is too small %f. Such small box won't be created.\n", area);
                }
                layer->state = RECT_LAYER_IDLE;
            } break;
            }
        } break;

        case SDL_MOUSEMOTION: {
            layer->proto_end = camera_map_screen(
                camera,
                event->motion.x,
                event->motion.y);
        } break;
        }
    } else if (layer->state == RECT_LAYER_RESIZE) {
        switch (event->type) {
        case SDL_MOUSEMOTION: {
            Rect *rects = dynarray_data(layer->rects);
            trace_assert(layer->selection >= 0);
            rects[layer->selection] = rect_from_points(
                vec(rects[layer->selection].x, rects[layer->selection].y),
                vec_sum(
                    camera_map_screen(
                        camera,
                        event->button.x,
                        event->button.y),
                    vec(RECT_LAYER_SELECTION_THICCNESS * -0.5f,
                        RECT_LAYER_SELECTION_THICCNESS * -0.5f)));
        } break;

        case SDL_MOUSEBUTTONUP: {
            layer->state = RECT_LAYER_IDLE;
        } break;
        }
    } else {
        switch (event->type) {
        case SDL_MOUSEBUTTONDOWN: {
            switch (event->button.button) {
            case SDL_BUTTON_LEFT: {
                Point position = camera_map_screen(
                    camera,
                    event->button.x,
                    event->button.y);

                if (layer->selection >= 0 &&
                    rect_contains_point(
                        rect_layer_resize_anchor(
                            layer,
                            (size_t)layer->selection),
                        position)) {
                    layer->state = RECT_LAYER_RESIZE;
                } else {
                    layer->selection = rect_layer_rect_at(layer, position);

                    if (layer->selection < 0) {
                        layer->state = RECT_LAYER_PROTO;
                        layer->proto_begin = position;
                        layer->proto_end = position;
                    }
                }
            } break;
            }
        } break;

        case SDL_KEYDOWN: {
            switch (event->key.keysym.sym) {
            case SDLK_DELETE: {
                if (layer->selection >= 0) {
                    rect_layer_delete_rect_at(layer, (size_t) layer->selection);
                    layer->selection = -1;
                }
            } break;
            }
        } break;
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

const char *rect_layer_ids(const RectLayer *layer)
{
    return dynarray_data(layer->ids);
}

int rect_layer_dump_stream(const RectLayer *layer, FILE *filedump)
{
    trace_assert(layer);
    trace_assert(filedump);

    size_t n = dynarray_count(layer->ids);
    char *ids = dynarray_data(layer->ids);
    Rect *rects = dynarray_data(layer->rects);
    Color *colors = dynarray_data(layer->colors);

    fprintf(filedump, "%zd\n", n);
    for (size_t i = 0; i < n; ++i) {
        fprintf(filedump, "%s %f %f %f %f ",
                ids + RECT_LAYER_ID_MAX_SIZE * i,
                rects[i].x, rects[i].y, rects[i].w, rects[i].h);
        color_hex_to_stream(colors[i], filedump);
        fprintf(filedump, "\n");
    }

    return 0;
}
