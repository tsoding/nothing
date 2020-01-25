#ifndef LAYER_PICKER_H_
#define LAYER_PICKER_H_

#include "game/camera.h"

typedef struct RectLayer RectLayer;

typedef enum {
    LAYER_PICKER_BACKGROUND = 0,
    LAYER_PICKER_PLAYER,
    LAYER_PICKER_BACK_PLATFORMS,
    LAYER_PICKER_PLATFORMS,
    LAYER_PICKER_GOALS,
    LAYER_PICKER_LAVA,
    LAYER_PICKER_BOXES,
    LAYER_PICKER_LABELS,
    LAYER_PICKER_REGIONS,
    LAYER_PICKER_PP,

    LAYER_PICKER_N
} LayerPicker;

int layer_picker_render(const LayerPicker *layer_picker,
                        const Camera *camera);
int layer_picker_event(LayerPicker *layer_picker,
                       const SDL_Event *event,
                       const Camera *camera,
                       bool *selected);

#endif  // LAYER_PICKER_H_
