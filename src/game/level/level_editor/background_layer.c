#include <string.h>
#include <SDL.h>

#include "system/stacktrace.h"
#include "game/camera.h"
#include "math/rect.h"
#include "color.h"
#include "background_layer.h"
#include "undo_history.h"

BackgroundLayer create_background_layer(Color color)
{
    BackgroundLayer layer = {
        .color_picker = create_color_picker_from_rgba(color),
        .prev_color = color
    };
    return layer;
}

BackgroundLayer chop_background_layer(String *input)
{
    String line = trim(chop_by_delim(input, '\n'));
    return create_background_layer(hexs(line));
}

int background_layer_render(BackgroundLayer *layer,
                            const Camera *camera,
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

typedef struct {
    BackgroundLayer *layer;
    Color color;
} BackgroundUndoContext;

static
void background_undo_color(void *context, size_t context_size)
{
    trace_assert(context);
    trace_assert(sizeof(BackgroundUndoContext) == context_size);

    BackgroundUndoContext *undo_context = context;
    BackgroundLayer *background_layer = undo_context->layer;

    background_layer->color_picker = create_color_picker_from_rgba(undo_context->color);
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
        BackgroundUndoContext context = {
            .layer = layer,
            .color = layer->prev_color
        };

        undo_history_push(
            undo_history,
            background_undo_color,
            &context, sizeof(context));
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

    return fprintf(stream, "\n");
}
