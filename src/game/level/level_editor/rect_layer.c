#include <float.h>
#include <errno.h>

#include "game/camera.h"
#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "system/log.h"
#include "math/rect.h"
#include "color.h"
#include "rect_layer.h"
#include "dynarray.h"
#include "system/str.h"
#include "undo_history.h"
#include "game/level/action.h"
#include "game.h"
#include "math/extrema.h"

#define RECT_LAYER_SELECTION_THICCNESS 15.0f
#define RECT_LAYER_ID_LABEL_SIZE vec(3.0f, 3.0f)
#define CREATE_AREA_THRESHOLD 10.0

static int rect_clipboard = 0;
static Rect rect_clipboard_rect;
static Color rect_clipboard_color;

static Cursor_Style resize_styles[1 << RECT_SIDE_N] = {
    0,                         // [0]
    CURSOR_STYLE_RESIZE_VERT,  // [1]
    CURSOR_STYLE_RESIZE_HORIS, // [2]
    CURSOR_STYLE_RESIZE_DIAG1, // [3]
    CURSOR_STYLE_RESIZE_VERT,  // [4]
    0,                         // [5]
    CURSOR_STYLE_RESIZE_DIAG2, // [6]
    0,                         // [7]
    CURSOR_STYLE_RESIZE_HORIS, // [8]
    CURSOR_STYLE_RESIZE_DIAG2, // [9]
    0,                         // [10]
    0,                         // [11]
    CURSOR_STYLE_RESIZE_DIAG1  // [12]
};

typedef enum {
    RECT_UNDO_ADD,
    RECT_UNDO_DELETE,
    RECT_UNDO_UPDATE,
    RECT_UNDO_SWAP
} RectUndoType;

// Delete, Update
typedef struct {
    RectUndoType type;
    RectLayer *layer;
    size_t index;
    Rect rect;
    Color color;
    Action action;
    char id[ENTITY_MAX_ID_SIZE];
} UndoElementContext;

// Add
typedef struct {
    RectUndoType type;
    RectLayer *layer;
    size_t index;
} UndoAddContext;

// Swap
typedef struct {
    RectUndoType type;
    RectLayer *layer;
    size_t index1;
    size_t index2;
} UndoSwapContext;

typedef union {
    RectUndoType type;
    UndoAddContext add;
    UndoElementContext element;
    UndoSwapContext swap;
} RectUndoContext;

static
RectUndoContext create_rect_undo_add_context(RectLayer *layer, size_t index)
{
    trace_assert(layer);
    trace_assert(index < layer->rects.count);

    RectUndoContext undo_context;
    undo_context.add.type = RECT_UNDO_ADD;
    undo_context.add.layer = layer;
    undo_context.add.index = index;
    return undo_context;
}

static
RectUndoContext create_rect_undo_element_context(RectLayer *layer, size_t index)
{
    trace_assert(layer);
    trace_assert(index < layer->rects.count);

    RectUndoContext undo_context;
    undo_context.element.layer = layer;
    undo_context.element.index = index;
    dynarray_copy_to(&layer->rects, &undo_context.element.rect, index);
    dynarray_copy_to(&layer->colors, &undo_context.element.color, index);
    dynarray_copy_to(&layer->ids, undo_context.element.id, index);
    dynarray_copy_to(&layer->actions, &undo_context.element.action, index);
    return undo_context;
}

static
RectUndoContext create_rect_undo_update_context(RectLayer *rect_layer, size_t index)
{
    RectUndoContext undo_context = create_rect_undo_element_context(rect_layer, index);
    undo_context.type = RECT_UNDO_UPDATE;
    return undo_context;
}

static
RectUndoContext create_rect_undo_delete_context(RectLayer *rect_layer, size_t i)
{
    RectUndoContext undo_context = create_rect_undo_element_context(rect_layer, i);
    undo_context.type = RECT_UNDO_DELETE;
    return undo_context;
}

static
RectUndoContext create_rect_undo_swap_context(RectLayer *rect_layer, size_t index1, size_t index2)
{
    RectUndoContext undo_context;
    undo_context.swap.type = RECT_UNDO_SWAP;
    undo_context.swap.layer = rect_layer;
    undo_context.swap.index1 = index1;
    undo_context.swap.index2 = index2;
    return undo_context;
}

