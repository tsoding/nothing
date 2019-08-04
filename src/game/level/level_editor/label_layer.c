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

#define LABEL_LAYER_SELECTION_THICCNESS 5.0f

// TODO(#999): LabelLayer does not support UndoHistory

typedef enum {
    LABEL_LAYER_IDLE = 0,
    LABEL_LAYER_MOVE,
    LABEL_LAYER_EDIT_TEXT,
    LABEL_LAYER_EDIT_ID
} LabelLayerState;

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
};

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
        if (label_layer->state == LABEL_LAYER_EDIT_TEXT && label_layer->selected == (int) i) {
            if (edit_field_render_world(
                    label_layer->edit_field,
                    camera,
                    positions[i]) < 0) {
                return -1;
            }
        } else {
            if (camera_render_text(
                    camera,
                    texts + i * LABEL_LAYER_TEXT_MAX_SIZE,
                    LABELS_SIZE,
                    color_scale(
                        colors[i],
                        rgba(1.0f, 1.0f, 1.0f, active ? 1.0f : 0.5f)),
                    positions[i]) < 0) {
                return -1;
            }
        }

        if (label_layer->state == LABEL_LAYER_EDIT_ID && label_layer->selected == (int)i) {
            if (edit_field_render_world(
                    label_layer->edit_field,
                    camera,
                    vec_sub(
                        positions[i],
                        vec(0.0f, FONT_CHAR_HEIGHT))) < 0) {
                return -1;
            }
        } else {
            if (camera_render_text(
                    camera,
                    ids + i * LABEL_LAYER_ID_MAX_SIZE,
                    vec(1.0f, 1.0f),
                    color_scale(
                        color_invert(colors[i]),
                        rgba(1.0f, 1.0f, 1.0f, active ? 1.0f : 0.5f)),
                    vec_sub(positions[i], vec(0.0f, FONT_CHAR_HEIGHT))) < 0) {
                return -1;
            }
        }
    }

    if (label_layer->selected >= 0) {

        Rect selection =
            rect_scale(
                camera_rect(
                    camera,
                    rect_boundary2(
                        sprite_font_boundary_box(
                            camera_font(camera),
                            positions[label_layer->selected],
                            LABELS_SIZE,
                            texts + label_layer->selected * LABEL_LAYER_TEXT_MAX_SIZE),
                        sprite_font_boundary_box(
                            camera_font(camera),
                            vec_sub(
                                positions[label_layer->selected],
                                vec(0.0f, FONT_CHAR_HEIGHT)),
                            vec(1.0f, 1.0f),
                            ids + label_layer->selected * LABEL_LAYER_ID_MAX_SIZE))),
                LABEL_LAYER_SELECTION_THICCNESS * 0.5f);


        if (camera_draw_thicc_rect_screen(
                camera,
                selection,
                colors[label_layer->selected],
                LABEL_LAYER_SELECTION_THICCNESS) < 0) {
            return -1;
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
void label_layer_delete_nth_label(LabelLayer *label_layer,
                                  size_t i)
{
    trace_assert(label_layer);
    dynarray_delete_at(label_layer->ids, i);
    dynarray_delete_at(label_layer->positions, i);
    dynarray_delete_at(label_layer->colors, i);
    dynarray_delete_at(label_layer->texts, i);
}

static
int label_layer_add_label(LabelLayer *label_layer,
                           Point position,
                           Color color)
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

    return (int) n;
}

static
int label_layer_idle_event(LabelLayer *label_layer,
                           const SDL_Event *event,
                           const Camera *camera)
{
    trace_assert(label_layer);
    trace_assert(event);
    trace_assert(camera);

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

                label_layer->color_picker =
                    create_color_picker_from_rgba(colors[element]);
            } else {
                label_layer->selected = label_layer_add_label(
                    label_layer,
                    position,
                    color_picker_rgba(
                        &label_layer->color_picker));
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
                label_layer_delete_nth_label(
                    label_layer,
                    (size_t) label_layer->selected);
                label_layer->selected = -1;
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
                           const Camera *camera)
{
    trace_assert(label_layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(label_layer->selected >= 0);

    switch (event->type) {
    case SDL_MOUSEMOTION: {
        Point *positions = dynarray_data(label_layer->positions);
        positions[label_layer->selected] =
            vec_sub(
                camera_map_screen(
                    camera,
                    event->motion.x,
                    event->motion.y),
                label_layer->move_anchor);
    } break;

    case SDL_MOUSEBUTTONUP: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
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
                                const Camera *camera)
{
    trace_assert(label_layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(label_layer->selected >= 0);

    switch (event->type) {
    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_RETURN: {
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
                              const Camera *camera)
{
    trace_assert(label_layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(label_layer->selected >= 0);

    switch (event->type) {
    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_RETURN: {
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

int label_layer_event(LabelLayer *label_layer,
                      const SDL_Event *event,
                      const Camera *camera,
                      UndoHistory *undo_history)
{
    trace_assert(label_layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(undo_history);

    int changed = 0;

    if (color_picker_event(
            &label_layer->color_picker,
            event,
            &changed,
            undo_history) < 0) {
        return -1;
    }

    if (changed) {
        if (label_layer->selected >= 0) {
            Color *colors = dynarray_data(label_layer->colors);
            colors[label_layer->selected] =
                color_picker_rgba(&label_layer->color_picker);
        }
        return 0;
    }

    switch (label_layer->state) {
    case LABEL_LAYER_IDLE:
        return label_layer_idle_event(label_layer, event, camera);

    case LABEL_LAYER_MOVE:
        return label_layer_move_event(label_layer, event, camera);

    case LABEL_LAYER_EDIT_TEXT:
        return label_layer_edit_text_event(label_layer, event, camera);

    case LABEL_LAYER_EDIT_ID:
        return label_layer_edit_id_event(label_layer, event, camera);
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
