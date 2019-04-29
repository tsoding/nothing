#include <stdbool.h>

#include "game/level/boxes.h"
#include "system/stacktrace.h"
#include "game/camera.h"
#include "proto_rect.h"
#include "color_picker.h"
#include "color.h"

#define COLOR_CELL_WIDTH 50.0f
#define COLOR_CELL_HEIGHT 50.0f

// TODO(#788): Colors of ColorPicker are poor
static Color colors[] = {
    {1.0f, 0.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 1.0f}
};
static const size_t colors_count = sizeof(colors) / sizeof(Color);

int color_picker_render(const ColorPicker *color_picker,
                        Camera *camera)
{
    trace_assert(color_picker);
    trace_assert(camera);

    for (size_t i = 0; i < colors_count; ++i) {
        if (camera_fill_rect_screen(
                camera,
                rect(COLOR_CELL_WIDTH * (float) i, 0,
                     COLOR_CELL_WIDTH,
                     COLOR_CELL_HEIGHT),
                colors[i]) < 0) {
            return -1;
        }
    }

    return 0;
}

int color_picker_mouse_button(ColorPicker *color_picker,
                              const SDL_MouseButtonEvent *event,
                              bool *selected)
{
    trace_assert(color_picker);
    trace_assert(event);

    if (event->type == SDL_MOUSEBUTTONDOWN) {
        switch (event->button) {
        case SDL_BUTTON_LEFT: {
            for (size_t i = 0; i < colors_count; ++i) {
                const Vec mouse_position = vec((float) event->x, (float) event->y);
                const Rect color_cell =
                    rect(COLOR_CELL_WIDTH * (float) i, 0,
                         COLOR_CELL_WIDTH,
                         COLOR_CELL_HEIGHT);
                if (rect_contains_point(color_cell, mouse_position)) {
                    color_picker->color = colors[i];
                    *selected = true;
                }
            }
        } break;
        }
    }

    return 0;
}
