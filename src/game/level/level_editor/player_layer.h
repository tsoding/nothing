#ifndef PLAYER_LAYER_H_
#define PLAYER_LAYER_H_

#include "color_picker.h"
#include "layer.h"
#include "system/lt.h"
#include "system/line_stream.h"

typedef struct {
    Vec position;
    ColorPicker color_picker;
    Color prev_color;
} PlayerLayer;

PlayerLayer create_player_layer(Vec position, Color color);
PlayerLayer create_player_layer_from_line_stream(LineStream *line_stream);

LayerPtr player_layer_as_layer(PlayerLayer *player_layer);
int player_layer_render(const PlayerLayer *player_layer,
                        Camera *camera,
                        int active);
int player_layer_event(PlayerLayer *player_layer,
                       const SDL_Event *event,
                       const Camera *camera,
                       UndoHistory *undo_history);

int player_layer_dump_stream(const PlayerLayer *player_layer,
                             FILE *filedump);

#endif  // PLAYER_LAYER_H_
