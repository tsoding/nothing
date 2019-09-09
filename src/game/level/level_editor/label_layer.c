#include <stdio.h>

#include <SDL.h>

#include "system/line_stream.h"
#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "system/lt.h"
#include "system/str.h"
#include "system/log.h"
#include "math/point.h"
#include "label_layer.h"
#include "dynarray.h"
#include "color.h"
#include "game/camera.h"
#include "color_picker.h"
#include "ui/edit_field.h"
#include "math/extrema.h"

#define LABEL_LAYER_SELECTION_THICCNESS 5.0f

// TODO(#1050): LabelLayer does not support copy-pasting

typedef enum {
    LABEL_LAYER_IDLE = 0,
    LABEL_LAYER_MOVE,
    LABEL_LAYER_EDIT_TEXT,
    LABEL_LAYER_EDIT_ID,
    LABEL_LAYER_RECOLOR
} LabelLayerState;

static int clipboard;
static char clipboard_text[LABEL_LAYER_TEXT_MAX_SIZE];
static Color clipboard_color;

struct LabelLayer {
    Lt *lt;
    LabelLayerState state;
    Dynarray *ids;
    Dynarray *positions;
    Dynarray *colors;
    Dynarray *texts;
    int selected;
    ColorPicker color_picker;
    Point move_anchor;
    Edit_field *edit_field;
    Point inter_position;
    Color inter_color;
};

typedef enum {
    UNDO_ADD,
    UNDO_DELETE,
    UNDO_UPDATE
} UndoType;

typedef struct {
    UndoType type;
    LabelLayer *layer;
    char id[LABEL_LAYER_ID_MAX_SIZE];
    Point position;
    Color color;
    char text[LABEL_LAYER_TEXT_MAX_SIZE];
    size_t index;
} UndoContext;

static
UndoContext create_undo_context(LabelLayer *label_layer, UndoType type)
{
    UndoContext undo_context;

    size_t index = type == UNDO_ADD
        ? dynarray_count(label_layer->positions) - 1
        : (size_t)label_layer->selected;

    undo_context.type = type;
    undo_context.layer = label_layer;
    dynarray_copy_to(label_layer->ids, &undo_context.id, index);
    dynarray_copy_to(label_layer->positions, &undo_context.position, index);
    dynarray_copy_to(label_layer->colors, &undo_context.color, index);
    dynarray_copy_to(label_layer->texts, &undo_context.text, index);
    undo_context.index = index;

    return undo_context;
}

static
void label_layer_undo(void *context, size_t context_size)
{
    trace_assert(context);
    trace_assert(sizeof(UndoContext) == context_size);

    UndoContext *undo_context = context;
    LabelLayer *label_layer = undo_context->layer;

    switch (undo_context->type) {
    case UNDO_ADD: {
        dynarray_delete_at(label_layer->ids, undo_context->index);
        dynarray_delete_at(label_layer->positions, undo_context->index);
        dynarray_delete_at(label_layer->colors, undo_context->index);
        dynarray_delete_at(label_layer->texts, undo_context->index);
    } break;

    case UNDO_DELETE: {
        dynarray_insert_before(label_layer->ids, undo_context->index, &undo_context->id);
        dynarray_insert_before(label_layer->positions, undo_context->index, &undo_context->position);
        dynarray_insert_before(label_layer->colors, undo_context->index, &undo_context->color);
        dynarray_insert_before(label_layer->texts, undo_context->index, &undo_context->text);
    } break;

    case UNDO_UPDATE: {
        dynarray_replace_at(label_layer->ids, undo_context->index, &undo_context->id);
        dynarray_replace_at(label_layer->positions, undo_context->index, &undo_context->position);
        dynarray_replace_at(label_layer->colors, undo_context->index, &undo_context->color);
        dynarray_replace_at(label_layer->texts, undo_context->index, &undo_context->text);
    } break;
    }
}

