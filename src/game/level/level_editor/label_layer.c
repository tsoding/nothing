#include <stdio.h>

#include <SDL.h>

#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "system/str.h"
#include "system/log.h"
#include "math/vec.h"
#include "label_layer.h"
#include "dynarray.h"
#include "color.h"
#include "game/camera.h"
#include "color_picker.h"
#include "ui/edit_field.h"
#include "math/extrema.h"
#include "config.h"

#define LABEL_LAYER_SELECTION_THICCNESS 5.0f


static int label_clipboard = 0;
static char label_clipboard_text[LABEL_LAYER_TEXT_MAX_SIZE];
static Color label_clipboard_color;

typedef enum {
    LABEL_UNDO_ADD,
    LABEL_UNDO_DELETE,
    LABEL_UNDO_UPDATE,
    LABEL_UNDO_SWAP
} LabelUndoType;

typedef struct {
    LabelUndoType type;
    LabelLayer *layer;
    char id[LABEL_LAYER_ID_MAX_SIZE];
    Vec2f position;
    Color color;
    char text[LABEL_LAYER_TEXT_MAX_SIZE];
    size_t index;
    size_t index2;
} LabelUndoContext;

static
LabelUndoContext create_label_undo_swap_context(LabelLayer *label_layer,
                                     size_t index, size_t index2)
{
    trace_assert(label_layer);
    trace_assert(index < label_layer->positions.count);
    trace_assert(index2 < label_layer->positions.count);

    LabelUndoContext undo_context;
    undo_context.type = LABEL_UNDO_SWAP;
    undo_context.layer = label_layer;
    undo_context.index = index;
    undo_context.index2 = index2;
    return undo_context;
}

static
LabelUndoContext create_label_undo_context(LabelLayer *label_layer, LabelUndoType type)
{
    trace_assert(label_layer);
    trace_assert(type != LABEL_UNDO_SWAP);

    LabelUndoContext undo_context;

    size_t index = type == LABEL_UNDO_ADD
        ? label_layer->positions.count - 1
        : (size_t)label_layer->selection;

    undo_context.type = type;
    undo_context.layer = label_layer;
    dynarray_copy_to(&label_layer->ids, &undo_context.id, index);
    dynarray_copy_to(&label_layer->positions, &undo_context.position, index);
    dynarray_copy_to(&label_layer->colors, &undo_context.color, index);
    dynarray_copy_to(&label_layer->texts, &undo_context.text, index);
    undo_context.index = index;

    return undo_context;
}

static
void label_layer_undo(void *context, size_t context_size)
{
    trace_assert(context);
    trace_assert(sizeof(LabelUndoContext) == context_size);

    LabelUndoContext *undo_context = context;
    LabelLayer *label_layer = undo_context->layer;

    switch (undo_context->type) {
    case LABEL_UNDO_ADD: {
        dynarray_delete_at(&label_layer->ids, undo_context->index);
        dynarray_delete_at(&label_layer->positions, undo_context->index);
        dynarray_delete_at(&label_layer->colors, undo_context->index);
        dynarray_delete_at(&label_layer->texts, undo_context->index);
    } break;

    case LABEL_UNDO_DELETE: {
        dynarray_insert_before(&label_layer->ids, undo_context->index, &undo_context->id);
        dynarray_insert_before(&label_layer->positions, undo_context->index, &undo_context->position);
        dynarray_insert_before(&label_layer->colors, undo_context->index, &undo_context->color);
        dynarray_insert_before(&label_layer->texts, undo_context->index, &undo_context->text);
    } break;

    case LABEL_UNDO_UPDATE: {
        dynarray_replace_at(&label_layer->ids, undo_context->index, &undo_context->id);
        dynarray_replace_at(&label_layer->positions, undo_context->index, &undo_context->position);
        dynarray_replace_at(&label_layer->colors, undo_context->index, &undo_context->color);
        dynarray_replace_at(&label_layer->texts, undo_context->index, &undo_context->text);
    } break;

    case LABEL_UNDO_SWAP: {
        dynarray_swap(&label_layer->ids, undo_context->index, undo_context->index2);
        dynarray_swap(&label_layer->positions, undo_context->index, undo_context->index2);
        dynarray_swap(&label_layer->colors, undo_context->index, undo_context->index2);
        dynarray_swap(&label_layer->texts, undo_context->index, undo_context->index2);
    } break;
    }
}

