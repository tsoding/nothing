#ifndef COLOR_PICKER_H_
#define COLOR_PICKER_H_

#include <stdbool.h>
#include "layer.h"

typedef struct {
    Color color;
} ColorPicker;

LayerPtr color_picker_as_layer(ColorPicker *color_picker);

int color_picker_render(const ColorPicker *color_picker,
                        Camera *camera);
int color_picker_mouse_button(ColorPicker *color_picker,
                              const SDL_MouseButtonEvent *event,
                              bool *selected);

#endif  // COLOR_PICKER_H_
