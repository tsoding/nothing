#ifndef PLAYER_LAYER_H_
#define PLAYER_LAYER_H_

#include "color_picker.h"
#include "layer.h"
#include "system/lt.h"
#include "system/line_stream.h"

typedef struct {
    Lt *lt;
    Vec position;
    ColorPicker color_picker;
} PlayerLayer;

PlayerLayer *create_player_layer_from_line_stream(LineStream *line_stream);
void destroy_player_layer(PlayerLayer *player_layer);

LayerPtr player_layer_as_layer(PlayerLayer *player_layer);
int player_layer_render(const PlayerLayer *player_layer,
                        Camera *camera,
                        float fa);
int player_layer_event(PlayerLayer *player_layer,
                       const SDL_Event *event,
                       const Camera *camera);



#endif  // PLAYER_LAYER_H_
