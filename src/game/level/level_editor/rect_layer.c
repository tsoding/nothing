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
#include "undo_history.h"

#define RECT_LAYER_SELECTION_THICCNESS 10.0f
#define RECT_LAYER_ID_LABEL_SIZE vec(3.0f, 3.0f)
#define CREATE_AREA_THRESHOLD 10.0

// TODO(#1051): RectLayer does not support copy-pasting

static int clipboard = 0;
static Rect clipboard_rect;
static Color clipboard_color;

typedef enum {
    RECT_LAYER_IDLE = 0,
    RECT_LAYER_CREATE,
    // TODO(#955): Rectangles in Level Editor have only one resize anchor to work with
    RECT_LAYER_RESIZE,
    RECT_LAYER_MOVE,
    RECT_LAYER_ID_RENAME,
    RECT_LAYER_RECOLOR
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
    Color inter_color;
    Rect inter_rect;
};

typedef enum {
    UNDO_ADD,
    UNDO_DELETE,
    UNDO_UPDATE
} UndoType;

typedef struct {
    UndoType type;
    RectLayer *layer;
    Rect rect;
    Color color;
    char id[RECT_LAYER_ID_MAX_SIZE];
    size_t index;
} UndoContext;

static
UndoContext create_undo_context(RectLayer *rect_layer, UndoType type)
{
    trace_assert(rect_layer);

    size_t index = type == UNDO_ADD ? dynarray_count(rect_layer->rects) - 1 : (size_t) rect_layer->selection;

    UndoContext undo_context;
    undo_context.type = type;
    undo_context.layer = rect_layer;
    dynarray_copy_to(rect_layer->rects, &undo_context.rect, index);
    dynarray_copy_to(rect_layer->colors, &undo_context.color, index);
    dynarray_copy_to(rect_layer->ids, undo_context.id, index);
    undo_context.index = index;

    return undo_context;
}

static
void rect_layer_undo(void *context, size_t context_size)
{
    trace_assert(context);
    trace_assert(sizeof(UndoContext) == context_size);

    UndoContext *undo_context = context;
    RectLayer *rect_layer = undo_context->layer;

    switch (undo_context->type) {
    case UNDO_ADD: {
        dynarray_delete_at(rect_layer->rects, undo_context->index);
        dynarray_delete_at(rect_layer->colors, undo_context->index);
        dynarray_delete_at(rect_layer->ids, undo_context->index);
        rect_layer->selection = -1;
    } break;

    case UNDO_DELETE: {
        dynarray_insert_before(rect_layer->rects, undo_context->index, &undo_context->rect);
        dynarray_insert_before(rect_layer->colors, undo_context->index, &undo_context->color);
        dynarray_insert_before(rect_layer->ids, undo_context->index, &undo_context->id);
        rect_layer->selection = -1;
    } break;

    case UNDO_UPDATE: {
        dynarray_replace_at(rect_layer->rects, undo_context->index, &undo_context->rect);
        dynarray_replace_at(rect_layer->colors, undo_context->index, &undo_context->color);
        dynarray_replace_at(rect_layer->ids, undo_context->index, &undo_context->id);
    } break;
    }
}

#define UNDO_PUSH(LAYER, HISTORY, UNDO_TYPE)                            \
    do {                                                                \
        UndoContext context = create_undo_context(LAYER, UNDO_TYPE);    \
        undo_history_push(                                              \
            HISTORY,                                                    \
            rect_layer_undo,                                            \
            &context,                                                   \
            sizeof(context));                                           \
    } while(0)


static int rect_layer_add_rect(RectLayer *layer,
                               Rect rect,
                               Color color,
                               UndoHistory *undo_history)
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

    UNDO_PUSH(layer, undo_history, UNDO_ADD);

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

static Rect rect_layer_resize_anchor(const Camera *camera, Rect boundary_rect)
{
    const Rect overlay_rect =
        rect_scale(
            camera_rect(camera, boundary_rect),
            RECT_LAYER_SELECTION_THICCNESS * 0.5f);

    return rect(
        overlay_rect.x + overlay_rect.w,
        overlay_rect.y + overlay_rect.h,
        RECT_LAYER_SELECTION_THICCNESS * 2.0f,
        RECT_LAYER_SELECTION_THICCNESS * 2.0f);
}

static int rect_layer_delete_rect_at(RectLayer *layer,
                                     size_t i,
                                     UndoHistory *undo_history)
{
    trace_assert(layer);

    UNDO_PUSH(layer, undo_history, UNDO_DELETE);

    dynarray_delete_at(layer->rects, i);
    dynarray_delete_at(layer->colors, i);
    dynarray_delete_at(layer->ids, i);

    return 0;
}

