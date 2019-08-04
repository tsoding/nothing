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
#include "ui/edit_field.h"

#define RECT_LAYER_ID_MAX_SIZE 36
#define RECT_LAYER_SELECTION_THICCNESS 10.0f
#define CREATE_AREA_THRESHOLD 10.0

// TODO(#1003): RectLayer does not support UndoHistory

typedef enum {
    RECT_LAYER_IDLE = 0,
    RECT_LAYER_CREATE,
    // TODO(#955): Rectangles in Level Editor have only one resize anchor to work with
    RECT_LAYER_RESIZE,
    RECT_LAYER_MOVE,
    RECT_LAYER_ID_RENAME,
} RectLayerState;

struct RectLayer {
    Lt *lt;
    RectLayerState state;
    Dynarray *ids;
    Dynarray *rects;
    Dynarray *colors;
    ColorPicker color_picker;
    Vec create_begin;
    Vec create_end;
    int selection;
    Vec move_anchor;
    Edit_field *id_edit_field;
};

typedef int (*EventHandler)(RectLayer *layer, const SDL_Event *event, const Camera *camera);

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

// TODO(#956): rect_layer_rect_at doesn't return rectangles according to some z-order
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

static Rect rect_layer_resize_anchor(const RectLayer *layer, const Camera *camera, size_t i)
{
    Rect *rects = dynarray_data(layer->rects);
    const Rect overlay_rect =
        rect_scale(
            camera_rect(camera, rects[i]),
            RECT_LAYER_SELECTION_THICCNESS * 0.5f);

    return rect(
        overlay_rect.x + overlay_rect.w,
        overlay_rect.y + overlay_rect.h,
        RECT_LAYER_SELECTION_THICCNESS * 2.0f,
        RECT_LAYER_SELECTION_THICCNESS * 2.0f);
}

static int rect_layer_delete_rect_at(RectLayer *layer, size_t i)
{
    trace_assert(layer);

    dynarray_delete_at(layer->rects, i);
    dynarray_delete_at(layer->colors, i);
    dynarray_delete_at(layer->ids, i);

    return 0;
}

static int rect_layer_event_idle(RectLayer *layer, const SDL_Event *event, const Camera *camera)
{
    trace_assert(layer);
    trace_assert(event);
    trace_assert(camera);

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            Point position = camera_map_screen(
                camera,
                event->button.x,
                event->button.y);
            int rect_at_position =
                rect_layer_rect_at(layer, position);

            if (rect_at_position >= 0) {
                Rect *rects = dynarray_data(layer->rects);
                Color *colors = dynarray_data(layer->colors);
                layer->selection = rect_at_position;
                layer->state = RECT_LAYER_MOVE;
                layer->move_anchor =
                    vec_sub(
                        position,
                        vec(
                            rects[layer->selection].x,
                            rects[layer->selection].y));
                layer->color_picker =
                    create_color_picker_from_rgba(colors[rect_at_position]);
            } else if (layer->selection >= 0 && rect_contains_point(
                           rect_layer_resize_anchor(
                               layer,
                               camera,
                               (size_t)layer->selection),
                           vec(
                               (float) event->button.x,
                               (float) event->button.y))) {
                layer->state = RECT_LAYER_RESIZE;
            } else {
                layer->selection = rect_at_position;

                if (layer->selection < 0) {
                    layer->state = RECT_LAYER_CREATE;
                    layer->create_begin = position;
                    layer->create_end = position;
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

        case SDLK_F2: {
            if (layer->selection >= 0) {
                const char *ids = dynarray_data(layer->ids);
                layer->state = RECT_LAYER_ID_RENAME;
                edit_field_replace(
                    layer->id_edit_field,
                    ids + layer->selection * RECT_LAYER_ID_MAX_SIZE);
                SDL_StartTextInput();
            }
        } break;
        }
    } break;
    }

    return 0;
}

static int rect_layer_event_create(RectLayer *layer, const SDL_Event *event, const Camera *camera)
{
    trace_assert(layer);
    trace_assert(event);
    trace_assert(camera);

    switch (event->type) {
    case SDL_MOUSEBUTTONUP: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            const Rect real_rect =
                rect_from_points(
                    layer->create_begin,
                    layer->create_end);
            const float area = real_rect.w * real_rect.h;

            if (area >= CREATE_AREA_THRESHOLD) {
                rect_layer_add_rect(
                    layer,
                    real_rect,
                    color_picker_rgba(&layer->color_picker));
            } else {
                log_info("The area is too small %f. Such small box won't be created.\n", area);
            }
            layer->state = RECT_LAYER_IDLE;
        } break;
        }
    } break;

    case SDL_MOUSEMOTION: {
        layer->create_end = camera_map_screen(
            camera,
            event->motion.x,
            event->motion.y);
    } break;
    }
    return 0;
}

static int rect_layer_event_resize(RectLayer *layer, const SDL_Event *event, const Camera *camera)
{
    trace_assert(layer);
    trace_assert(event);
    trace_assert(camera);

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

    return 0;
}

