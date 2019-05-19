#ifndef PLAYER_LAYER_H_
#define PLAYER_LAYER_H_

#include "color_picker.h"
#include "layer.h"

typedef struct {
    Vec position;
    ColorPicker color_picker;
} PlayerLayer;

LayerPtr player_layer_as_layer(PlayerLayer *player_layer);
int player_layer_render(const PlayerLayer *player_layer,
                        Camera *camera);
int player_layer_event(PlayerLayer *player_layer,
                       const SDL_Event *event,
                       const Camera *camera);

#endif  // PLAYER_LAYER_H_