static int rect_layer_event_idle(RectLayer *layer,
                                 const SDL_Event *event,
                                 const Camera *camera,
                                 UndoHistory *undo_history)
{
    trace_assert(layer);
    trace_assert(event);
    trace_assert(camera);

    int color_changed = 0;
    if (color_picker_event(&layer->color_picker, event, camera, &color_changed) < 0) {
        return -1;
    }

    if (color_changed) {
        if (layer->selection >= 0) {
            dynarray_copy_to(layer->colors, &layer->inter_color, (size_t)layer->selection);
            layer->state = RECT_LAYER_RECOLOR;
        }
        return 0;
    }

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

            Rect *rects = dynarray_data(layer->rects);
            Color *colors = dynarray_data(layer->colors);

            if (rect_at_position >= 0) {
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

                dynarray_copy_to(layer->rects, &layer->inter_rect, (size_t) rect_at_position);
            } else if (layer->selection >= 0 && rect_contains_point(
                           rect_layer_resize_anchor(
                               camera,
                               rects[layer->selection]),
                           vec(
                               (float) event->button.x,
                               (float) event->button.y))) {
                layer->state = RECT_LAYER_RESIZE;
                dynarray_copy_to(layer->rects, &layer->inter_rect, (size_t) layer->selection);
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
                rect_layer_delete_rect_at(layer, (size_t) layer->selection, undo_history);
                layer->selection = -1;
            }
        } break;

        case SDLK_F2: {
            if (layer->selection >= 0) {
                const char *ids = dynarray_data(layer->ids);
                Color *colors = dynarray_data(layer->colors);

                edit_field_restyle(
                    layer->id_edit_field,
                    RECT_LAYER_ID_LABEL_SIZE,
                    color_invert(colors[layer->selection]));

                layer->state = RECT_LAYER_ID_RENAME;
                edit_field_replace(
                    layer->id_edit_field,
                    ids + layer->selection * RECT_LAYER_ID_MAX_SIZE);
                SDL_StartTextInput();
            }
        } break;

        case SDLK_c: {
            if ((event->key.keysym.mod & KMOD_LCTRL) && layer->selection >= 0) {
                clipboard = 1;
                dynarray_copy_to(layer->rects, &clipboard_rect, (size_t)layer->selection);
                dynarray_copy_to(layer->colors, &clipboard_color, (size_t)layer->selection);
            }
        } break;

        case SDLK_v: {
            if ((event->key.keysym.mod & KMOD_LCTRL) && clipboard) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                Point position = camera_map_screen(camera, x, y);

                rect_layer_add_rect(
                    layer,
                    rect(position.x, position.y,
                         clipboard_rect.w, clipboard_rect.h),
                    clipboard_color,
                    undo_history);
            }
        } break;
        }
    } break;
    }

    return 0;
}

static int rect_layer_event_create(RectLayer *layer,
                                   const SDL_Event *event,
                                   const Camera *camera,
                                   UndoHistory *undo_history)
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
                    color_picker_rgba(&layer->color_picker),
                    undo_history);
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

