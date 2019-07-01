#ifndef COLOR_PICKER_H_
#define COLOR_PICKER_H_

#include <stdbool.h>
#include "layer.h"
#include "ui/slider.h"

typedef struct {
    // TODO(#933): ColorPicker should use array of sliders
    Slider hue;
    Slider saturation;
    Slider lightness;
} ColorPicker;

typedef struct LineStream LineStream;

ColorPicker create_color_picker_from_rgba(Color color);

int color_picker_read_from_line_stream(ColorPicker *color_picker,
                                       LineStream *line_stream);

LayerPtr color_picker_as_layer(ColorPicker *color_picker);

int color_picker_render(const ColorPicker *color_picker,
                        Camera *camera);
int color_picker_event(ColorPicker *color_picker, const SDL_Event *event, int *selected);

Color color_picker_rgba(const ColorPicker *color_picker);

#endif  // COLOR_PICKER_H_
