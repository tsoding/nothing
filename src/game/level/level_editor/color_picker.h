#ifndef COLOR_PICKER_H_
#define COLOR_PICKER_H_

#include <stdbool.h>
#include "layer.h"
#include "ui/slider.h"

typedef enum {
    COLOR_SLIDER_HUE = 0,
    COLOR_SLIDER_SAT,
    COLOR_SLIDER_LIT,
    COLOR_SLIDER_N
} ColorPickerSlider;

typedef struct {
    Slider sliders[COLOR_SLIDER_N];
} ColorPicker;

typedef struct LineStream LineStream;
typedef struct UndoHistory UndoHistory;

ColorPicker create_color_picker_from_rgba(Color color);

int color_picker_read_from_line_stream(ColorPicker *color_picker,
                                       LineStream *line_stream);

LayerPtr color_picker_as_layer(ColorPicker *color_picker);

int color_picker_render(const ColorPicker *color_picker,
                        Camera *camera);
int color_picker_event(ColorPicker *color_picker,
                       const SDL_Event *event,
                       int *selected,
                       UndoHistory *undo_history);

Color color_picker_rgba(const ColorPicker *color_picker);

#endif  // COLOR_PICKER_H_
