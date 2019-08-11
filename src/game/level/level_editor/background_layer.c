#include <SDL.h>

#include "system/stacktrace.h"
#include "math/rect.h"
#include "color.h"
#include "background_layer.h"

// TODO(#1012): UndoHistory is not support in BackgroundLayer

int background_layer_render(BackgroundLayer *layer,
                            Camera *camera,
                            int active)
{
    trace_assert(layer);
    trace_assert(camera);

    if (active) {
        return color_picker_render(&layer->color_picker, camera);
    }

    return 0;
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

    return color_picker_event(
        &layer->color_picker, event,
        NULL);
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
