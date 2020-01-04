#ifndef BACKGROUND_LAYER_H_
#define BACKGROUND_LAYER_H_

#include "color_picker.h"
#include "system/s.h"

typedef struct {
    ColorPicker color_picker;
    Color prev_color;
} BackgroundLayer;

BackgroundLayer create_background_layer(Color color);
BackgroundLayer chop_background_layer(String *input);

static inline
LayerPtr background_layer_as_layer(BackgroundLayer *layer)
{
    return (LayerPtr) {
        .ptr = layer,
        .type = LAYER_BACKGROUND
    };
}

int background_layer_render(BackgroundLayer *layer,
                            const Camera *camera,
                            int active);
int background_layer_event(BackgroundLayer *layer,
                           const SDL_Event *event,
                           const Camera *camera,
                           UndoHistory *undo_history);
int background_layer_dump_stream(BackgroundLayer *layer,
                                 FILE *stream);

#endif  // BACKGROUND_LAYER_H_
