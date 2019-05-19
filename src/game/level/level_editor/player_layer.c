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

    return 0;
}

int player_layer_event(PlayerLayer *player_layer,
                       const SDL_Event *event,
                       const Camera *camera)
{
    trace_assert(player_layer);
    trace_assert(event);
    trace_assert(camera);
    return 0;
}