static
void rect_layer_undo(void *context, size_t context_size)
{
    trace_assert(context);
    trace_assert(sizeof(RectUndoContext) == context_size);

    RectUndoContext *undo_context = context;

    switch (undo_context->type) {
    case RECT_UNDO_ADD: {
        RectLayer *layer = undo_context->add.layer;
        dynarray_delete_at(&layer->rects, undo_context->add.index);
        dynarray_delete_at(&layer->colors, undo_context->add.index);
        dynarray_delete_at(&layer->ids, undo_context->add.index);
        dynarray_delete_at(&layer->actions, undo_context->add.index);
        layer->selection = -1;
    } break;

    case RECT_UNDO_DELETE: {
        RectLayer *layer = undo_context->element.layer;
        dynarray_insert_before(&layer->rects, undo_context->element.index, &undo_context->element.rect);
        dynarray_insert_before(&layer->colors, undo_context->element.index, &undo_context->element.color);
        dynarray_insert_before(&layer->ids, undo_context->element.index, &undo_context->element.id);
        dynarray_insert_before(&layer->actions, undo_context->element.index, &undo_context->element.action);
        layer->selection = -1;
    } break;

    case RECT_UNDO_UPDATE: {
        RectLayer *layer = undo_context->element.layer;
        dynarray_replace_at(&layer->rects, undo_context->element.index, &undo_context->element.rect);
        dynarray_replace_at(&layer->colors, undo_context->element.index, &undo_context->element.color);
        dynarray_replace_at(&layer->ids, undo_context->element.index, &undo_context->element.id);
        dynarray_replace_at(&layer->actions, undo_context->element.index, &undo_context->element.action);
    } break;

    case RECT_UNDO_SWAP: {
        RectLayer *layer = undo_context->element.layer;
        dynarray_swap(&layer->rects, undo_context->swap.index1, undo_context->swap.index2);
        dynarray_swap(&layer->colors, undo_context->swap.index1, undo_context->swap.index2);
        dynarray_swap(&layer->ids, undo_context->swap.index1, undo_context->swap.index2);
        dynarray_swap(&layer->actions, undo_context->swap.index1, undo_context->swap.index2);
    } break;
    }
}

#define RECT_UNDO_PUSH(HISTORY, CONTEXT)                                     \
    do {                                                                \
        RectUndoContext context = (CONTEXT);                                \
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

    dynarray_push(&layer->rects, &rect);
    dynarray_push(&layer->colors, &color);

    char id[ENTITY_MAX_ID_SIZE];
    snprintf(id, ENTITY_MAX_ID_SIZE, "%s_%d",
             layer->id_name_prefix,
             layer->id_name_counter++);
    dynarray_push(&layer->ids, id);

    dynarray_push_empty(&layer->actions);

    RECT_UNDO_PUSH(
        undo_history,
        create_rect_undo_add_context(
            layer,
            layer->rects.count - 1));

    return 0;
}

static int rect_layer_rect_at(RectLayer *layer, Vec2f position)
{
    trace_assert(layer);

    int n = (int) layer->rects.count;
    Rect *rects = (Rect*)layer->rects.data;

    for (int i = n - 1; i >= 0; --i) {
        if (rect_contains_point(rects[i], position)) {
            return (int) i;
        }
    }

    return -1;
}

static void rect_layer_swap_elements(RectLayer *layer, size_t a, size_t b,
                                     UndoHistory *undo_history)
{
    trace_assert(layer);
    trace_assert(a < layer->rects.count);
    trace_assert(b < layer->rects.count);

    dynarray_swap(&layer->rects, a, b);
    dynarray_swap(&layer->colors, a, b);
    dynarray_swap(&layer->ids, a, b);
    dynarray_swap(&layer->actions, a, b);

    RECT_UNDO_PUSH(undo_history, create_rect_undo_swap_context(layer, a, b));
}

static int rect_layer_delete_rect_at_index(RectLayer *layer,
                                     size_t i,
                                     UndoHistory *undo_history)
{
    trace_assert(layer);

    RECT_UNDO_PUSH(undo_history, create_rect_undo_delete_context(layer, i));

    dynarray_delete_at(&layer->rects, i);
    dynarray_delete_at(&layer->colors, i);
    dynarray_delete_at(&layer->ids, i);
    dynarray_delete_at(&layer->actions, i);

    return 0;
}

static int calc_resize_mask(Vec2f point, Rect rect)
{
    int mask = 0;
    for (Rect_side side = 0; side < RECT_SIDE_N; ++side) {
        if (rect_side_distance(rect, point, side) < RECT_LAYER_SELECTION_THICCNESS) {
            mask = mask | (1 << side);
        }
    }
    return mask;
}