#define UNDO_PUSH(LAYER, HISTORY, UNDO_TYPE)                            \
    do {                                                                \
        UndoContext context = create_undo_context(LAYER, UNDO_TYPE);    \
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

LabelLayer *create_label_layer(void)
{
    Lt *lt = create_lt();

    LabelLayer *label_layer = PUSH_LT(
        lt, nth_calloc(1, sizeof(LabelLayer)), free);
    if (label_layer == NULL) {
        RETURN_LT(lt, NULL);
    }
    label_layer->lt = lt;

    label_layer->ids = PUSH_LT(
        lt,
        create_dynarray(sizeof(char) * LABEL_LAYER_ID_MAX_SIZE),
        destroy_dynarray);
    if (label_layer->ids == NULL) {
        RETURN_LT(lt, NULL);
    }

    label_layer->positions = PUSH_LT(lt, create_dynarray(sizeof(Point)), destroy_dynarray);
    if (label_layer->positions == NULL) {
        RETURN_LT(lt, NULL);
    }

    label_layer->colors = PUSH_LT(lt, create_dynarray(sizeof(Color)), destroy_dynarray);
    if (label_layer->colors == NULL) {
        RETURN_LT(lt, NULL);
    }

    label_layer->texts = PUSH_LT(
        lt,
        create_dynarray(sizeof(char) * LABEL_LAYER_TEXT_MAX_SIZE),
        destroy_dynarray);
    if (label_layer->texts == NULL) {
        RETURN_LT(lt, NULL);
    }

    label_layer->color_picker = create_color_picker_from_rgba(COLOR_RED);
    label_layer->selected = -1;

    label_layer->edit_field = PUSH_LT(
        lt,
        create_edit_field(LABELS_SIZE, COLOR_RED),
        destroy_edit_field);
    if (label_layer->edit_field == NULL) {
        RETURN_LT(lt, NULL);
    }

    return label_layer;
}

LabelLayer *create_label_layer_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);
    LabelLayer *label_layer = create_label_layer();

    if (label_layer == NULL) {
        RETURN_LT(label_layer->lt, NULL);
    }

    const char *line = line_stream_next(line_stream);
    if (line == NULL) {
        log_fail("Could not read amount of labels\n");
        RETURN_LT(label_layer->lt, NULL);
    }

    size_t n = 0;
    if (sscanf(line, "%zu", &n) == EOF) {
        log_fail("Could not parse amount of labels\n");
        RETURN_LT(label_layer->lt, NULL);
    }

    for (size_t i = 0; i < n; ++i) {
        char hex[7];
        char id[LABEL_LAYER_ID_MAX_SIZE];
        Point position;

        line = line_stream_next(line_stream);
        if (line == NULL) {
            log_fail("Could not read label meta info\n");
            RETURN_LT(label_layer->lt, NULL);
        }

        if (sscanf(
                line,
                "%"STRINGIFY(LABEL_LAYER_ID_MAX_SIZE)"s%f%f%6s\n",
                id, &position.x, &position.y, hex) == EOF) {
            log_fail("Could not parse label meta info\n");
            RETURN_LT(label_layer->lt, NULL);
        }

        Color color = hexstr(hex);

        dynarray_push(label_layer->ids, id);
        dynarray_push(label_layer->positions, &position);
        dynarray_push(label_layer->colors, &color);

        line = line_stream_next(line_stream);
        if (line == NULL) {
            log_fail("Could not read label text\n");
        }

        char label_text[LABEL_LAYER_TEXT_MAX_SIZE] = {0};
        memcpy(label_text, line, LABEL_LAYER_TEXT_MAX_SIZE - 1);
        trim_endline(label_text);
        dynarray_push(label_layer->texts, &label_text);
    }

    return label_layer;
}

void destroy_label_layer(LabelLayer *label_layer)
{
    trace_assert(label_layer);
    destroy_lt(label_layer->lt);
}

