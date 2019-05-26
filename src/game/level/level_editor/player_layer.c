#include <SDL.h>

#include "game/camera.h"
#include "system/stacktrace.h"
#include "player_layer.h"
#include "system/nth_alloc.h"
#include "system/log.h"

PlayerLayer *create_player_layer_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);

    Lt *lt = create_lt();

    PlayerLayer *player_layer = PUSH_LT(lt, nth_calloc(1, sizeof(PlayerLayer)), free);
    player_layer->lt = lt;

    char colorstr[7];

    const char *line = line_stream_next(line_stream);
    if (line == NULL) {
        log_fail("Could not read Player Layer\n");
        RETURN_LT(lt, NULL);
    }

    if (sscanf(line,
               "%f%f%6s",
               &player_layer->position.x,
               &player_layer->position.y,
               colorstr) == EOF) {
        log_fail("Could not read Player Layer\n");
        RETURN_LT(lt, NULL);
    }
    player_layer->color_picker.color = hexstr(colorstr);

    line = line_stream_next(line_stream);
    if (line == NULL) {
        log_fail("Could not read Player Layer\n");
        RETURN_LT(lt, NULL);
    }

    size_t n;
    if (sscanf(line, "%lu", &n) == EOF) {
        log_fail("Could not read Player Layer\n");
        RETURN_LT(lt, NULL);
    }

    player_layer->source_code = PUSH_LT(
        lt, line_stream_collect_n_lines(line_stream, n), free);
    if (player_layer->source_code == NULL) {
        log_fail("Could not read Player Layer\n");
        RETURN_LT(lt, NULL);
    }

    return player_layer;
}

void destroy_player_layer(PlayerLayer *player_layer)
{
    trace_assert(player_layer);
    destroy_lt(player_layer->lt);
}

LayerPtr player_layer_as_layer(PlayerLayer *player_layer)
{
    LayerPtr layer = {
        .type = LAYER_PLAYER,
        .ptr = player_layer
    };
    return layer;
}

int player_layer_render(const PlayerLayer *player_layer,
                        Camera *camera,
                        float fa)
{
    trace_assert(player_layer);
    trace_assert(camera);

    if (camera_fill_rect(
            camera,
            rect_from_vecs(
                player_layer->position,
                vec(25.0f, 25.0f)),
            color_scale(
                player_layer->color_picker.color,
                rgba(1.0f, 1.0f, 1.0f, fa))) < 0) {
        return -1;
    }

    return 0;
}

// TODO(#859): PlayerLayer does not allow to change to position of the Player

int player_layer_event(PlayerLayer *player_layer,
                       const SDL_Event *event,
                       const Camera *camera)
{
    trace_assert(player_layer);
    trace_assert(event);
    trace_assert(camera);

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP: {
        bool selected = false;
        if (color_picker_mouse_button(
                &player_layer->color_picker,
                &event->button,
                &selected) < 0) {
            return -1;
        }

        if (!selected &&
            event->type == SDL_MOUSEBUTTONUP &&
            event->button.button == SDL_BUTTON_LEFT) {
            player_layer->position =
                camera_map_screen(camera,
                                  event->button.x,
                                  event->button.y);
        }
    } break;
    }

    return 0;
}
