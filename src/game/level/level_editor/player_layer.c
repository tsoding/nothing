#include <SDL2/SDL.h>

#include "game/camera.h"
#include "system/stacktrace.h"
#include "player_layer.h"

int player_layer_render(const PlayerLayer *player_layer,
                        Camera *camera)
{
    trace_assert(player_layer);
    trace_assert(camera);

    return 0;
}

int player_layer_mouse_button(PlayerLayer *player_layer,
                              const SDL_MouseButtonEvent *mouse_button)
{
    trace_assert(player_layer);
    trace_assert(mouse_button);

    return 0;
}

int player_layer_mouse_motion(PlayerLayer *player_layer,
                              const SDL_MouseMotionEvent *mouse_motion)
{
    trace_assert(player_layer);
    trace_assert(mouse_motion);

    return 0;
}