int label_layer_render(const LabelLayer *label_layer,
                       Camera *camera,
                       int active)
{
    trace_assert(label_layer);
    trace_assert(camera);

    if (active && color_picker_render(&label_layer->color_picker, camera) < 0) {
        return -1;
    }

    size_t n = dynarray_count(label_layer->ids);
    char *ids = dynarray_data(label_layer->ids);
    Point *positions = dynarray_data(label_layer->positions);
    Color *colors = dynarray_data(label_layer->colors);
    char *texts = dynarray_data(label_layer->texts);

    /* TODO(#891): LabelLayer doesn't show the final position of Label after the animation */
    for (size_t i = 0; i < n; ++i) {
        const Color color = label_layer->state == LABEL_LAYER_RECOLOR && label_layer->selected == (int) i
            ? label_layer->inter_color
            : colors[i];

        const Point position =
            label_layer->state == LABEL_LAYER_MOVE && label_layer->selected == (int) i
            ? label_layer->inter_position
            : positions[i];

        // Label Text
        if (label_layer->state == LABEL_LAYER_EDIT_TEXT && label_layer->selected == (int) i) {
            if (edit_field_render_world(
                    label_layer->edit_field,
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
        if (label_layer->state == LABEL_LAYER_EDIT_ID && label_layer->selected == (int)i) {
            if (edit_field_render_world(
                    label_layer->edit_field,
                    camera,
                    vec_sub(
                        position,
                        vec(0.0f, FONT_CHAR_HEIGHT))) < 0) {
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
                    vec_sub(position, vec(0.0f, FONT_CHAR_HEIGHT))) < 0) {
                return -1;
            }
        }

        // Label Selection
        if (active && label_layer->selected == (int) i) {
            Rect selection =
                rect_scale(
                    camera_rect(
                        camera,
                        rect_boundary2(
                            sprite_font_boundary_box(
                                camera_font(camera),
                                position,
                                LABELS_SIZE,
                                texts + label_layer->selected * LABEL_LAYER_TEXT_MAX_SIZE),
                            sprite_font_boundary_box(
                                camera_font(camera),
                                vec_sub(
                                    position,
                                    vec(0.0f, FONT_CHAR_HEIGHT)),
                                vec(1.0f, 1.0f),
                                ids + label_layer->selected * LABEL_LAYER_ID_MAX_SIZE))),
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
                           const Sprite_font *font,
                           Point position)
{
    trace_assert(label_layer);

    const size_t n = dynarray_count(label_layer->texts);
    char *ids = dynarray_data(label_layer->ids);
    char *texts = dynarray_data(label_layer->texts);
    Point *positions = dynarray_data(label_layer->positions);

    for (size_t i = 0; i < n; ++i) {
        Rect boundary = rect_boundary2(
            sprite_font_boundary_box(
                font,
                positions[i],
                LABELS_SIZE,
                texts + i * LABEL_LAYER_TEXT_MAX_SIZE),
            sprite_font_boundary_box(
                font,
                vec_sub(
                    positions[i],
                    vec(0.0f, FONT_CHAR_HEIGHT)),
                vec(1.0f, 1.0f),
                ids + i * LABEL_LAYER_ID_MAX_SIZE));

        if (rect_contains_point(boundary, position)) {
            return (int) i;
        }
    }

    return -1;
}

static
void label_layer_delete_selected_label(LabelLayer *label_layer,
                                       UndoHistory *undo_history)
{
    trace_assert(label_layer);
    trace_assert(label_layer->selected >= 0);

    UNDO_PUSH(label_layer, undo_history, UNDO_DELETE);

    dynarray_delete_at(label_layer->ids, (size_t)label_layer->selected);
    dynarray_delete_at(label_layer->positions, (size_t)label_layer->selected);
    dynarray_delete_at(label_layer->colors, (size_t)label_layer->selected);
    dynarray_delete_at(label_layer->texts, (size_t)label_layer->selected);

    label_layer->selected = -1;
}

static
int label_layer_add_label(LabelLayer *label_layer,
                          Point position,
                          Color color,
                          const char *text,
                          UndoHistory *undo_history)
{
    trace_assert(label_layer);

    // TODO(#982): id generation code is duplicated in label_layer, point_layer and rect_layer
    char id[LABEL_LAYER_ID_MAX_SIZE];
    for (size_t i = 0; i < LABEL_LAYER_ID_MAX_SIZE - 1; ++i) {
        id[i] = (char) ('a' + rand() % ('z' - 'a' + 1));
    }
    id[LABEL_LAYER_ID_MAX_SIZE - 1] = '\0';

    size_t n = dynarray_count(label_layer->ids);

    dynarray_push(label_layer->ids, id);
    dynarray_push(label_layer->positions, &position);
    dynarray_push(label_layer->colors, &color);
    dynarray_push_empty(label_layer->texts);
    memcpy(
        dynarray_pointer_at(label_layer->texts, n),
        text,
        min_size_t(LABEL_LAYER_ID_MAX_SIZE - 1, strlen(text)));

    UNDO_PUSH(label_layer, undo_history, UNDO_ADD);

    return (int) n;
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
        if (label_layer->selected >= 0) {
            label_layer->state = LABEL_LAYER_RECOLOR;
            label_layer->inter_color = color_picker_rgba(&label_layer->color_picker);
        }
        return 0;
    }

    Color *colors = dynarray_data(label_layer->colors);
    Point *positions = dynarray_data(label_layer->positions);
    char *ids = dynarray_data(label_layer->ids);
    char *texts = dynarray_data(label_layer->texts);

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            const Point position = camera_map_screen(
                camera,
                event->button.x,
                event->button.y);

            const int element = label_layer_element_at(
                label_layer,
                camera_font(camera),
                position);

            if (element >= 0) {
                label_layer->move_anchor = vec_sub(position, positions[element]);
                label_layer->selected = element;
                label_layer->state = LABEL_LAYER_MOVE;
                label_layer->inter_position = positions[element];

                label_layer->color_picker =
                    create_color_picker_from_rgba(colors[element]);
            } else {
                label_layer->selected = label_layer_add_label(
                    label_layer,
                    position,
                    color_picker_rgba(
                        &label_layer->color_picker),
                    "",
                    undo_history);
                label_layer->state = LABEL_LAYER_EDIT_TEXT;
                edit_field_replace(
                    label_layer->edit_field,
                    texts + label_layer->selected * LABEL_LAYER_TEXT_MAX_SIZE);
                edit_field_restyle(
                    label_layer->edit_field,
                    LABELS_SIZE,
                    colors[label_layer->selected]);
                SDL_StartTextInput();
            }
        } break;
        }
    } break;

    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_F2: {
            if (label_layer->selected >= 0) {
                label_layer->state = LABEL_LAYER_EDIT_TEXT;
                edit_field_replace(
                    label_layer->edit_field,
                    texts + label_layer->selected * LABEL_LAYER_TEXT_MAX_SIZE);
                edit_field_restyle(
                    label_layer->edit_field,
                    LABELS_SIZE,
                    colors[label_layer->selected]);
                SDL_StartTextInput();
            }
        } break;

        case SDLK_F3: {
            if (label_layer->selected >= 0) {
                label_layer->state = LABEL_LAYER_EDIT_ID;
                edit_field_replace(
                    label_layer->edit_field,
                    ids + label_layer->selected * LABEL_LAYER_ID_MAX_SIZE);
                edit_field_restyle(
                    label_layer->edit_field,
                    vec(1.0f, 1.0f),
                    color_invert(colors[label_layer->selected]));
                SDL_StartTextInput();
            }
        } break;

        case SDLK_DELETE: {
            if (label_layer->selected >= 0) {
                label_layer_delete_selected_label(
                    label_layer,
                    undo_history);
                label_layer->selected = -1;
            }
        } break;

        case SDLK_c: {
            if ((event->key.keysym.mod & KMOD_LCTRL) && label_layer->selected >= 0) {
                clipboard = 1;
                dynarray_copy_to(label_layer->texts, clipboard_text, (size_t)label_layer->selected);
                dynarray_copy_to(label_layer->colors, &clipboard_color, (size_t)label_layer->selected);
            }
        } break;

        case SDLK_v: {
            if ((event->key.keysym.mod & KMOD_LCTRL) && clipboard) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                Point position = camera_map_screen(camera, x, y);

                label_layer_add_label(
                    label_layer,
                    position,
                    clipboard_color,
                    clipboard_text,
                    undo_history);
            }
        } break;
        }
    } break;
    }

    return 0;
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
    trace_assert(label_layer->selected >= 0);

    switch (event->type) {
    case SDL_MOUSEMOTION: {
        label_layer->inter_position = vec_sub(
            camera_map_screen(
                camera,
                event->motion.x,
                event->motion.y),
            label_layer->move_anchor);
    } break;

    case SDL_MOUSEBUTTONUP: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            UNDO_PUSH(label_layer, undo_history, UNDO_UPDATE);

            dynarray_replace_at(
                label_layer->positions,
                (size_t)label_layer->selected,
                &label_layer->inter_position);
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
    trace_assert(label_layer->selected >= 0);

    switch (event->type) {
    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_RETURN: {
            UNDO_PUSH(label_layer, undo_history, UNDO_UPDATE);

            char *text =
                (char*)dynarray_data(label_layer->texts) + label_layer->selected * LABEL_LAYER_TEXT_MAX_SIZE;
            memset(text, 0, LABEL_LAYER_TEXT_MAX_SIZE);
            memcpy(text, edit_field_as_text(label_layer->edit_field), LABEL_LAYER_TEXT_MAX_SIZE - 1);
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

    return edit_field_event(label_layer->edit_field, event);
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
    trace_assert(label_layer->selected >= 0);

    switch (event->type) {
    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_RETURN: {
            UNDO_PUSH(label_layer, undo_history, UNDO_UPDATE);

            char *id =
                (char*)dynarray_data(label_layer->ids) + label_layer->selected * LABEL_LAYER_ID_MAX_SIZE;
            memset(id, 0, LABEL_LAYER_ID_MAX_SIZE);
            memcpy(id, edit_field_as_text(label_layer->edit_field), LABEL_LAYER_ID_MAX_SIZE - 1);
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

    return edit_field_event(label_layer->edit_field, event);
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
    trace_assert(label_layer->selected >= 0);

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
            UNDO_PUSH(label_layer, undo_history, UNDO_UPDATE);

            dynarray_replace_at(
                label_layer->colors,
                (size_t) label_layer->selected,
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
    return dynarray_count(label_layer->ids);
}

char *label_layer_ids(const LabelLayer *label_layer)
{
    return dynarray_data(label_layer->ids);
}

Point *label_layer_positions(const LabelLayer *label_layer)
{
    return dynarray_data(label_layer->positions);
}

Color *label_layer_colors(const LabelLayer *label_layer)
{
    return dynarray_data(label_layer->colors);
}

char *labels_layer_texts(const LabelLayer *label_layer)
{
    return dynarray_data(label_layer->texts);
}

int label_layer_dump_stream(const LabelLayer *label_layer, FILE *filedump)
{
    trace_assert(label_layer);
    trace_assert(filedump);

    size_t n = dynarray_count(label_layer->ids);
    char *ids = dynarray_data(label_layer->ids);
    Point *positions = dynarray_data(label_layer->positions);
    Color *colors = dynarray_data(label_layer->colors);
    char *texts = dynarray_data(label_layer->texts);

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