#define LABEL_UNDO_PUSH(HISTORY, CONTEXT)                                     \
    do {                                                                \
        LabelUndoContext context = (CONTEXT);                                \
        undo_history_push(                                              \
            HISTORY,                                                    \
            label_layer_undo,                                           \
            &context,                                                   \
            sizeof(context));                                           \
    } while(0)


LayerPtr label_layer_as_layer(LabelLayer *label_layer)
{
    LayerPtr layer = {
        .ptr = label_layer,
        .type = LAYER_LABEL
    };
    return layer;
}

LabelLayer *create_label_layer(Memory *memory,
                               const char *id_name_prefix)
{
    trace_assert(memory);
    trace_assert(id_name_prefix);

    LabelLayer *result = memory_alloc(memory, sizeof(LabelLayer));
    memset(result, 0, sizeof(LabelLayer));
    result->ids = create_dynarray(memory, sizeof(char) * LABEL_LAYER_ID_MAX_SIZE);
    result->positions = create_dynarray(memory, sizeof(Vec2f));
    result->colors = create_dynarray(memory, sizeof(Color));
    result->texts = create_dynarray(memory, sizeof(char) * LABEL_LAYER_TEXT_MAX_SIZE);
    result->color_picker = create_color_picker_from_rgba(COLOR_RED);
    result->selection = -1;
    result->edit_field.font_size = LABELS_SIZE;
    result->edit_field.font_color = COLOR_RED;
    result->id_name_prefix = id_name_prefix;
    return result;
}

void label_layer_load(LabelLayer *label_layer,
                        Memory *memory,
                        String *input)
{
    trace_assert(label_layer);
    trace_assert(memory);
    trace_assert(input);

    int n = atoi(string_to_cstr(memory, trim(chop_by_delim(input, '\n'))));
    char id[LABEL_LAYER_ID_MAX_SIZE];
    char label_text[LABEL_LAYER_TEXT_MAX_SIZE];
    for (int i = 0; i < n; ++i) {
        String meta = trim(chop_by_delim(input, '\n'));

        String string_id = trim(chop_word(&meta));
        Vec2f position;
        position.x = strtof(string_to_cstr(memory, trim(chop_word(&meta))), NULL);
        position.y = strtof(string_to_cstr(memory, trim(chop_word(&meta))), NULL);
        Color color = hexs(trim(chop_word(&meta)));

        memset(id, 0, LABEL_LAYER_ID_MAX_SIZE);
        memcpy(
            id,
            string_id.data,
            min_size_t(LABEL_LAYER_ID_MAX_SIZE - 1, string_id.count));

        String label_text_string =
            trim(chop_by_delim(input, '\n'));
        memset(label_text, 0, LABEL_LAYER_TEXT_MAX_SIZE);
        memcpy(
            label_text,
            label_text_string.data,
            min_size_t(LABEL_LAYER_TEXT_MAX_SIZE - 1,
                       label_text_string.count));

        dynarray_push(&label_layer->ids, id);
        dynarray_push(&label_layer->positions, &position);
        dynarray_push(&label_layer->colors, &color);
        dynarray_push(&label_layer->texts, label_text);
    }
}

static inline
Rect boundary_of_element(const LabelLayer *label_layer,
                         size_t i,
                         Vec2f position)
{
    trace_assert(i < label_layer->texts.count);

    char *ids = (char *)label_layer->ids.data;
    char *texts = (char *)label_layer->texts.data;

    return rect_boundary2(
        sprite_font_boundary_box(
            position,
            LABELS_SIZE,
            texts + i * LABEL_LAYER_TEXT_MAX_SIZE),
        sprite_font_boundary_box(
            vec_sum(
                position,
                vec_mult(
                    vec(0.0f, FONT_CHAR_HEIGHT),
                    LABELS_SIZE)),
            vec(1.0f, 1.0f),
            ids + i * LABEL_LAYER_ID_MAX_SIZE));
}