static int rect_layer_event_move(RectLayer *layer, const SDL_Event *event, const Camera *camera)
{
    trace_assert(layer);
    trace_assert(event);
    trace_assert(camera);

    switch (event->type) {
    case SDL_MOUSEMOTION: {
        Point position = vec_sub(
            camera_map_screen(
                camera,
                event->button.x,
                event->button.y),
            layer->move_anchor);

        Rect *rects = dynarray_data(layer->rects);

        trace_assert(layer->selection >= 0);

        rects[layer->selection].x = position.x;
        rects[layer->selection].y = position.y;
    } break;

    case SDL_MOUSEBUTTONUP: {
        layer->state = RECT_LAYER_IDLE;
    } break;
    }
    return 0;
}

static int rect_layer_event_id_rename(RectLayer *layer, const SDL_Event *event, const Camera *camera)
{
    trace_assert(layer);
    trace_assert(event);
    trace_assert(camera);

    switch (event->type) {
    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_RETURN: {
            char *id =
                (char *)dynarray_data(layer->ids) + layer->selection * RECT_LAYER_ID_MAX_SIZE;
            memset(id, 0, RECT_LAYER_ID_MAX_SIZE);
            memcpy(id, edit_field_as_text(layer->id_edit_field), RECT_LAYER_ID_MAX_SIZE - 1);
            layer->state = RECT_LAYER_IDLE;
            SDL_StopTextInput();
        } break;

        case SDLK_ESCAPE: {
            layer->state = RECT_LAYER_IDLE;
            SDL_StopTextInput();
        } break;
        }
    } break;
    }

    return edit_field_event(layer->id_edit_field, event);
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

    layer->id_edit_field = PUSH_LT(
        lt,
        create_edit_field(
            vec(3.0f, 3.0f),
            COLOR_BLACK),
        destroy_edit_field);
    if (layer->id_edit_field == NULL) {
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
    const char *ids = dynarray_data(layer->ids);

    // The Rectangles
    for (size_t i = 0; i < n; ++i) {
        if (camera_fill_rect(
                camera,
                rects[i],
                color_scale(
                    colors[i],
                    rgba(1.0f, 1.0f, 1.0f, active ? 1.0f : 0.5f))) < 0) {
            return -1;
        }
    }

    // Proto Rectangle
    const Color color = color_picker_rgba(&layer->color_picker);
    if (layer->state == RECT_LAYER_CREATE) {
        if (camera_fill_rect(camera, rect_from_points(layer->create_begin, layer->create_end), color) < 0) {
            return -1;
        }
    }

    // ID renaming Edit Field
    if (layer->state == RECT_LAYER_ID_RENAME) {
        if (edit_field_render_screen(layer->id_edit_field, camera, vec(400.0f, 400.0f)) < 0) {
            return -1;
        }
    }

    // Selection Overlay
    if (active && layer->selection >= 0) {
        const Rect overlay_rect =
            rect_scale(
                camera_rect(camera, rects[layer->selection]),
                RECT_LAYER_SELECTION_THICCNESS * 0.5f);
        const Color overlay_color = color_invert(colors[layer->selection]);

        // Main Rectangle
        if (camera_fill_rect(
                camera,
                rects[layer->selection],
                color_scale(
                    colors[layer->selection],
                    rgba(1.0f, 1.0f, 1.0f, active ? 1.0f : 0.5f))) < 0) {
            return -1;
        }

        if (camera_draw_thicc_rect_screen(
                camera,
                overlay_rect,
                overlay_color,
                RECT_LAYER_SELECTION_THICCNESS) < 0) {
            return -1;
        }

        // Rectangle Id
        if (camera_render_text(
                camera,
                ids + layer->selection * RECT_LAYER_ID_MAX_SIZE,
                vec(3.0f, 3.0f),
                color_invert(colors[layer->selection]),
                rect_position(rects[layer->selection])) < 0) {
            return -1;
        }

        // Resize Anchor
        if (camera_fill_rect_screen(
                camera,
                rect_layer_resize_anchor(layer, camera, (size_t) layer->selection),
                overlay_color) < 0) {
            return -1;
        }
    }

    if (active && color_picker_render(&layer->color_picker, camera) < 0) {
        return -1;
    }

    return 0;
}

int rect_layer_event(RectLayer *layer,
                     const SDL_Event *event,
                     const Camera *camera,
                     UndoHistory *undo_history)
{
    trace_assert(layer);
    trace_assert(event);
    trace_assert(undo_history);

    int selected = 0;
    if (color_picker_event(&layer->color_picker, event, &selected, undo_history) < 0) {
        return -1;
    }

    if (selected) {
        if (layer->selection >= 0) {
            Color *colors = dynarray_data(layer->colors);
            colors[layer->selection] = color_picker_rgba(&layer->color_picker);
        }

        return 0;
    }

    switch (layer->state) {
    case RECT_LAYER_IDLE:
        return rect_layer_event_idle(layer, event, camera);

    case RECT_LAYER_CREATE:
        return rect_layer_event_create(layer, event, camera);

    case RECT_LAYER_RESIZE:
        return rect_layer_event_resize(layer, event, camera);

    case RECT_LAYER_MOVE:
        return rect_layer_event_move(layer, event, camera);

    case RECT_LAYER_ID_RENAME:
        return rect_layer_event_id_rename(layer, event, camera);
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
