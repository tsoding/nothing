#include <float.h>
#include <errno.h>

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
#include "game/level/action.h"
#include "action_picker.h"
#include "game.h"

#define RECT_LAYER_SELECTION_THICCNESS 15.0f
#define RECT_LAYER_ID_LABEL_SIZE vec(3.0f, 3.0f)
#define CREATE_AREA_THRESHOLD 10.0
#define RECT_LAYER_GRID_ROWS 3
#define RECT_LAYER_GRID_COLUMNS 4
#define SNAPPING_THRESHOLD 10.0f

static int clipboard = 0;
static Rect clipboard_rect;
static Color clipboard_color;

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
    RECT_LAYER_IDLE = 0,
    RECT_LAYER_CREATE,
    RECT_LAYER_RESIZE,
    RECT_LAYER_MOVE,
    RECT_LAYER_ID_RENAME,
    RECT_LAYER_RECOLOR
} RectLayerState;

struct RectLayer {
    Lt *lt;
    RectLayerState state;
    int resize_mask;
    Dynarray *ids;
    Dynarray *rects;
    Dynarray *colors;
    Dynarray *actions;
    ColorPicker color_picker;
    ActionPicker action_picker;
    Vec2f create_begin;
    Vec2f create_end;
    int selection;
    Vec2f move_anchor;          // The mouse offset from the left-top
                                // corner of the rect during moving it
    Edit_field *id_edit_field;
    Color inter_color;
    Rect inter_rect;
    int id_name_counter;
    const char *id_name_prefix;
    Grid *grid;
    Cursor *cursor;
};

typedef enum {
    UNDO_ADD,
    UNDO_DELETE,
    UNDO_UPDATE,
    UNDO_SWAP
} UndoType;

// Delete, Update
typedef struct {
    UndoType type;
    RectLayer *layer;
    size_t index;
    Rect rect;
    Color color;
    Action action;
    char id[ENTITY_MAX_ID_SIZE];
} UndoElementContext;

// Add
typedef struct {
    UndoType type;
    RectLayer *layer;
    size_t index;
} UndoAddContext;

// Swap
typedef struct {
    UndoType type;
    RectLayer *layer;
    size_t index1;
    size_t index2;
} UndoSwapContext;

typedef union {
    UndoType type;
    UndoAddContext add;
    UndoElementContext element;
    UndoSwapContext swap;
} UndoContext;

static
UndoContext create_undo_add_context(RectLayer *layer, size_t index)
{
    trace_assert(layer);
    trace_assert(index < dynarray_count(layer->rects));

    UndoContext undo_context;
    undo_context.add.type = UNDO_ADD;
    undo_context.add.layer = layer;
    undo_context.add.index = index;
    return undo_context;
}

static
UndoContext create_undo_element_context(RectLayer *layer)
{
    trace_assert(layer);
    size_t index = (size_t) layer->selection;
    trace_assert(index < dynarray_count(layer->rects));

    UndoContext undo_context;
    undo_context.element.layer = layer;
    undo_context.element.index = index;
    dynarray_copy_to(layer->rects, &undo_context.element.rect, index);
    dynarray_copy_to(layer->colors, &undo_context.element.color, index);
    dynarray_copy_to(layer->ids, undo_context.element.id, index);
    dynarray_copy_to(layer->actions, &undo_context.element.action, index);
    return undo_context;
}

static
UndoContext create_undo_update_context(RectLayer *rect_layer)
{
    UndoContext undo_context = create_undo_element_context(rect_layer);
    undo_context.type = UNDO_UPDATE;
    return undo_context;
}

static
UndoContext create_undo_delete_context(RectLayer *rect_layer)
{
    UndoContext undo_context = create_undo_element_context(rect_layer);
    undo_context.type = UNDO_DELETE;
    return undo_context;
}

static
UndoContext create_undo_swap_context(RectLayer *rect_layer, size_t index1, size_t index2)
{
    UndoContext undo_context;
    undo_context.swap.type = UNDO_SWAP;
    undo_context.swap.layer = rect_layer;
    undo_context.swap.index1 = index1;
    undo_context.swap.index2 = index2;
    return undo_context;
}

