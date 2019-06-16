#include "game/camera.h"
#include "rect_layer.h"
#include "point_layer.h"
#include "player_layer.h"
#include "label_layer.h"
#include "./layer.h"

int layer_render(LayerPtr layer, Camera *camera, float fa)
{
    switch (layer.type) {
    case LAYER_RECT:
        return rect_layer_render(layer.ptr, camera, fa);

    case LAYER_POINT:
        return point_layer_render(layer.ptr, camera, fa);

    case LAYER_PLAYER:
        return player_layer_render(layer.ptr, camera, fa);

    case LAYER_COLOR_PICKER:
        return color_picker_render(layer.ptr, camera);

    case LAYER_LABEL:
        return label_layer_render(layer.ptr, camera, fa);
    }

    return -1;
}

int layer_event(LayerPtr layer,
                const SDL_Event *event,
                const Camera *camera)
{
    switch (layer.type) {
    case LAYER_RECT:
        return rect_layer_event(layer.ptr, event, camera);

    case LAYER_POINT:
        return point_layer_event(layer.ptr, event, camera);

    case LAYER_PLAYER:
        return player_layer_event(layer.ptr, event, camera);

    case LAYER_COLOR_PICKER: {
        if (event->type == SDL_MOUSEBUTTONUP || event->type == SDL_MOUSEBUTTONDOWN) {
            return color_picker_mouse_button(layer.ptr, &event->button, NULL);
        } else {
            return 0;
        }
    } break;
    case LAYER_LABEL:
        return label_layer_event(layer.ptr, event, camera);
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
        color_hex_to_stream(((ColorPicker*)layer.ptr)->color, stream);
        return fprintf(stream, "\n");
    }

    case LAYER_LABEL:
        return label_layer_dump_stream(layer.ptr, stream);
    }

    return -1;
}
