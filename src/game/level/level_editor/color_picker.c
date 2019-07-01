#include <stdbool.h>

#include "game/level/boxes.h"
#include "system/stacktrace.h"
#include "system/line_stream.h"
#include "system/log.h"
#include "game/camera.h"
#include "proto_rect.h"
#include "color_picker.h"
#include "color.h"

#define COLOR_SLIDER_HEIGHT 50.0f
#define COLOR_SLIDER_WIDTH 300.0f

LayerPtr color_picker_as_layer(ColorPicker *color_picker)
{
    LayerPtr layer = {
        .ptr = color_picker,
        .type = LAYER_COLOR_PICKER
    };
    return layer;
}

ColorPicker create_color_picker_from_rgba(Color color)
{
    Color color_hsla = rgba_to_hsla(color);
    ColorPicker color_picker = {
        .hue = {0, color_hsla.r, 360.0f},
        .saturation = {0, color_hsla.g, 1.0f},
        .lightness = {0, color_hsla.b, 1.0f}
    };
    return color_picker;
}

int color_picker_read_from_line_stream(ColorPicker *color_picker,
                                       LineStream *line_stream)
{
    char color[7];
    const char *line = line_stream_next(line_stream);
    if (line == NULL) {
        return -1;
    }

    if (sscanf(line, "%6s", color) == EOF) {
        log_fail("Could not read color\n");
    }

    *color_picker = create_color_picker_from_rgba(hexstr(color));

    return 0;
}

// TODO(#930): Color Picker doesn't have any visual indication about the current color
int color_picker_render(const ColorPicker *color_picker,
                        Camera *camera)
{
    trace_assert(color_picker);
    trace_assert(camera);

    /* TODO(#931): Color Picker sliders don't have any labels */

    if (slider_render(
            &color_picker->hue,
            camera,
            rect(0.0f, COLOR_SLIDER_HEIGHT,
                 COLOR_SLIDER_WIDTH, COLOR_SLIDER_HEIGHT)) < 0) {
        return -1;
    }

    if (slider_render(
            &color_picker->saturation,
            camera,
            rect(0.0f, COLOR_SLIDER_HEIGHT * 2.0f,
                 COLOR_SLIDER_WIDTH, COLOR_SLIDER_HEIGHT)) < 0) {
        return -1;
    }

    if (slider_render(
            &color_picker->lightness,
            camera,
            rect(0.0f, COLOR_SLIDER_HEIGHT * 3.0f,
                 COLOR_SLIDER_WIDTH, COLOR_SLIDER_HEIGHT)) < 0) {
        return -1;
    }


    return 0;
}

// TODO: the `selected` event propagation control is cumbersome
int color_picker_event(ColorPicker *color_picker, const SDL_Event *event, int *selected_out)
{
    trace_assert(color_picker);
    trace_assert(event);

    int selected = 0;

    if (slider_event(&color_picker->hue,
                     event,
                     rect(0.0f, COLOR_SLIDER_HEIGHT,
                          COLOR_SLIDER_WIDTH, COLOR_SLIDER_HEIGHT),
                     &selected) < 0) {
        return -1;
    }

    if (!selected) {
        if (slider_event(&color_picker->saturation,
                         event,
                         rect(0.0f, COLOR_SLIDER_HEIGHT * 2.0f,
                              COLOR_SLIDER_WIDTH, COLOR_SLIDER_HEIGHT),
                         &selected) < 0) {
            return -1;
        }
    }

    if (!selected) {
        if (slider_event(&color_picker->lightness,
                         event,
                         rect(0.0f, COLOR_SLIDER_HEIGHT * 3.0f,
                              COLOR_SLIDER_WIDTH, COLOR_SLIDER_HEIGHT),
                         &selected) < 0) {
            return -1;
        }
    }

    if (selected_out) {
        *selected_out = selected;
    }

    return 0;
}

Color color_picker_rgba(const ColorPicker *color_picker)
{
    return hsla(
        color_picker->hue.value,
        color_picker->saturation.value,
        color_picker->lightness.value,
        1.0f);
}