#define TOOL_BUTTON_WIDTH 50.0f
#define TOOL_BUTTON_HEIGHT 50.0f
#define TOOL_BAR_PADDING 20.0f
#define TOOL_BAR_BACKGROUND rgba(0.8f, 0.8f, 0.8f, 1.0f)
#define TOOL_BAR_FOREGROUND rgba(0.2f, 0.2f, 0.2f, 1.0f)

static
Rect subtract_tool_button_rect(const Camera *camera)
{
    const Rect view_port = camera_view_port_screen(camera);
    return rect(
        TOOL_BAR_PADDING,
        view_port.h - TOOL_BUTTON_HEIGHT - TOOL_BAR_PADDING,
        TOOL_BUTTON_WIDTH,
        TOOL_BUTTON_HEIGHT);
}

static
Rect snapping_tool_button_rect(const Camera *camera)
{
    const Rect view_port = camera_view_port_screen(camera);
    return rect(
        TOOL_BAR_PADDING + TOOL_BUTTON_WIDTH + TOOL_BAR_PADDING,
        view_port.h - TOOL_BUTTON_HEIGHT - TOOL_BAR_PADDING,
        TOOL_BUTTON_WIDTH,
        TOOL_BUTTON_HEIGHT);
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
            dynarray_copy_to(&layer->colors, &layer->inter_color, (size_t)layer->selection);
            layer->state = RECT_LAYER_RECOLOR;
        }
        return 0;
    }

    Rect *rects = (Rect*)layer->rects.data;

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            Vec2f screen_position =
                vec((float) event->button.x,
                    (float) event->button.y);
            if (rect_contains_point(subtract_tool_button_rect(camera), screen_position)) {
                layer->subtract_enabled = !layer->subtract_enabled;
                return 0;
            }

            if (rect_contains_point(snapping_tool_button_rect(camera), screen_position)) {
                layer->snapping_enabled = !layer->snapping_enabled;
                return 0;
            }

            Vec2f position = camera_map_screen(
                camera,
                event->button.x,
                event->button.y);
            int rect_at_position =
                rect_layer_rect_at(layer, position);


            Color *colors = (Color*)layer->colors.data;

            if (layer->subtract_enabled) {
                layer->state = RECT_LAYER_SUBTRACT;
                layer->create_begin = position;
                layer->create_end = position;
            } else if (layer->selection >= 0 &&
                       layer->selection == rect_at_position &&
                       (layer->resize_mask = calc_resize_mask(
                           vec((float) event->button.x, (float)event->button.y),
                           camera_rect(camera, rects[layer->selection])))) {
                layer->state = RECT_LAYER_RESIZE;
                dynarray_copy_to(&layer->rects, &layer->inter_rect, (size_t) layer->selection);
            } else if (rect_at_position >= 0) {
                layer->selection = rect_at_position;
                layer->state = RECT_LAYER_MOVE;
                layer->move_anchor = vec_sub(
                    position,
                    vec(
                        rects[layer->selection].x,
                        rects[layer->selection].y));
                layer->color_picker =
                    create_color_picker_from_rgba(colors[rect_at_position]);
                dynarray_copy_to(&layer->rects, &layer->inter_rect, (size_t) rect_at_position);
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

    case SDL_MOUSEMOTION: {
        int resize_mask = 0;
        Vec2f position = camera_map_screen(
            camera,
            event->button.x,
            event->button.y);
        if (layer->selection >= 0 &&
            layer->selection == rect_layer_rect_at(layer, position) &&
            (resize_mask = calc_resize_mask(
                vec((float) event->button.x, (float)event->button.y),
                camera_rect(camera, rects[layer->selection])))) {
            layer->cursor->style = resize_styles[resize_mask];
        } else {
            layer->cursor->style = CURSOR_STYLE_POINTER;
        }
    } break;

    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_UP: {
            if ((event->key.keysym.mod & KMOD_SHIFT)
                && (layer->selection >= 0)
                && ((size_t)(layer->selection + 1) < layer->rects.count)) {
                rect_layer_swap_elements(
                    layer,
                    (size_t) layer->selection,
                    (size_t) layer->selection + 1,
                    undo_history);
                layer->selection++;
            }
        } break;

        case SDLK_DOWN: {
            if ((event->key.keysym.mod & KMOD_SHIFT)
                && (layer->selection > 0)
                && ((size_t) layer->selection < layer->rects.count)) {
                rect_layer_swap_elements(
                    layer,
                    (size_t) layer->selection,
                    (size_t) layer->selection - 1,
                    undo_history);
                layer->selection--;
            }
        } break;

        case SDLK_DELETE: {
            if (layer->selection >= 0) {
                rect_layer_delete_rect_at_index(
                    layer,
                    (size_t) layer->selection,
                    undo_history);
                layer->selection = -1;
            }
        } break;

        case SDLK_q: {
            // TODO(#1171): there is no UI indication that we are in the snapping mode
            layer->snapping_enabled = !layer->snapping_enabled;
        } break;

        case SDLK_F2: {
            if (layer->selection >= 0) {
                const char *ids = (char*)layer->ids.data;
                Color *colors = (Color*)layer->colors.data;

                edit_field_restyle(
                    &layer->id_edit_field,
                    RECT_LAYER_ID_LABEL_SIZE,
                    color_invert(colors[layer->selection]));

                layer->state = RECT_LAYER_ID_RENAME;
                edit_field_replace(
                    &layer->id_edit_field,
                    ids + layer->selection * ENTITY_MAX_ID_SIZE);
                SDL_StartTextInput();
            }
        } break;

        case SDLK_c: {
            if ((event->key.keysym.mod & KMOD_LCTRL) && layer->selection >= 0) {
                rect_clipboard = 1;
                dynarray_copy_to(&layer->rects, &rect_clipboard_rect, (size_t)layer->selection);
                dynarray_copy_to(&layer->colors, &rect_clipboard_color, (size_t)layer->selection);
            }
        } break;

        case SDLK_v: {
            if ((event->key.keysym.mod & KMOD_LCTRL) && rect_clipboard) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                Vec2f position = camera_map_screen(camera, x, y);

                rect_layer_add_rect(
                    layer,
                    rect(position.x, position.y,
                         rect_clipboard_rect.w, rect_clipboard_rect.h),
                    rect_clipboard_color,
                    undo_history);
            }
        } break;
        }
    } break;
    }

    return 0;
}