int label_layer_render(const LabelLayer *label_layer,
                       const Camera *camera,
                       int active)
{
    trace_assert(label_layer);
    trace_assert(camera);

    if (active && color_picker_render(&label_layer->color_picker, camera) < 0) {
        return -1;
    }

    size_t n = label_layer->ids.count;
    char *ids = (char *)label_layer->ids.data;
    Vec2f *positions = (Vec2f *)label_layer->positions.data;
    Color *colors = (Color *)label_layer->colors.data;
    char *texts = (char *)label_layer->texts.data;

    /* TODO(#891): LabelLayer doesn't show the final position of Label after the animation */
    for (size_t i = 0; i < n; ++i) {
        const Color color = label_layer->state == LABEL_LAYER_RECOLOR && label_layer->selection == (int) i
            ? label_layer->inter_color
            : colors[i];

        const Vec2f position =
            label_layer->state == LABEL_LAYER_MOVE && label_layer->selection == (int) i
            ? label_layer->inter_position
            : positions[i];

        // Label Text
        if (label_layer->state == LABEL_LAYER_EDIT_TEXT && label_layer->selection == (int) i) {
            if (edit_field_render_world(
                    &label_layer->edit_field,
                    camera,
                    position) < 0) {
                return -1;
            }
        } else {
            if (camera_render_text(
                    camera,
                    texts + i * LABEL_LAYER_TEXT_MAX_SIZE,
                    LABELS_SIZE,
                    color_scale(
                        color,
                        rgba(1.0f, 1.0f, 1.0f, active ? 1.0f : 0.5f)),
                    position) < 0) {
                return -1;
            }
        }

        // Label ID
        if (label_layer->state == LABEL_LAYER_EDIT_ID && label_layer->selection == (int)i) {
            if (edit_field_render_world(
                    &label_layer->edit_field,
                    camera,
                    vec_sum(
                        position,
                        vec_mult(
                            vec(0.0f, FONT_CHAR_HEIGHT),
                            LABELS_SIZE))) < 0) {
                return -1;
            }
        } else {
            if (camera_render_text(
                    camera,
                    ids + i * LABEL_LAYER_ID_MAX_SIZE,
                    vec(1.0f, 1.0f),
                    color_scale(
                        color_invert(color),
                        rgba(1.0f, 1.0f, 1.0f, active ? 1.0f : 0.5f)),
                    vec_sum(
                        position,
                        vec_mult(
                            vec(0.0f, FONT_CHAR_HEIGHT),
                            LABELS_SIZE))) < 0) {
                return -1;
            }
        }

        // Label Selection
        if (active && label_layer->selection == (int) i) {
            Rect selection =
                rect_pad(
                    camera_rect(
                        camera,
                        boundary_of_element(
                            label_layer,
                            i,
                            position)),
                    LABEL_LAYER_SELECTION_THICCNESS * 0.5f);


            if (camera_draw_thicc_rect_screen(
                    camera,
                    selection,
                    color,
                    LABEL_LAYER_SELECTION_THICCNESS) < 0) {
                return -1;
            }
        }
    }


    return 0;
}

static
int label_layer_element_at(LabelLayer *label_layer,
                           Vec2f position)
{
    trace_assert(label_layer);

    Vec2f *positions = (Vec2f*)label_layer->positions.data;

    const int n = (int) label_layer->texts.count;
    for (int i = n - 1; i >= 0; --i) {
        if (rect_contains_point(
                boundary_of_element(
                    label_layer,
                    (size_t) i,
                    positions[i]),
                position)) {
            return i;
        }
    }

    return -1;
}

