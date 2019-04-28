#include <SDL2/SDL.h>

#include "system/stacktrace.h"
#include "layer_picker.h"
#include "color.h"
#include "game/camera.h"

#define LAYER_CELL_WIDTH 100.0f
#define LAYER_CELL_HEIGHT 100.0f
#define LAYER_TITLE_SIZE 5.0f

static const Color LAYER_CELL_BACKGROUND_COLORS[LAYER_PICKER_N] = {
    {1.0f, 0.0f, 0.0f, 1.0f},  // LAYER_PICKER_BOXES = 0,
    {0.0f, 1.0f, 0.0f, 1.0f},  // LAYER_PICKER_PLATFORMS,
    {0.0f, 0.0f, 1.0f, 1.0f},  // LAYER_PICKER_BACK_PLATFORMS,
};

static const char *LAYER_CELL_TITLES[LAYER_PICKER_N] = {
    "B",                    // LAYER_PICKER_BOXES = 0,
    "P",                    // LAYER_PICKER_PLATFORMS,
    "BP"                    // LAYER_PICKER_BACK_PLATFORMS,
};

int layer_picker_render(const LayerPicker *layer_picker,
                        Camera *camera)
{
    trace_assert(layer_picker);
    trace_assert(camera);

    volatile const Rect viewport = camera_view_port_screen(camera);
    const Vec position = vec(0.0f, viewport.h - LAYER_CELL_HEIGHT);

    for (size_t i = 0; i < LAYER_PICKER_N; ++i) {
        if (camera_fill_rect_screen(
                camera,
                rect(LAYER_CELL_WIDTH * (float) i + position.x, position.y,
                     LAYER_CELL_WIDTH,
                     LAYER_CELL_HEIGHT),
                LAYER_CELL_BACKGROUND_COLORS[i]) < 0) {
            return -1;
        }

        if (camera_render_text_screen(
                camera,
                LAYER_CELL_TITLES[i],
                vec(LAYER_TITLE_SIZE, LAYER_TITLE_SIZE),
                color_invert(LAYER_CELL_BACKGROUND_COLORS[i]),
                vec(LAYER_CELL_WIDTH * (float) i + position.x, position.y)) < 0) {
            return -1;
        }

        // TODO: the selected layer is not indicated
    }

    return 0;
}

int layer_picker_event(LayerPicker *layer_picker,
                       const SDL_Event *event)
{
    trace_assert(layer_picker);
    trace_assert(event);

    // TODO: layer_picker_event is not implemented

    return 0;
}