static int rect_layer_event_resize(RectLayer *layer,
                                   const SDL_Event *event,
                                   const Camera *camera,
                                   UndoHistory *undo_history)
{
    trace_assert(layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(layer->selection >= 0);

    switch (event->type) {
    case SDL_MOUSEMOTION: {
        layer->inter_rect = rect_from_points(
            vec(layer->inter_rect.x, layer->inter_rect.y),
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
        UNDO_PUSH(layer, undo_history, UNDO_UPDATE);
        dynarray_replace_at(layer->rects, (size_t) layer->selection, &layer->inter_rect);
    } break;
    }

    return 0;
}

static int rect_layer_event_move(RectLayer *layer,
                                 const SDL_Event *event,
                                 const Camera *camera,
                                 UndoHistory *undo_history)
{
    trace_assert(layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(layer->selection >= 0);

    switch (event->type) {
    case SDL_MOUSEMOTION: {
        Point position = vec_sub(
            camera_map_screen(
                camera,
                event->button.x,
                event->button.y),
            layer->move_anchor);

        trace_assert(layer->selection >= 0);

        layer->inter_rect.x = position.x;
        layer->inter_rect.y = position.y;
    } break;

    case SDL_MOUSEBUTTONUP: {
        layer->state = RECT_LAYER_IDLE;
        UNDO_PUSH(layer, undo_history, UNDO_UPDATE);
        dynarray_replace_at(layer->rects, (size_t) layer->selection, &layer->inter_rect);
    } break;
    }
    return 0;
}

static int rect_layer_event_id_rename(RectLayer *layer,
                                      const SDL_Event *event,
                                      const Camera *camera,
                                      UndoHistory *undo_history)
{
    trace_assert(layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(layer->selection >= 0);

    switch (event->type) {
    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_RETURN: {
            UNDO_PUSH(layer, undo_history, UNDO_UPDATE);

            char *id = dynarray_pointer_at(layer->ids, (size_t)layer->selection);
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
            RECT_LAYER_ID_LABEL_SIZE,
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
        Rect rect = rects[i];
        Color color = colors[i];

        if (layer->selection == (int) i) {
            if (layer->state == RECT_LAYER_RESIZE || layer->state == RECT_LAYER_MOVE) {
                rect = layer->inter_rect;
            }

            if (layer->state == RECT_LAYER_RECOLOR) {
                color = layer->inter_color;
            }
        }

        if (camera_fill_rect(
                camera,
                rect,
                color_scale(
                    color,
                    rgba(1.0f, 1.0f, 1.0f, active ? 1.0f : 0.5f))) < 0) {
            return -1;
        }

        // Selection Overlay
        if (active && (size_t) layer->selection == i) {
            const Rect overlay_rect =
                rect_scale(
                    camera_rect(camera, rect),
                    RECT_LAYER_SELECTION_THICCNESS * 0.5f);
            const Color overlay_color = color_invert(color);

            // Main Rectangle
            if (camera_fill_rect(
                    camera,
                    rect,
                    color_scale(
                        color,
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
            if (layer->state == RECT_LAYER_ID_RENAME) {
                // ID renaming Edit Field
                if (edit_field_render_world(
                        layer->id_edit_field,
                        camera,
                        rect_position(rect)) < 0) {
                    return -1;
                }
            } else {
                // Id text
                if (camera_render_text(
                        camera,
                        ids + layer->selection * RECT_LAYER_ID_MAX_SIZE,
                        RECT_LAYER_ID_LABEL_SIZE,
                        color_invert(color),
                        rect_position(rect)) < 0) {
                    return -1;
                }
            }

            // Resize Anchor
            if (camera_fill_rect_screen(
                    camera,
                    rect_layer_resize_anchor(camera, rect),
                    overlay_color) < 0) {
                return -1;
            }
        }
    }

    // Proto Rectangle
    const Color color = color_picker_rgba(&layer->color_picker);
    if (layer->state == RECT_LAYER_CREATE) {
        if (camera_fill_rect(camera, rect_from_points(layer->create_begin, layer->create_end), color) < 0) {
            return -1;
        }
    }

    if (active && color_picker_render(&layer->color_picker, camera) < 0) {
        return -1;
    }

    return 0;
}

static
int rect_layer_event_recolor(RectLayer *layer,
                             const SDL_Event *event,
                             const Camera *camera,
                             UndoHistory *undo_history)
{
    trace_assert(layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(undo_history);
    trace_assert(layer->selection >= 0);

    int color_changed = 0;
    if (color_picker_event(&layer->color_picker, event, camera, &color_changed) < 0) {
        return -1;
    }

    if (color_changed) {
        layer->inter_color = color_picker_rgba(&layer->color_picker);

        if (!color_picker_drag(&layer->color_picker)) {
            UNDO_PUSH(layer, undo_history, UNDO_UPDATE);
            dynarray_replace_at(layer->colors, (size_t) layer->selection, &layer->inter_color);
            layer->state = RECT_LAYER_IDLE;
        }
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

    switch (layer->state) {
    case RECT_LAYER_IDLE:
        return rect_layer_event_idle(layer, event, camera, undo_history);

    case RECT_LAYER_CREATE:
        return rect_layer_event_create(layer, event, camera, undo_history);

    case RECT_LAYER_RESIZE:
        return rect_layer_event_resize(layer, event, camera, undo_history);

    case RECT_LAYER_MOVE:
        return rect_layer_event_move(layer, event, camera, undo_history);

    case RECT_LAYER_ID_RENAME:
        return rect_layer_event_id_rename(layer, event, camera, undo_history);

    case RECT_LAYER_RECOLOR:
        return rect_layer_event_recolor(layer, event, camera, undo_history);
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