static
void label_layer_delete_selected_label(LabelLayer *label_layer,
                                       UndoHistory *undo_history)
{
    trace_assert(label_layer);
    trace_assert(label_layer->selection >= 0);

    LABEL_UNDO_PUSH(undo_history, create_label_undo_context(label_layer, LABEL_UNDO_DELETE));

    dynarray_delete_at(&label_layer->ids, (size_t)label_layer->selection);
    dynarray_delete_at(&label_layer->positions, (size_t)label_layer->selection);
    dynarray_delete_at(&label_layer->colors, (size_t)label_layer->selection);
    dynarray_delete_at(&label_layer->texts, (size_t)label_layer->selection);

    label_layer->selection = -1;
}

static
int label_layer_add_label(LabelLayer *label_layer,
                          Vec2f position,
                          Color color,
                          const char *text,
                          UndoHistory *undo_history)
{
    trace_assert(label_layer);

    // TODO(#982): id generation code is duplicated in label_layer, point_layer and rect_layer
    char id[LABEL_LAYER_ID_MAX_SIZE];
    snprintf(id, LABEL_LAYER_ID_MAX_SIZE, "%s_%d",
             label_layer->id_name_prefix,
             label_layer->id_name_counter++);

    size_t n = label_layer->ids.count;

    dynarray_push(&label_layer->ids, id);
    dynarray_push(&label_layer->positions, &position);
    dynarray_push(&label_layer->colors, &color);
    dynarray_push_empty(&label_layer->texts);
    memcpy(
        dynarray_pointer_at(&label_layer->texts, n),
        text,
        min_size_t(LABEL_LAYER_ID_MAX_SIZE - 1, strlen(text)));

    LABEL_UNDO_PUSH(undo_history, create_label_undo_context(label_layer, LABEL_UNDO_ADD));

    return (int) n;
}

static
void label_layer_swap_elements(LabelLayer *label_layer,
                               size_t a, size_t b,
                               UndoHistory *undo_history)
{
    trace_assert(label_layer);
    trace_assert(undo_history);
    trace_assert(a < label_layer->positions.count);
    trace_assert(b < label_layer->positions.count);

    dynarray_swap(&label_layer->ids, a, b);
    dynarray_swap(&label_layer->positions, a, b);
    dynarray_swap(&label_layer->colors, a, b);
    dynarray_swap(&label_layer->texts, a, b);

    LABEL_UNDO_PUSH(undo_history, create_label_undo_swap_context(label_layer, a, b));
}

