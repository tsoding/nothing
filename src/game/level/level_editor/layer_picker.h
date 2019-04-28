#ifndef LAYER_PICKER_H_
#define LAYER_PICKER_H_

typedef struct Layer Layer;
typedef struct Camera Camera;

typedef enum {
    LAYER_PICKER_BOXES = 0,
    LAYER_PICKER_PLATFORMS,
    LAYER_PICKER_BACK_PLATFORMS,

    LAYER_PICKER_N
} LayerPicker;

int layer_picker_render(const LayerPicker *layer_picker,
                        Camera *camera);
int layer_picker_event(LayerPicker *layer_picker,
                       const SDL_Event *event);

#endif  // LAYER_PICKER_H_
