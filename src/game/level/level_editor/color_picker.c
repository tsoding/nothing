#include <stdbool.h>
#include <string.h>

#include "game/level/boxes.h"
#include "system/stacktrace.h"
#include "system/line_stream.h"
#include "system/log.h"
#include "game/camera.h"
#include "color_picker.h"
#include "color.h"
#include "undo_history.h"

// TODO(#1021): ColorPicker doesn't have any padding between the sliders
#define COLOR_SLIDER_HEIGHT 60.0f
#define COLOR_PICKER_WIDTH 300.0f
#define COLOR_PICKER_HEIGHT (COLOR_SLIDER_HEIGHT * COLOR_SLIDER_N)
#define COLOR_PICKER_REFERENCE 1920.0f
#define COLOR_PICKER_HW_RATIO (COLOR_PICKER_HEIGHT/ COLOR_PICKER_WIDTH)
#define COLOR_PICKER_WR_RATIO (COLOR_PICKER_WIDTH / COLOR_PICKER_REFERENCE)

const char *slider_labels[COLOR_SLIDER_N] = {
    "Hue",
    "Saturation",
    "Lightness"
};

ColorPicker create_color_picker_from_rgba(Color color)
{
    Color color_hsla = rgba_to_hsla(color);
    ColorPicker color_picker = {
        .sliders = {
            {0, color_hsla.r, 360.0f},
            {0, color_hsla.g, 1.0f},
            {0, color_hsla.b, 1.0f}
        }
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

int color_picker_render(const ColorPicker *color_picker,
                        Camera *camera)
{
    trace_assert(color_picker);
    trace_assert(camera);

    const Rect viewport = camera_view_port_screen(camera);
    const Rect boundary = rect(
        0.0f, 0.0f,
        viewport.w * COLOR_PICKER_WR_RATIO,
        viewport.w * COLOR_PICKER_WR_RATIO * COLOR_PICKER_HW_RATIO);

    const float color_slider_height =
        boundary.h / (COLOR_SLIDER_N + 1.0f);

    if (camera_fill_rect_screen(
            camera,
            rect(boundary.x, boundary.y,
                 boundary.w, color_slider_height),
            color_picker_rgba(color_picker)) < 0) {
        return -1;
    }

    for (ColorPickerSlider index = 0; index < COLOR_SLIDER_N; ++index) {
        const Rect slider_rect =
            rect(boundary.x,
                 boundary.y + color_slider_height * (float) (index + 1),
                 boundary.w, color_slider_height);
        const float font_scale = boundary.w / COLOR_PICKER_WIDTH;
        const Point label_size = vec(2.5f * font_scale, 2.5f * font_scale);

        if (slider_render(
                &color_picker->sliders[index],
                camera,
                slider_rect) < 0) {
            return -1;
        }

        if (camera_render_text_screen(
                camera,
                slider_labels[index],
                label_size,
                COLOR_BLACK,
                vec(slider_rect.x + boundary.w,
                    slider_rect.y + color_slider_height * 0.5f - label_size.y * (float) FONT_CHAR_HEIGHT * 0.5f)) < 0) {
            return -1;
        }
    }

    return 0;
}

// TODO(#932): the `selected` event propagation control is cumbersome
int color_picker_event(ColorPicker *color_picker,
                       const SDL_Event *event,
                       const Camera *camera,
                       int *selected_out)
{
    trace_assert(color_picker);
    trace_assert(event);
    trace_assert(camera);

    int selected = 0;

    const Rect viewport = camera_view_port_screen(camera);
    const Rect boundary = rect(
        0.0f, 0.0f,
        viewport.w * COLOR_PICKER_WR_RATIO,
        viewport.w * COLOR_PICKER_WR_RATIO * COLOR_PICKER_HW_RATIO);

    const float color_slider_height =
        boundary.h / (COLOR_SLIDER_N + 1.0f);

    for (ColorPickerSlider index = 0;
         !selected && index < COLOR_SLIDER_N;
         ++index) {
        if (slider_event(
                &color_picker->sliders[index],
                event,
                rect(boundary.x,
                     boundary.y + color_slider_height * (float) (index + 1),
                     boundary.w, color_slider_height),
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
        color_picker->sliders[COLOR_SLIDER_HUE].value,
        color_picker->sliders[COLOR_SLIDER_SAT].value,
        color_picker->sliders[COLOR_SLIDER_LIT].value,
        1.0f);
}