static
int label_layer_idle_event(LabelLayer *label_layer,
                           const SDL_Event *event,
                           const Camera *camera,
                           UndoHistory *undo_history)
{
    trace_assert(label_layer);
    trace_assert(event);
    trace_assert(camera);


    int changed = 0;
    if (color_picker_event(
            &label_layer->color_picker,
            event,
            camera,
            &changed) < 0) {
        return -1;
    }

    if (changed) {
        if (label_layer->selection >= 0) {
            label_layer->state = LABEL_LAYER_RECOLOR;
            label_layer->inter_color = color_picker_rgba(&label_layer->color_picker);
        }
        return 0;
    }

    Color *colors = (Color*)label_layer->colors.data;
    Vec2f *positions = (Vec2f*)label_layer->positions.data;
    char *ids = (char*)label_layer->ids.data;
    char *texts = (char*)label_layer->texts.data;

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            const Vec2f position = camera_map_screen(
                camera,
                event->button.x,
                event->button.y);

            const int element = label_layer_element_at(
                label_layer,
                position);

            if (element >= 0) {
                label_layer->move_anchor = vec_sub(position, positions[element]);
                label_layer->selection = element;
                label_layer->state = LABEL_LAYER_MOVE;
                label_layer->inter_position = positions[element];

                label_layer->color_picker =
                    create_color_picker_from_rgba(colors[element]);
            } else {
                label_layer->selection = label_layer_add_label(
                    label_layer,
                    position,
                    color_picker_rgba(
                        &label_layer->color_picker),
                    "",
                    undo_history);
                label_layer->state = LABEL_LAYER_EDIT_TEXT;
                edit_field_replace(
                    &label_layer->edit_field,
                    texts + label_layer->selection * LABEL_LAYER_TEXT_MAX_SIZE);
                edit_field_restyle(
                    &label_layer->edit_field,
                    LABELS_SIZE,
                    colors[label_layer->selection]);
                SDL_StartTextInput();
            }
        } break;
        }
    } break;

    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_UP: {
            if ((event->key.keysym.mod & KMOD_SHIFT)
                && (label_layer->selection >= 0)
                && ((size_t)(label_layer->selection + 1) < label_layer->positions.count)) {
                label_layer_swap_elements(
                    label_layer,
                    (size_t) label_layer->selection,
                    (size_t) label_layer->selection + 1,
                    undo_history);
                label_layer->selection++;
            }
        } break;

        case SDLK_DOWN: {
            if ((event->key.keysym.mod & KMOD_SHIFT)
                && (label_layer->selection > 0)
                && ((size_t) label_layer->selection < label_layer->positions.count)) {
                label_layer_swap_elements(
                    label_layer,
                    (size_t) label_layer->selection,
                    (size_t) label_layer->selection - 1,
                    undo_history);
                label_layer->selection--;
            }
        } break;

        case SDLK_F2: {
            if (label_layer->selection >= 0) {
                label_layer->state = LABEL_LAYER_EDIT_TEXT;
                edit_field_replace(
                    &label_layer->edit_field,
                    texts + label_layer->selection * LABEL_LAYER_TEXT_MAX_SIZE);
                edit_field_restyle(
                    &label_layer->edit_field,
                    LABELS_SIZE,
                    colors[label_layer->selection]);
                SDL_StartTextInput();
            }
        } break;

        case SDLK_F3: {
            if (label_layer->selection >= 0) {
                label_layer->state = LABEL_LAYER_EDIT_ID;
                edit_field_replace(
                    &label_layer->edit_field,
                    ids + label_layer->selection * LABEL_LAYER_ID_MAX_SIZE);
                edit_field_restyle(
                    &label_layer->edit_field,
                    vec(1.0f, 1.0f),
                    color_invert(colors[label_layer->selection]));
                SDL_StartTextInput();
            }
        } break;

        case SDLK_DELETE: {
            if (label_layer->selection >= 0) {
                label_layer_delete_selected_label(
                    label_layer,
                    undo_history);
                label_layer->selection = -1;
            }
        } break;

        case SDLK_c: {
            if ((event->key.keysym.mod & KMOD_LCTRL) && label_layer->selection >= 0) {
                label_clipboard = 1;
                dynarray_copy_to(&label_layer->texts, label_clipboard_text, (size_t)label_layer->selection);
                dynarray_copy_to(&label_layer->colors, &label_clipboard_color, (size_t)label_layer->selection);
            }
        } break;

        case SDLK_v: {
            if ((event->key.keysym.mod & KMOD_LCTRL) && label_clipboard) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                Vec2f position = camera_map_screen(camera, x, y);

                label_layer_add_label(
                    label_layer,
                    position,
                    label_clipboard_color,
                    label_clipboard_text,
                    undo_history);
            }
        } break;
        }
    } break;
    }

    return 0;
}

static
void snap_inter_position(LabelLayer *label_layer, float snap_threshold)
{
    trace_assert(label_layer);
    trace_assert(label_layer->selection >= 0);
    trace_assert(label_layer->state == LABEL_LAYER_MOVE);

    const size_t n = label_layer->positions.count;
    Vec2f *positions = (Vec2f*)label_layer->positions.data;

    Rect a = boundary_of_element(
        label_layer,
        (size_t) label_layer->selection,
        label_layer->inter_position);

    for (size_t i = 0; i < n; ++i) {
        if (i == (size_t) label_layer->selection) continue;

        const Rect b = boundary_of_element(label_layer, i, positions[i]);

        if (segment_overlap(vec(a.x, a.x + a.w), vec(b.x,  b.x + b.w))) {
            snap_seg2seg(&label_layer->inter_position.y,
                         b.y, a.h, b.h, snap_threshold);
        }

        if (segment_overlap(vec(a.y, a.y + a.h), vec(b.y,  b.y  + b.h))) {
            snap_seg2seg(&label_layer->inter_position.x,
                         b.x, a.w, b.w, snap_threshold);
        }
    }
}

