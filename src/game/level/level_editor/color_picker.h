#ifndef COLOR_PICKER_H_
#define COLOR_PICKER_H_

#include <stdbool.h>
#include "layer.h"
#include "ui/slider.h"

typedef struct {
    Color color;
    Slider slider;
} ColorPicker;

typedef struct LineStream LineStream;

int color_picker_read_from_line_stream(ColorPicker *color_picker,
                                       LineStream *line_stream);

LayerPtr color_picker_as_layer(ColorPicker *color_picker);

int color_picker_render(const ColorPicker *color_picker,
                        Camera *camera);
int color_picker_event(ColorPicker *color_picker, const SDL_Event *event, int *selected);

#endif  // COLOR_PICKER_H_
