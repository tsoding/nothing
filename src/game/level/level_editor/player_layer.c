#include <stdio.h>

#include <SDL.h>

#include "game/camera.h"
#include "system/stacktrace.h"
#include "player_layer.h"
#include "system/nth_alloc.h"
#include "system/log.h"

PlayerLayer *create_player_layer(Vec position, Color color)
{
    Lt *lt = create_lt();

    PlayerLayer *player_layer = PUSH_LT(lt, nth_calloc(1, sizeof(PlayerLayer)), free);
    player_layer->lt = lt;

    player_layer->position = position;
    player_layer->color_picker.color = color;

    return player_layer;
}

PlayerLayer *create_player_layer_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);

    PlayerLayer *player_layer = create_player_layer(
        vec(0.0f, 0.0f), COLOR_BLACK);

    const char *line = line_stream_next(line_stream);
    if (line == NULL) {
        log_fail("Could not read Player Layer\n");
        RETURN_LT(player_layer->lt, NULL);
    }

    char colorstr[7];
    Point position;
    if (sscanf(line,
               "%f%f%6s",
               &position.x,
               &position.y,
               colorstr) == EOF) {
        log_fail("Could not read Player Layer\n");
        RETURN_LT(player_layer->lt, NULL);
    }

    return create_player_layer(position, hexstr(colorstr));
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
                        int active)
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
                rgba(1.0f, 1.0f, 1.0f, active ? 1.0f : 0.0f))) < 0) {
        return -1;
    }

    return 0;
}


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

int player_layer_dump_stream(const PlayerLayer *player_layer,
                             FILE *filedump)
{
    trace_assert(player_layer);
    trace_assert(filedump);

    fprintf(filedump, "%f %f ", player_layer->position.x, player_layer->position.y);
    color_hex_to_stream(player_layer->color_picker.color, filedump);
    fprintf(filedump, "\n");

    return 0;
}