static
int label_layer_move_event(LabelLayer *label_layer,
                           const SDL_Event *event,
                           const Camera *camera,
                           UndoHistory *undo_history)
{
    trace_assert(label_layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(label_layer->selection >= 0);

    Vec2f *positions = (Vec2f*)label_layer->positions.data;

    switch (event->type) {
    case SDL_MOUSEMOTION: {
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        const Vec2f mouse_pos = vec_sub(
            camera_map_screen(
                camera,
                event->motion.x,
                event->motion.y),
            label_layer->move_anchor);

        if (!(state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL])) {
            label_layer->inter_position = mouse_pos;
        } else {
            const Vec2f label_pos = positions[label_layer->selection];

            const float dx = fabsf(label_pos.x - mouse_pos.x);
            const float dy = fabsf(label_pos.y - mouse_pos.y);

            if (dx > dy) {
                label_layer->inter_position = vec(mouse_pos.x, label_pos.y);
            } else {
                label_layer->inter_position = vec(label_pos.x, mouse_pos.y);
            }
        }

        snap_inter_position(label_layer, SNAPPING_THRESHOLD);
    } break;

    case SDL_MOUSEBUTTONUP: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            const float distance = vec_length(
                vec_sub(label_layer->inter_position,
                        positions[label_layer->selection]));

            if (distance > 1e-6) {
                LABEL_UNDO_PUSH(undo_history, create_label_undo_context(label_layer, LABEL_UNDO_UPDATE));

                dynarray_replace_at(
                    &label_layer->positions,
                    (size_t)label_layer->selection,
                    &label_layer->inter_position);
            }

            label_layer->state = LABEL_LAYER_IDLE;
        } break;
        }
    } break;
    }

    return 0;
}

static
int label_layer_edit_text_event(LabelLayer *label_layer,
                                const SDL_Event *event,
                                const Camera *camera,
                                UndoHistory *undo_history)
{
    trace_assert(label_layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(label_layer->selection >= 0);

    switch (event->type) {
    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_RETURN: {
            LABEL_UNDO_PUSH(undo_history, create_label_undo_context(label_layer, LABEL_UNDO_UPDATE));

            char *text =
                (char*)label_layer->texts.data + label_layer->selection * LABEL_LAYER_TEXT_MAX_SIZE;
            memset(text, 0, LABEL_LAYER_TEXT_MAX_SIZE);
            memcpy(text, edit_field_as_text(&label_layer->edit_field), LABEL_LAYER_TEXT_MAX_SIZE - 1);
            label_layer->state = LABEL_LAYER_IDLE;
            SDL_StopTextInput();
            return 0;
        } break;

        case SDLK_ESCAPE: {
            label_layer->state = LABEL_LAYER_IDLE;
            SDL_StopTextInput();
            return 0;
        } break;
        }
    } break;
    }

    return edit_field_event(&label_layer->edit_field, event);
}