static
void rect_layer_undo(void *context, size_t context_size)
{
    trace_assert(context);
    trace_assert(sizeof(UndoContext) == context_size);

    UndoContext *undo_context = context;

    switch (undo_context->type) {
    case UNDO_ADD: {
        RectLayer *layer = undo_context->add.layer;
        dynarray_delete_at(layer->rects, undo_context->add.index);
        dynarray_delete_at(layer->colors, undo_context->add.index);
        dynarray_delete_at(layer->ids, undo_context->add.index);
        dynarray_delete_at(layer->actions, undo_context->add.index);
        layer->selection = -1;
    } break;

    case UNDO_DELETE: {
        RectLayer *layer = undo_context->element.layer;
        dynarray_insert_before(layer->rects, undo_context->element.index, &undo_context->element.rect);
        dynarray_insert_before(layer->colors, undo_context->element.index, &undo_context->element.color);
        dynarray_insert_before(layer->ids, undo_context->element.index, &undo_context->element.id);
        dynarray_insert_before(layer->actions, undo_context->element.index, &undo_context->element.action);
        layer->selection = -1;
    } break;

    case UNDO_UPDATE: {
        RectLayer *layer = undo_context->element.layer;
        dynarray_replace_at(layer->rects, undo_context->element.index, &undo_context->element.rect);
        dynarray_replace_at(layer->colors, undo_context->element.index, &undo_context->element.color);
        dynarray_replace_at(layer->ids, undo_context->element.index, &undo_context->element.id);
        dynarray_replace_at(layer->actions, undo_context->element.index, &undo_context->element.action);
    } break;

    case UNDO_SWAP: {
        RectLayer *layer = undo_context->element.layer;
        dynarray_swap(layer->rects, undo_context->swap.index1, undo_context->swap.index2);
        dynarray_swap(layer->colors, undo_context->swap.index1, undo_context->swap.index2);
        dynarray_swap(layer->ids, undo_context->swap.index1, undo_context->swap.index2);
        dynarray_swap(layer->actions, undo_context->swap.index1, undo_context->swap.index2);
    } break;
    }
}

#define UNDO_PUSH(HISTORY, CONTEXT)                                     \
    do {                                                                \
        UndoContext context = (CONTEXT);                                \
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

    char id[ENTITY_MAX_ID_SIZE];
    snprintf(id, ENTITY_MAX_ID_SIZE, "%s_%d",
             layer->id_name_prefix,
             layer->id_name_counter++);
    if (dynarray_push(layer->ids, id)) {
        return -1;
    }

    dynarray_push_empty(layer->actions);

    UNDO_PUSH(
        undo_history,
        create_undo_add_context(
            layer,
            dynarray_count(layer->rects) - 1));

    return 0;
}

static int rect_layer_rect_at(RectLayer *layer, Vec2f position)
{
    trace_assert(layer);

    int n = (int) dynarray_count(layer->rects);
    Rect *rects = dynarray_data(layer->rects);

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
    trace_assert(a < dynarray_count(layer->rects));
    trace_assert(b < dynarray_count(layer->rects));

    dynarray_swap(layer->rects, a, b);
    dynarray_swap(layer->colors, a, b);
    dynarray_swap(layer->ids, a, b);
    dynarray_swap(layer->actions, a, b);

    UNDO_PUSH(undo_history, create_undo_swap_context(layer, a, b));
}