#define GEOMETRY_CAPACITY 256

typedef struct {
    size_t first;
    size_t count;
    Rect rects[GEOMETRY_CAPACITY];
    Color colors[GEOMETRY_CAPACITY];
} Geometry;

static
void push_geometry(Geometry *geometry, Rect rect, Color color)
{
    assert(geometry);
    // TODO(#1252): push_geometry may fail if there is too many rects produced
    assert(geometry->count < GEOMETRY_CAPACITY);

    if ((rect.w * rect.h) > 1e-6f) {
        size_t i = (geometry->first + geometry->count) % GEOMETRY_CAPACITY;
        geometry->rects[i] = rect;
        geometry->colors[i] = color;
        geometry->count++;
    }
}

static
void subtract_rect_from_rect(Rect a, Color color_a, Rect c, Geometry *result)
{
    assert(result);

    Rect b = rects_overlap_area(a, c);

    if (b.w * b.h < 1e-6) {
        push_geometry(result, a, color_a);
        return;
    }

    push_geometry(result, (Rect) {a.x, a.y, a.w, b.y - a.y}, color_a);
    push_geometry(result, (Rect) {a.x, b.y, b.x - a.x, b.h}, color_a);
    push_geometry(result, (Rect) {
        b.x + b.w,
        b.y,
        a.w - (b.x - a.x) - b.w,
        b.h
    }, color_a);
    push_geometry(result, (Rect) {
        a.x,
        b.y + b.h,
        a.w,
        a.h - (b.y - a.y) - b.h
    }, color_a);
}

static
void subtract_rect_from_geometry(Geometry *result, Rect b)
{
    assert(result);

    size_t count = result->count;
    size_t first = result->first;

    for (size_t i = 0; i < count; ++i) {
        result->first = (result->first + 1) % GEOMETRY_CAPACITY;
        result->count -= 1;
        subtract_rect_from_rect(
            result->rects[(i + first) % GEOMETRY_CAPACITY],
            result->colors[(i + first) % GEOMETRY_CAPACITY],
            b,
            result);
    }
}

