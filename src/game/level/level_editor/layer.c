#include "game/camera.h"
#include "rect_layer.h"
#include "point_layer.h"
#include "player_layer.h"
#include "label_layer.h"
#include "./layer.h"

int layer_render(LayerPtr layer, Camera *camera, int active)
{
    switch (layer.type) {
    case LAYER_RECT:
        return rect_layer_render(layer.ptr, camera, active);

    case LAYER_POINT:
        return point_layer_render(layer.ptr, camera, active);

    case LAYER_PLAYER:
        return player_layer_render(layer.ptr, camera, active);

    case LAYER_COLOR_PICKER:
        return active ? color_picker_render(layer.ptr, camera) : 0;

    case LAYER_LABEL:
        return label_layer_render(layer.ptr, camera, active);
    }

    return -1;
}

int layer_event(LayerPtr layer,
                const SDL_Event *event,
                const Camera *camera,
                UndoHistory *undo_history)
{
    switch (layer.type) {
    case LAYER_RECT:
        return rect_layer_event(layer.ptr, event, camera, undo_history);

    case LAYER_POINT:
        return point_layer_event(layer.ptr, event, camera, undo_history);

    case LAYER_PLAYER:
        return player_layer_event(layer.ptr, event, camera, undo_history);

    case LAYER_COLOR_PICKER:
        // TODO(#1000): undo history is not really applicable to color picker as layer
        return color_picker_event(layer.ptr, event, NULL, undo_history);

    case LAYER_LABEL:
        return label_layer_event(layer.ptr, event, camera, undo_history);
    }

    return -1;
}

int layer_dump_stream(LayerPtr layer,
                      FILE *stream)
{
    switch (layer.type) {
    case LAYER_RECT:
        return rect_layer_dump_stream(layer.ptr, stream);

    case LAYER_POINT:
        return point_layer_dump_stream(layer.ptr, stream);

    case LAYER_PLAYER:
        return player_layer_dump_stream(layer.ptr, stream);

    case LAYER_COLOR_PICKER: {
        color_hex_to_stream(color_picker_rgba(layer.ptr), stream);
        return fprintf(stream, "\n");
    }

    case LAYER_LABEL:
        return label_layer_dump_stream(layer.ptr, stream);
    }

    return -1;
}