static int rect_layer_delete_rect_at(RectLayer *layer,
                                     size_t i,
                                     UndoHistory *undo_history)
{
    trace_assert(layer);

    UNDO_PUSH(undo_history, create_undo_delete_context(layer));

    dynarray_delete_at(layer->rects, i);
    dynarray_delete_at(layer->colors, i);
    dynarray_delete_at(layer->ids, i);
    dynarray_delete_at(layer->actions, i);

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

    Rect *rects = dynarray_data(layer->rects);

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            Vec2f position = camera_map_screen(
                camera,
                event->button.x,
                event->button.y);
            int rect_at_position =
                rect_layer_rect_at(layer, position);

            Color *colors = dynarray_data(layer->colors);

            if (layer->selection >= 0 &&
                layer->selection == rect_at_position &&
                (layer->resize_mask = calc_resize_mask(
                    vec((float) event->button.x, (float)event->button.y),
                    camera_rect(camera, rects[layer->selection])))) {
                layer->state = RECT_LAYER_RESIZE;
                dynarray_copy_to(layer->rects, &layer->inter_rect, (size_t) layer->selection);
            } else if (rect_at_position >= 0) {
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
                && ((size_t)(layer->selection + 1) < dynarray_count(layer->rects))) {
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
                && ((size_t) layer->selection < dynarray_count(layer->rects))) {
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
                    ids + layer->selection * ENTITY_MAX_ID_SIZE);
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
                Vec2f position = camera_map_screen(camera, x, y);

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

static inline
int segment_overlap(Vec2f a, Vec2f b)
{
    trace_assert(a.x <= a.y);
    trace_assert(b.x <= b.y);
    return a.y >= b.x && b.y >= a.x;
}

static
void f(float *src, float dest,
       float src_shift, float dest_shift,
       float snap_threshold)
{
    if (fabsf((*src + src_shift) - (dest + dest_shift)) < snap_threshold)
        *src = dest + dest_shift - src_shift;
}

static
void temp_snap(float *src, float dest,
               float src_shift, float dest_shift,
               float snap_threshold)
{
    f(src, dest, src_shift,          0, snap_threshold);
    f(src, dest, src_shift, dest_shift, snap_threshold);
}

static
void snap_var(float *ys, float yd, float hs, float hd, float st)
{
    temp_snap(ys, yd,  0, hd, st);
    temp_snap(ys, yd, hs, hd, st);
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

    Rect *rects = dynarray_data(layer->rects);

    switch (event->type) {
    case SDL_MOUSEMOTION: {
        Vec2f position = camera_map_screen(
            camera,
            event->button.x,
            event->button.y);

        switch (layer->resize_mask) {
        case 1: {               // TOP
            float y = position.y;
            float x = rects[layer->selection].x;
            float w = rects[layer->selection].w;
            for (size_t i = 0; i < dynarray_count(layer->rects); ++i) {
                if (i == (size_t) layer->selection) continue;

                const Rect b = rects[i];
                if (segment_overlap(vec(x, x + w), vec(b.x, b.x + b.w))) {
                    temp_snap(&y, b.y, 0, b.h, SNAPPING_THRESHOLD);
                }
            }

            layer->inter_rect = rect_from_points(
                vec(x, y),
                rect_position2(rects[layer->selection]));
        } break;

        case 2: {               // LEFT
            float y = rects[layer->selection].y;
            float x = position.x;
            float h = rects[layer->selection].h;
            for (size_t i = 0; i < dynarray_count(layer->rects); ++i) {
                if (i == (size_t) layer->selection) continue;

                const Rect b = rects[i];
                if (segment_overlap(vec(y, y + h), vec(b.y, b.y + b.h))) {
                    temp_snap(&x, b.x, 0, b.w, SNAPPING_THRESHOLD);
                }
            }

            layer->inter_rect = rect_from_points(
                vec(x, y),
                rect_position2(rects[layer->selection]));
        } break;

        case 3: {               // TOP,LEFT
            float x = position.x;
            float y = position.y;
            float w = rects[layer->selection].w;
            float h = rects[layer->selection].h;
            for (size_t i = 0; i < dynarray_count(layer->rects); ++i) {
                if (i == (size_t) layer->selection) continue;

                const Rect b = rects[i];
                if (segment_overlap(vec(y, y + h), vec(b.y, b.y + b.h))) {
                    temp_snap(&x, b.x, 0, b.w, SNAPPING_THRESHOLD);
                }

                if (segment_overlap(vec(x, x + w), vec(b.x, b.x + b.w))) {
                    temp_snap(&y, b.y, 0, b.h, SNAPPING_THRESHOLD);
                }
            }

            layer->inter_rect = rect_from_points(
                vec(x, y),
                rect_position2(rects[layer->selection]));
        } break;

        case 4: {               // BOTTOM
            float y = position.y;
            float x = rects[layer->selection].x;
            float w = rects[layer->selection].w;
            for (size_t i = 0; i < dynarray_count(layer->rects); ++i) {
                if (i == (size_t) layer->selection) continue;

                const Rect b = rects[i];
                if (segment_overlap(vec(x, x + w), vec(b.x, b.x + b.w))) {
                    temp_snap(&y, b.y, 0, b.h, SNAPPING_THRESHOLD);
                }
            }

            layer->inter_rect = rect_from_points(
                rect_position(rects[layer->selection]),
                vec(rects[layer->selection].x + rects[layer->selection].w,
                    y));
        } break;

        case 6: {               // BOTTOM,LEFT
            float x = position.x;
            float y = position.y;
            float w = rects[layer->selection].w;
            float h = rects[layer->selection].h;
            for (size_t i = 0; i < dynarray_count(layer->rects); ++i) {
                if (i == (size_t) layer->selection) continue;

                const Rect b = rects[i];
                if (segment_overlap(vec(y, y + h), vec(b.y, b.y + b.h))) {
                    temp_snap(&x, b.x, 0, b.w, SNAPPING_THRESHOLD);
                }

                if (segment_overlap(vec(x, x + w), vec(b.x, b.x + b.w))) {
                    temp_snap(&y, b.y, 0, b.h, SNAPPING_THRESHOLD);
                }
            }

            layer->inter_rect = rect_from_points(
                vec(x, rects[layer->selection].y),
                vec(rects[layer->selection].x + rects[layer->selection].w,
                    y));
        } break;

        case 8: {               // RIGHT
            float y = rects[layer->selection].y;
            float x = position.x;
            float h = rects[layer->selection].h;
            for (size_t i = 0; i < dynarray_count(layer->rects); ++i) {
                if (i == (size_t) layer->selection) continue;

                const Rect b = rects[i];
                if (segment_overlap(vec(y, y + h), vec(b.y, b.y + b.h))) {
                    temp_snap(&x, b.x, 0, b.w, SNAPPING_THRESHOLD);
                }
            }

            layer->inter_rect = rect_from_points(
                rect_position(rects[layer->selection]),
                vec(x,
                    rects[layer->selection].y + rects[layer->selection].h));
        } break;

        case 9: {               // TOP,RIGHT
            float x = position.x;
            float y = position.y;
            float w = rects[layer->selection].w;
            float h = rects[layer->selection].h;
            for (size_t i = 0; i < dynarray_count(layer->rects); ++i) {
                if (i == (size_t) layer->selection) continue;

                const Rect b = rects[i];
                if (segment_overlap(vec(y, y + h), vec(b.y, b.y + b.h))) {
                    temp_snap(&x, b.x, 0, b.w, SNAPPING_THRESHOLD);
                }

                if (segment_overlap(vec(x, x + w), vec(b.x, b.x + b.w))) {
                    temp_snap(&y, b.y, 0, b.h, SNAPPING_THRESHOLD);
                }
            }

            layer->inter_rect = rect_from_points(
                vec(rects[layer->selection].x, y),
                vec(x,
                    rects[layer->selection].y + rects[layer->selection].h));
        } break;

        case 12: {              // BOTTOM,RIGHT
            float x = position.x;
            float y = position.y;
            float w = rects[layer->selection].w;
            float h = rects[layer->selection].h;
            for (size_t i = 0; i < dynarray_count(layer->rects); ++i) {
                if (i == (size_t) layer->selection) continue;

                const Rect b = rects[i];
                if (segment_overlap(vec(y, y + h), vec(b.y, b.y + b.h))) {
                    temp_snap(&x, b.x, 0, b.w, SNAPPING_THRESHOLD);
                }

                if (segment_overlap(vec(x, x + w), vec(b.x, b.x + b.w))) {
                    temp_snap(&y, b.y, 0, b.h, SNAPPING_THRESHOLD);
                }
            }

            layer->inter_rect = rect_from_points(
                rect_position(rects[layer->selection]),
                vec(x, y));
        } break;
        }
    } break;

    case SDL_MOUSEBUTTONUP: {
        layer->state = RECT_LAYER_IDLE;
        UNDO_PUSH(undo_history, create_undo_update_context(layer));
        dynarray_replace_at(layer->rects, (size_t) layer->selection, &layer->inter_rect);
    } break;
    }

    return 0;
}

static
void snap_rects(size_t ignore_index, Rect *a,
                Rect *rects, size_t rects_size,
                float snapping_threshold)
{
    trace_assert(rects);
    trace_assert(a);

    for (size_t i = 0; i < rects_size; ++i) {
        if (i == ignore_index) continue;

        const Rect b = rects[i];

        if (segment_overlap(vec(a->x, a->x + a->w), vec(b.x,  b.x  + b.w))) {
            snap_var(&a->y, b.y, a->h, b.h, snapping_threshold);
        }

        if (segment_overlap(vec(a->y, a->y + a->h), vec(b.y,  b.y  + b.h))) {
            snap_var(&a->x, b.x, a->w, b.w, snapping_threshold);
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

    Rect *rects = dynarray_data(layer->rects);

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

        // TODO(#1141): Rect Snapping in Level Editor should be optional
        // TODO(#1142): Resize mode of Rect Layer does not support Snapping
        snap_rects((size_t) layer->selection, &layer->inter_rect,
                   rects, dynarray_count(layer->rects),
                   SNAPPING_THRESHOLD / camera->scale);
    } break;

    case SDL_MOUSEBUTTONUP: {
        layer->state = RECT_LAYER_IDLE;

        float distance = vec_length(
            vec_sub(rect_position(layer->inter_rect),
                    rect_position(rects[layer->selection])));

        if (distance > 1e-6) {
            UNDO_PUSH(undo_history, create_undo_update_context(layer));
            dynarray_replace_at(layer->rects, (size_t) layer->selection, &layer->inter_rect);
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
            UNDO_PUSH(undo_history, create_undo_update_context(layer));

            char *id = dynarray_pointer_at(layer->ids, (size_t)layer->selection);
            memset(id, 0, ENTITY_MAX_ID_SIZE);
            memcpy(id, edit_field_as_text(layer->id_edit_field), ENTITY_MAX_ID_SIZE - 1);
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

RectLayer *create_rect_layer(const char *id_name_prefix, Cursor *cursor)
{
    trace_assert(cursor);

    Lt *lt = create_lt();

    RectLayer *layer = PUSH_LT(lt, nth_calloc(1, sizeof(RectLayer)), free);
    if (layer == NULL) {
        RETURN_LT(lt, NULL);
    }
    layer->lt = lt;

    layer->ids = PUSH_LT(
        lt,
        create_dynarray(sizeof(char) * ENTITY_MAX_ID_SIZE),
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

    layer->actions = PUSH_LT(
        lt,
        create_dynarray(sizeof(Action)),
        destroy_dynarray);
    if (layer->actions == NULL) {
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

    layer->grid =
        PUSH_LT(
            lt,
            nth_calloc(
                1,
                sizeof(Grid) + sizeof(Widget*) * RECT_LAYER_GRID_ROWS * RECT_LAYER_GRID_COLUMNS),
            free);
    if (layer->grid == NULL) {
        RETURN_LT(lt, NULL);
    }
    layer->grid->rows = RECT_LAYER_GRID_ROWS;
    layer->grid->columns = RECT_LAYER_GRID_COLUMNS;
    grid_put_widget(layer->grid, &layer->action_picker.widget, 0, RECT_LAYER_GRID_COLUMNS - 1);

    layer->color_picker = create_color_picker_from_rgba(rgba(1.0f, 0.0f, 0.0f, 1.0f));
    layer->selection = -1;
    layer->id_name_prefix = id_name_prefix;
    layer->cursor = cursor;

    return layer;
}

RectLayer *create_rect_layer_from_line_stream(LineStream *line_stream,
                                              const char *id_name_prefix,
                                              Cursor *cursor)
{
    trace_assert(line_stream);

    RectLayer *layer = create_rect_layer(id_name_prefix, cursor);
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
        char id[ENTITY_MAX_ID_SIZE];

        int n = 0;
        if (sscanf(line,
                   "%"STRINGIFY(ENTITY_MAX_ID_SIZE)"s%f%f%f%f%6s%n",
                   id,
                   &rect.x, &rect.y,
                   &rect.w, &rect.h,
                   hex, &n) <= 0) {
            log_fail("%s\n", strerror(errno));
            RETURN_LT(layer->lt, NULL);
        }
        line += n;

        Color color = hexstr(hex);
        dynarray_push(layer->rects, &rect);
        dynarray_push(layer->ids, id);
        dynarray_push(layer->colors, &color);

        Action action = {
            .type = ACTION_NONE,
            .entity_id = {0}
        };

        if (sscanf(line, "%d%n", (int*)&action.type, &n) > 0) {
            line += n;
            switch (action.type) {
            case ACTION_NONE: break;

            case ACTION_TOGGLE_GOAL:
            case ACTION_HIDE_LABEL: {
                if (sscanf(line, "%"STRINGIFY(ENTITY_MAX_ID_SIZE)"s", action.entity_id) <= 0) {
                    log_fail("%s\n", strerror(errno));
                    RETURN_LT(layer->lt, NULL);
                }
            } break;

            case ACTION_N: break;
            }
        }

        dynarray_push(layer->actions, &action);
    }

    return layer;
}

void destroy_rect_layer(RectLayer *layer)
{
    trace_assert(layer);
    RETURN_LT0(layer->lt);
}

int rect_layer_render(const RectLayer *layer, const Camera *camera, int active)
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
                    layer->id_edit_field,
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
            UNDO_PUSH(undo_history, create_undo_update_context(layer));
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

    switch (event->type) {
    case SDL_WINDOWEVENT: {
        switch (event->window.event) {
        case SDL_WINDOWEVENT_RESIZED: {
            grid_relayout(layer->grid, rect(0.0f, 0.0f,
                                            (float) event->window.data1,
                                            (float) event->window.data2));
        } break;
        }
    } break;
    }

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
    Action *actions = dynarray_data(layer->actions);

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
    return dynarray_data(layer->actions);
}
