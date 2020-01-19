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


ColorPicker create_color_picker_from_rgba(Color color);

int color_picker_render(const ColorPicker *color_picker,
                        const Camera *camera);
int color_picker_event(ColorPicker *color_picker,
                       const SDL_Event *event,
                       const Camera *camera,
                       int *selected);

Color color_picker_rgba(const ColorPicker *color_picker);

static inline
int color_picker_drag(const ColorPicker *color_picker)
{
    int result = 0;

    for (int i = 0; i < COLOR_SLIDER_N; ++i) {
        result = result || color_picker->sliders[i].drag;
    }

    return result;
}

#endif  // COLOR_PICKER_H_