static
int label_layer_edit_id_event(LabelLayer *label_layer,
                              const SDL_Event *event,
                              const Camera *camera,
                              UndoHistory *undo_history)
{
    trace_assert(label_layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(undo_history);
    trace_assert(label_layer->selection >= 0);

    switch (event->type) {
    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_RETURN: {
            LABEL_UNDO_PUSH(undo_history, create_label_undo_context(label_layer, LABEL_UNDO_UPDATE));

            char *id =
                (char*)label_layer->ids.data + label_layer->selection * LABEL_LAYER_ID_MAX_SIZE;
            memset(id, 0, LABEL_LAYER_ID_MAX_SIZE);
            memcpy(id, edit_field_as_text(&label_layer->edit_field), LABEL_LAYER_ID_MAX_SIZE - 1);
            label_layer->state = LABEL_LAYER_IDLE;
            SDL_StopTextInput();
            return 0;
        } break;

        case SDLK_ESCAPE: {
            label_layer->state = LABEL_LAYER_IDLE;
            SDL_StopTextInput();
            return 0;
        } break;
        }
    } break;
    }

    return edit_field_event(&label_layer->edit_field, event);
}

static
int label_layer_recolor_event(LabelLayer *label_layer,
                              const SDL_Event *event,
                              const Camera *camera,
                              UndoHistory *undo_history)
{
    trace_assert(label_layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(undo_history);
    trace_assert(label_layer->selection >= 0);

    int changed = 0;

    if (color_picker_event(
            &label_layer->color_picker,
            event,
            camera,
            &changed) < 0) {
        return -1;
    }

    if (changed) {
        label_layer->inter_color =
            color_picker_rgba(&label_layer->color_picker);

        if (!color_picker_drag(&label_layer->color_picker)) {
            LABEL_UNDO_PUSH(undo_history, create_label_undo_context(label_layer, LABEL_UNDO_UPDATE));

            dynarray_replace_at(
                &label_layer->colors,
                (size_t) label_layer->selection,
                &label_layer->inter_color);
            label_layer->state = LABEL_LAYER_IDLE;
        }
    }

    return 0;
}

int label_layer_event(LabelLayer *label_layer,
                      const SDL_Event *event,
                      const Camera *camera,
                      UndoHistory *undo_history)
{
    trace_assert(label_layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(undo_history);

    switch (label_layer->state) {
    case LABEL_LAYER_IDLE:
        return label_layer_idle_event(label_layer, event, camera, undo_history);

    case LABEL_LAYER_MOVE:
        return label_layer_move_event(label_layer, event, camera, undo_history);

    case LABEL_LAYER_EDIT_TEXT:
        return label_layer_edit_text_event(label_layer, event, camera, undo_history);

    case LABEL_LAYER_EDIT_ID:
        return label_layer_edit_id_event(label_layer, event, camera, undo_history);

    case LABEL_LAYER_RECOLOR:
        return label_layer_recolor_event(label_layer, event, camera, undo_history);
    }

    return 0;
}

size_t label_layer_count(const LabelLayer *label_layer)
{
    return label_layer->ids.count;
}

char *label_layer_ids(const LabelLayer *label_layer)
{
    return (char *)label_layer->ids.data;
}

Vec2f *label_layer_positions(const LabelLayer *label_layer)
{
    return (Vec2f *)label_layer->positions.data;
}

Color *label_layer_colors(const LabelLayer *label_layer)
{
    return (Color *)label_layer->colors.data;
}

char *labels_layer_texts(const LabelLayer *label_layer)
{
    return (char *)label_layer->texts.data;
}

int label_layer_dump_stream(const LabelLayer *label_layer, FILE *filedump)
{
    trace_assert(label_layer);
    trace_assert(filedump);

    size_t n = label_layer->ids.count;
    char *ids = (char *)label_layer->ids.data;
    Vec2f *positions = (Vec2f *)label_layer->positions.data;
    Color *colors = (Color *)label_layer->colors.data;
    char *texts = (char *)label_layer->texts.data;

    fprintf(filedump, "%zd\n", n);
    for (size_t i = 0; i < n; ++i) {
        fprintf(filedump, "%s %f %f ",
                ids + LABEL_LAYER_ID_MAX_SIZE * i,
                positions[i].x, positions[i].y);
        color_hex_to_stream(colors[i], filedump);
        fprintf(filedump, "\n%s\n", texts + i * LABEL_LAYER_TEXT_MAX_SIZE);
    }

    return 0;
}
