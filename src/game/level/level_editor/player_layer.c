#include <SDL2/SDL.h>

#include "game/camera.h"
#include "system/stacktrace.h"
#include "player_layer.h"

LayerPtr player_layer_as_layer(PlayerLayer *player_layer)
{
    LayerPtr layer = {
        .type = LAYER_PLAYER,
        .ptr = player_layer
    };
    return layer;
}

int player_layer_render(const PlayerLayer *player_layer,
                        Camera *camera)
{
    trace_assert(player_layer);
    trace_assert(camera);

    if (camera_fill_rect(
            camera,
            rect_from_vecs(
                player_layer->position,
                vec(25.0f, 25.0f)),
            player_layer->color_picker.color) < 0) {
        return -1;
    }

    return 0;
}

// TODO: PlayerLayer does not allow to change to position of the Player

int player_layer_event(PlayerLayer *player_layer,
                       const SDL_Event *event,
                       const Camera *camera)
{
    trace_assert(player_layer);
    trace_assert(event);
    trace_assert(camera);

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        return color_picker_mouse_button(
            &player_layer->color_picker,
            &event->button,
            NULL);
    }

    return 0;
}