static int rect_layer_event_subtract(RectLayer *layer,
                                     const SDL_Event *event,
                                     const Camera *camera,
                                     UndoHistory *undo_history)
{
    trace_assert(layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(undo_history);

    Rect *rects = layer->rects.data;
    Color *colors = layer->colors.data;

    switch (event->type) {
    case SDL_MOUSEBUTTONUP: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            const Rect real_rect =
                rect_from_points(
                    layer->create_begin,
                    layer->create_end);
            const float area = real_rect.w * real_rect.h;

            Geometry geometry = {0};

            if (area >= CREATE_AREA_THRESHOLD) {
                for (size_t i = 0; i < layer->rects.count;) {
                    Rect overlap_area = rects_overlap_area(
                        real_rect,
                        rects[i]);
                    if (overlap_area.w * overlap_area.h > 1e-6) {
                        push_geometry(&geometry, rects[i], colors[i]);
                        rect_layer_delete_rect_at_index(layer, i, undo_history);
                    } else {
                        i++;
                    }
                }

                subtract_rect_from_geometry(&geometry, real_rect);

                for (size_t i = 0; i < geometry.count; ++i) {
                    size_t j = (i + geometry.first) % GEOMETRY_CAPACITY;
                    rect_layer_add_rect(
                        layer,
                        geometry.rects[j],
                        geometry.colors[j],
                        undo_history);
                }
            } else {
                log_info("The area is too small %f. Such small box won't be cut.\n", area);
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

static
void snap_rect_resize_if_enabled(RectLayer *layer, Rect *a, float snapping_threshold)
{
    trace_assert(layer);
    trace_assert(layer->selection >= 0);
    trace_assert(a);

    if (!layer->snapping_enabled) return;

    Rect *rects = (Rect*)layer->rects.data;
    size_t rects_size = layer->rects.count;

    for (size_t i = 0; i < rects_size; ++i) {
        if (i == (size_t) layer->selection) continue;

        const Rect b = rects[i];

        if (segment_overlap(vec(a->x, a->x + a->w), vec(b.x, b.x + b.w))) {
            snap_var2seg(&a->y, b.y, 0, b.h, snapping_threshold);
        }

        if (segment_overlap(vec(a->y, a->y + a->h), vec(b.y, b.y + b.h))) {
            snap_var2seg(&a->x, b.x, 0, b.w, snapping_threshold);
        }
    }
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

    Rect *rects = (Rect*)layer->rects.data;

    float scaled_snap_threshold = SNAPPING_THRESHOLD / camera->scale;

    switch (event->type) {
    case SDL_MOUSEMOTION: {
        Vec2f position = camera_map_screen(
            camera,
            event->button.x,
            event->button.y);

        switch (layer->resize_mask) {
        case 1: {               // TOP
            Rect a = rect(rects[layer->selection].x,
                          position.y,
                          rects[layer->selection].w,
                          rects[layer->selection].h);

            snap_rect_resize_if_enabled(layer, &a, scaled_snap_threshold);

            layer->inter_rect = rect_from_points(
                vec(a.x, a.y),
                rect_position2(rects[layer->selection]));
        } break;

        case 2: {               // LEFT
            Rect a = rect(position.x,
                          rects[layer->selection].y,
                          rects[layer->selection].w,
                          rects[layer->selection].h);

            snap_rect_resize_if_enabled(layer, &a, scaled_snap_threshold);

            layer->inter_rect = rect_from_points(
                vec(a.x, a.y),
                rect_position2(rects[layer->selection]));
        } break;

        case 3: {               // TOP,LEFT
            Rect a = rect(
                position.x,
                position.y,
                rects[layer->selection].w,
                rects[layer->selection].h);

            snap_rect_resize_if_enabled(layer, &a, scaled_snap_threshold);

            layer->inter_rect = rect_from_points(
                vec(a.x, a.y),
                rect_position2(rects[layer->selection]));
        } break;

        case 4: {               // BOTTOM
            Rect a = rect(rects[layer->selection].x,
                          position.y,
                          rects[layer->selection].w,
                          rects[layer->selection].h);

            snap_rect_resize_if_enabled(layer, &a, scaled_snap_threshold);

            layer->inter_rect = rect_from_points(
                rect_position(rects[layer->selection]),
                vec(rects[layer->selection].x + rects[layer->selection].w,
                    a.y));
        } break;

        case 6: {               // BOTTOM,LEFT
            Rect a = rect(
                position.x,
                position.y,
                rects[layer->selection].w,
                -rects[layer->selection].h);

            snap_rect_resize_if_enabled(layer, &a, scaled_snap_threshold);

            layer->inter_rect = rect_from_points(
                vec(a.x, rects[layer->selection].y),
                vec(rects[layer->selection].x + rects[layer->selection].w,
                    a.y));
        } break;

        case 8: {               // RIGHT
            Rect a = rect(position.x,
                          rects[layer->selection].y,
                          rects[layer->selection].w,
                          rects[layer->selection].h);

            snap_rect_resize_if_enabled(layer, &a, scaled_snap_threshold);

            layer->inter_rect = rect_from_points(
                rect_position(rects[layer->selection]),
                vec(a.x, rects[layer->selection].y + rects[layer->selection].h));
        } break;

        case 9: {               // TOP,RIGHT
            Rect a = rect(
                position.x,
                position.y,
                -rects[layer->selection].w,
                rects[layer->selection].h);

            snap_rect_resize_if_enabled(layer, &a, scaled_snap_threshold);

            layer->inter_rect = rect_from_points(
                vec(rects[layer->selection].x, a.y),
                vec(a.x,
                    rects[layer->selection].y + rects[layer->selection].h));
        } break;

        case 12: {              // BOTTOM,RIGHT
            Rect a = rect(
                position.x,
                position.y,
                -rects[layer->selection].w,
                -rects[layer->selection].h);

            snap_rect_resize_if_enabled(layer, &a, scaled_snap_threshold);

            layer->inter_rect = rect_from_points(
                rect_position(rects[layer->selection]),
                vec(a.x, a.y));
        } break;
        }

    } break;

    case SDL_MOUSEBUTTONUP: {
        layer->state = RECT_LAYER_IDLE;
        RECT_UNDO_PUSH(
            undo_history,
            create_rect_undo_update_context(
                layer,
                (size_t) layer->selection));
        dynarray_replace_at(&layer->rects, (size_t) layer->selection, &layer->inter_rect);
    } break;
    }

    return 0;
}

static
void snap_rect_move_if_enabled(RectLayer *layer, Rect *a,
                               float snapping_threshold)
{
    trace_assert(a);
    trace_assert(layer);
    trace_assert(layer->selection >= 0);

    if (!layer->snapping_enabled) return;

    Rect *rects = (Rect*)layer->rects.data;
    size_t rects_size = layer->rects.count;

    for (size_t i = 0; i < rects_size; ++i) {
        if (i == (size_t) layer->selection) continue;

        const Rect b = rects[i];

        if (segment_overlap(vec(a->x, a->x + a->w), vec(b.x,  b.x  + b.w))) {
            snap_seg2seg(&a->y, b.y, a->h, b.h, snapping_threshold);
        }

        if (segment_overlap(vec(a->y, a->y + a->h), vec(b.y,  b.y  + b.h))) {
            snap_seg2seg(&a->x, b.x, a->w, b.w, snapping_threshold);
        }
    }
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

    Rect *rects = (Rect*)layer->rects.data;

    switch (event->type) {
    case SDL_MOUSEMOTION: {
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        const Vec2f mouse_pos = vec_sub(
            camera_map_screen(
                camera,
                event->button.x,
                event->button.y),
            layer->move_anchor);

        if (!(state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL])) {
            layer->inter_rect.x = mouse_pos.x;
            layer->inter_rect.y = mouse_pos.y;
        } else {
            const Vec2f rect_pos = rect_position(rects[layer->selection]);

            const float dx = fabsf(rect_pos.x - mouse_pos.x);
            const float dy = fabsf(rect_pos.y - mouse_pos.y);

            if (dx > dy) {
                layer->inter_rect.x = mouse_pos.x;
                layer->inter_rect.y = rect_pos.y;
            } else {
                layer->inter_rect.x = rect_pos.x;
                layer->inter_rect.y = mouse_pos.y;
            }
        }

        snap_rect_move_if_enabled(layer, &layer->inter_rect,
                                  SNAPPING_THRESHOLD / camera->scale);
    } break;

    case SDL_MOUSEBUTTONUP: {
        layer->state = RECT_LAYER_IDLE;

        float distance = vec_length(
            vec_sub(rect_position(layer->inter_rect),
                    rect_position(rects[layer->selection])));

        if (distance > 1e-6) {
            RECT_UNDO_PUSH(
                undo_history,
                create_rect_undo_update_context(
                    layer, (size_t) layer->selection));
            dynarray_replace_at(
                &layer->rects,
                (size_t) layer->selection,
                &layer->inter_rect);
        }
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
            RECT_UNDO_PUSH(
                undo_history,
                create_rect_undo_update_context(
                    layer,
                    (size_t) layer->selection));

            char *id = dynarray_pointer_at(&layer->ids, (size_t)layer->selection);
            memset(id, 0, ENTITY_MAX_ID_SIZE);
            memcpy(id, edit_field_as_text(&layer->id_edit_field), ENTITY_MAX_ID_SIZE - 1);
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

    return edit_field_event(&layer->id_edit_field, event);
}

LayerPtr rect_layer_as_layer(RectLayer *rect_layer)
{
    LayerPtr layer = {
        .type = LAYER_RECT,
        .ptr = rect_layer
    };
    return layer;
}

RectLayer *create_rect_layer(Memory *memory,
                             const char *id_name_prefix,
                             Cursor *cursor)
{
    trace_assert(memory);
    trace_assert(id_name_prefix);
    trace_assert(cursor);

    RectLayer *rect_layer = memory_alloc(memory, sizeof(RectLayer));

    rect_layer->ids = create_dynarray(memory, sizeof(char) * ENTITY_MAX_ID_SIZE);
    rect_layer->rects = create_dynarray(memory, sizeof(Rect));
    rect_layer->colors = create_dynarray(memory, sizeof(Color));
    rect_layer->actions = create_dynarray(memory, sizeof(Action));
    rect_layer->id_edit_field.font_size = RECT_LAYER_ID_LABEL_SIZE;
    rect_layer->id_edit_field.font_color = COLOR_BLACK;
    rect_layer->color_picker = create_color_picker_from_rgba(rgba(1.0f, 0.0f, 0.0f, 1.0f));
    rect_layer->selection = -1;
    rect_layer->id_name_prefix = id_name_prefix;
    rect_layer->cursor = cursor;

    return rect_layer;
}

void rect_layer_load(RectLayer *layer, Memory *memory, String *input)
{
    trace_assert(layer);
    trace_assert(memory);
    trace_assert(input);

    int n = atoi(string_to_cstr(memory, trim(chop_by_delim(input, '\n'))));
    char id[ENTITY_MAX_ID_SIZE];
    for (int i = 0; i < n; ++i) {
        Rect rect;
        String line = trim(chop_by_delim(input, '\n'));
        String string_id = trim(chop_word(&line));
        rect.x = strtof(string_to_cstr(memory, trim(chop_word(&line))), NULL);
        rect.y = strtof(string_to_cstr(memory, trim(chop_word(&line))), NULL);
        rect.w = strtof(string_to_cstr(memory, trim(chop_word(&line))), NULL);
        rect.h = strtof(string_to_cstr(memory, trim(chop_word(&line))), NULL);
        Color color = hexs(trim(chop_word(&line)));

        memset(id, 0, ENTITY_MAX_ID_SIZE);
        memcpy(
            id,
            string_id.data,
            min_size_t(ENTITY_MAX_ID_SIZE - 1, string_id.count));

        dynarray_push(&layer->rects, &rect);
        dynarray_push(&layer->colors, &color);
        dynarray_push(&layer->ids, id);

        Action action = {
            .type = ACTION_NONE,
            .entity_id = {0}
        };

        String action_string = trim(chop_word(&line));
        if (action_string.count > 0) {
            action.type = (ActionType)atol(string_to_cstr(memory, action_string));
            switch (action.type) {
            case ACTION_NONE: break;
            case ACTION_TOGGLE_GOAL:
            case ACTION_HIDE_LABEL: {
                String label_id = trim(chop_word(&line));
                trace_assert(label_id.count > 0);
                memset(action.entity_id, 0, ENTITY_MAX_ID_SIZE);
                memcpy(action.entity_id,
                       label_id.data,
                       min_size_t(
                           ENTITY_MAX_ID_SIZE - 1,
                           label_id.count));
            } break;

            case ACTION_N: break;
            }
        }

        dynarray_push(&layer->actions, &action);
    }
}

static
void render_tool_bar_button(const Camera *camera,
                            Rect button_rect, const char *text,
                            Color background_color,
                            Color foreground_color)
{
    const Vec2f text_size = vec(5.0f, 5.0f);
    const Rect text_rect = sprite_font_boundary_box(
        vec(0.0f, 0.0f), text_size, text);
    camera_fill_rect_screen(
        camera,
        button_rect,
        background_color);
    camera_render_text_screen(
        camera,
        text,
        text_size,
        foreground_color,
        vec(
            button_rect.x + (button_rect.w - text_rect.w) * 0.5f,
            button_rect.y + (button_rect.h - text_rect.h) * 0.5f));
}


int rect_layer_render(const RectLayer *layer, const Camera *camera, int active)
{
    trace_assert(layer);
    trace_assert(camera);

    const size_t n = layer->rects.count;
    Rect *rects = (Rect *)layer->rects.data;
    Color *colors = (Color *)layer->colors.data;
    const char *ids = (const char *)layer->ids.data;

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

        // Main Rectangle
        if (camera_fill_rect(
                camera,
                rect,
                color_scale(
                    color,
                    rgba(1.0f, 1.0f, 1.0f, active ? 1.0f : 0.5f))) < 0) {
            return -1;
        }
    }

    // Selection Overlay
    if (active && layer->selection >= 0) {
        Rect rect = rects[layer->selection];
        Color color = colors[layer->selection];

        if (layer->state == RECT_LAYER_RESIZE || layer->state == RECT_LAYER_MOVE) {
            rect = layer->inter_rect;
        }

        if (layer->state == RECT_LAYER_RECOLOR) {
            color = layer->inter_color;
        }

        const Rect overlay_rect =
            rect_pad(
                camera_rect(camera, rect),
                -RECT_LAYER_SELECTION_THICCNESS * 0.5f);
        const Color overlay_color = color_invert(color);

        // Selection
        if (camera_draw_thicc_rect_screen(
                camera,
                overlay_rect,
                overlay_color,
                RECT_LAYER_SELECTION_THICCNESS) < 0) {
            return -1;
        }

        const Vec2f rect_id_pos = vec_sub(
            rect_position(rect),
            vec_mult(
                RECT_LAYER_ID_LABEL_SIZE,
                vec(0.0f, FONT_CHAR_HEIGHT)));

        // Rectangle Id
        if (layer->state == RECT_LAYER_ID_RENAME) {
            // ID renaming Edit Field
            if (edit_field_render_world(
                    &layer->id_edit_field,
                    camera,
                    rect_id_pos) < 0) {
                return -1;
            }
        } else {
            // Id text
            if (camera_render_text(
                    camera,
                    ids + layer->selection * ENTITY_MAX_ID_SIZE,
                    RECT_LAYER_ID_LABEL_SIZE,
                    color_invert(color),
                    rect_id_pos) < 0) {
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

    if (layer->state == RECT_LAYER_SUBTRACT) {
        if (camera_draw_rect(camera, rect_from_points(layer->create_begin, layer->create_end), color) < 0) {
            return -1;
        }
    }

    if (active && color_picker_render(&layer->color_picker, camera) < 0) {
        return -1;
    }

    // Tool bar
    if (active) {
        render_tool_bar_button(
            camera, subtract_tool_button_rect(camera), "/",
            layer->subtract_enabled ? TOOL_BAR_BACKGROUND : TOOL_BAR_FOREGROUND,
            layer->subtract_enabled ? TOOL_BAR_FOREGROUND : TOOL_BAR_BACKGROUND);

        render_tool_bar_button(
            camera, snapping_tool_button_rect(camera), "S",
            layer->snapping_enabled ? TOOL_BAR_BACKGROUND : TOOL_BAR_FOREGROUND,
            layer->snapping_enabled ? TOOL_BAR_FOREGROUND : TOOL_BAR_BACKGROUND);
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
            RECT_UNDO_PUSH(
                undo_history,
                create_rect_undo_update_context(
                    layer,
                    (size_t)layer->selection));
            dynarray_replace_at(&layer->colors, (size_t) layer->selection, &layer->inter_color);
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

    case RECT_LAYER_SUBTRACT:
        return rect_layer_event_subtract(layer, event, camera, undo_history);
    }


    return 0;
}

size_t rect_layer_count(const RectLayer *layer)
{
    return layer->rects.count;
}

const Rect *rect_layer_rects(const RectLayer *layer)
{
    return (const Rect *)layer->rects.data;
}

const Color *rect_layer_colors(const RectLayer *layer)
{
    return (const Color *)layer->colors.data;
}

const char *rect_layer_ids(const RectLayer *layer)
{
    return (const char *)layer->ids.data;
}

int rect_layer_dump_stream(const RectLayer *layer, FILE *filedump)
{
    trace_assert(layer);
    trace_assert(filedump);

    size_t n = layer->ids.count;
    char *ids = (char *)layer->ids.data;
    Rect *rects = (Rect *)layer->rects.data;
    Color *colors = (Color *)layer->colors.data;
    Action *actions = (Action *)layer->actions.data;

    fprintf(filedump, "%zd\n", n);
    for (size_t i = 0; i < n; ++i) {
        fprintf(filedump, "%s %f %f %f %f ",
                ids + ENTITY_MAX_ID_SIZE * i,
                rects[i].x, rects[i].y, rects[i].w, rects[i].h);
        color_hex_to_stream(colors[i], filedump);

        switch (actions[i].type) {
        case ACTION_NONE: {} break;

        case ACTION_TOGGLE_GOAL:
        case ACTION_HIDE_LABEL: {
            fprintf(filedump, " %d %.*s",
                    (int)actions[i].type,
                    ENTITY_MAX_ID_SIZE, actions[i].entity_id);
        } break;
        case ACTION_N: break;
        }

        fprintf(filedump, "\n");
    }

    return 0;
}

const Action *rect_layer_actions(const RectLayer *layer)
{
    return (const Action *)layer->actions.data;
}
