#include <SDL.h>

#include "system/stacktrace.h"
#include "game/camera.h"
#include "math/rect.h"
#include "color.h"
#include "background_layer.h"
#include "undo_history.h"

// TODO(#1012): UndoHistory is not support in BackgroundLayer

BackgroundLayer create_background_layer(Color color)
{
    BackgroundLayer layer = {
        .color_picker = create_color_picker_from_rgba(color),
        .prev_color = color
    };
    return layer;
}

int background_layer_read_from_line_stream(BackgroundLayer *layer,
                                           LineStream *line_stream)
{
    if (color_picker_read_from_line_stream(
            &layer->color_picker,
            line_stream) < 0) {
        return -1;
    }

    layer->prev_color = color_picker_rgba(&layer->color_picker);

    return 0;
}

int background_layer_render(BackgroundLayer *layer,
                            Camera *camera,
                            int active)
{
    trace_assert(layer);
    trace_assert(camera);

    if (active) {
        return color_picker_render(
            &layer->color_picker,
            camera);
    }

    return 0;
}

static
void background_undo_color(void *layer, Context context)
{
    trace_assert(layer);
    BackgroundLayer *background_layer = layer;

    trace_assert(sizeof(Color) < CONTEXT_SIZE);
    Color *color = (Color *)context.data;

    background_layer->color_picker = create_color_picker_from_rgba(*color);
}

int background_layer_event(BackgroundLayer *layer,
                           const SDL_Event *event,
                           const Camera *camera,
                           UndoHistory *undo_history)
{
    trace_assert(layer);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(undo_history);

    int selected = 0;

    if (color_picker_event(
            &layer->color_picker,
            event,
            camera,
            &selected) < 0) {
        return -1;
    }

    if (selected && !color_picker_drag(&layer->color_picker)) {
        undo_history_push(
            undo_history,
            create_action(
                layer,
                background_undo_color,
                &layer->prev_color, sizeof(layer->prev_color)));
        layer->prev_color = color_picker_rgba(&layer->color_picker);
    }

    return 0;
}

int background_layer_dump_stream(BackgroundLayer *layer,
                                 FILE *stream)
{
    trace_assert(layer);
    trace_assert(stream);

    color_hex_to_stream(
        color_picker_rgba(&layer->color_picker),
        stream);

    return fprintf(stream, "\n");;
}
