#include <stdbool.h>

#include "game/level/boxes.h"
#include "system/stacktrace.h"
#include "system/line_stream.h"
#include "system/log.h"
#include "game/camera.h"
#include "color_picker.h"
#include "color.h"
#include "undo_history.h"

#define COLOR_SLIDER_HEIGHT 50.0f
#define COLOR_SLIDER_WIDTH 300.0f
#define COLOR_SLIDER_PADDING_BOTTOM 10.0f

const char *slider_labels[COLOR_SLIDER_N] = {
    "Hue",
    "Saturation",
    "Lightness"
};

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

    if (camera_fill_rect_screen(
            camera,
            rect(0.0f, 0.0f, COLOR_SLIDER_WIDTH, COLOR_SLIDER_HEIGHT),
            color_picker_rgba(color_picker)) < 0) {
        return -1;
    }

    for (ColorPickerSlider index = 0; index < COLOR_SLIDER_N; ++index) {
        const Rect slider_rect =
            rect(0.0f, (COLOR_SLIDER_HEIGHT + COLOR_SLIDER_PADDING_BOTTOM) * (float) (index + 1),
                 COLOR_SLIDER_WIDTH, COLOR_SLIDER_HEIGHT);
        const Point label_size = vec(2.5f, 2.5f);

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
                vec(slider_rect.x + COLOR_SLIDER_WIDTH,
                    slider_rect.y + COLOR_SLIDER_HEIGHT * 0.5f - label_size.y * (float) FONT_CHAR_HEIGHT * 0.5f)) < 0) {
            return -1;
        }
    }

    return 0;
}

static void color_picker_reset_color(void *layer, Context context)
{
    trace_assert(layer);
    ColorPicker *color_picker = layer;
    *color_picker = create_color_picker_from_rgba(*((Color*)context.data));
}

// TODO(#932): the `selected` event propagation control is cumbersome
int color_picker_event(ColorPicker *color_picker,
                       const SDL_Event *event,
                       int *selected_out,
                       UndoHistory *undo_history)
{
    trace_assert(color_picker);
    trace_assert(event);
    trace_assert(undo_history);

    int selected = 0;

    for (ColorPickerSlider index = 0;
         !selected && index < COLOR_SLIDER_N;
         ++index) {
        if (slider_event(
                &color_picker->sliders[index],
                event,
                rect(0.0f, (COLOR_SLIDER_HEIGHT + COLOR_SLIDER_PADDING_BOTTOM) * (float) (index + 1),
                     COLOR_SLIDER_WIDTH, COLOR_SLIDER_HEIGHT),
                &selected) < 0) {
            return -1;
        }
    }

    // TODO: each and individual slider movement is pushed into the undo history
    if (selected) {
        trace_assert(sizeof(Color) <= CONTEXT_SIZE);

        Action action = {
            .layer = color_picker,
            .revert = color_picker_reset_color
        };
        const Color color = color_picker_rgba(color_picker);
        memcpy(action.context.data, &color, sizeof(Color));
        undo_history_push(undo_history, action);
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
