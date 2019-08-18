#include <stdbool.h>
#include <string.h>

#include <SDL.h>

#include "system/stacktrace.h"
#include "layer_picker.h"
#include "color.h"
#include "game/camera.h"
#include "math/rect.h"
#include "math/extrema.h"
#include "game/sprite_font.h"

#define LAYER_TITLE_PADDING 15.0f
#define LAYER_TITLE_SIZE 3.0f
#define LAYER_SELECTED_OFFSET 15.0f

static const Color LAYER_CELL_BACKGROUND_COLORS[LAYER_PICKER_N] = {
    {1.0f, 0.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    {1.0f, 0.2f, 0.6f, 1.0f},
    {0.6f, 0.6f, 1.0f, 1.0f},
    {0.2f, 1.0f, 0.6f, 1.0f},
    {0.2f, 0.6f, 1.0f, 1.0f},
    {0.2f, 1.0f, 0.6f, 1.0f},
};

static const char *LAYER_CELL_TITLES[LAYER_PICKER_N] = {
    "Background",              // LAYER_PICKER_BACKGROUND
    "Player",                  // LAYER_PICKER_PLAYER
    "Platforms",               // LAYER_PICKER_PLATFORMS,
    "Goals",                   // LAYER_PICKER_GOALS
    "Lava",                    // LAYER_PICKER_LAVA
    "Back Platforms",          // LAYER_PICKER_BACK_PLATFORMS,
    "Boxes",                   // LAYER_PICKER_BOXES = 0,
    "Labels",                  // LAYER_PICKER_LABELS
    "Regions",                 // LAYER_PICKER_REGIONS
};

inline static float layer_picker_max_width(void)
{
    size_t max = 0;

    for (size_t i = 0; i < LAYER_PICKER_N; ++i) {
        max = max_size_t(max, strlen(LAYER_CELL_TITLES[i]));
    }

    return (float) max * FONT_CHAR_WIDTH * LAYER_TITLE_SIZE + LAYER_TITLE_PADDING * 2.0f;
}

#define LAYER_CELL_WIDTH layer_picker_max_width()
#define LAYER_CELL_HEIGHT (LAYER_TITLE_SIZE * FONT_CHAR_HEIGHT + LAYER_TITLE_PADDING * 2.0f)
#define LAYER_CELL_REFERENCE 1980.0f
#define LAYER_CELL_WR_RATIO (LAYER_CELL_WIDTH / LAYER_CELL_REFERENCE)
#define LAYER_CELL_HW_RATIO (LAYER_CELL_HEIGHT / LAYER_CELL_WIDTH)
#define LAYER_TITLE_SW_RATIO (LAYER_TITLE_SIZE / LAYER_CELL_WIDTH)
#define LAYER_TITLE_PW_RATIO (LAYER_TITLE_PADDING / LAYER_CELL_WIDTH)
#define LAYER_CELL_OW_RATIO (LAYER_SELECTED_OFFSET / LAYER_CELL_WIDTH)

int layer_picker_render(const LayerPicker *layer_picker,
                        Camera *camera)
{
    trace_assert(layer_picker);
    trace_assert(camera);

    const Rect viewport = camera_view_port_screen(camera);

    for (size_t i = 0; i < LAYER_PICKER_N; ++i) {
        const Vec size = {
            .x = viewport.w * LAYER_CELL_WR_RATIO,
            .y = viewport.w * LAYER_CELL_WR_RATIO * LAYER_CELL_HW_RATIO
        };

        Vec position = {
            .x = 0.0f,
            .y = viewport.h * 0.5f - size.y * LAYER_PICKER_N * 0.5f
        };

        Color color = LAYER_CELL_BACKGROUND_COLORS[i];

        if (*layer_picker == i) {
            position.x += size.x * LAYER_CELL_OW_RATIO;
        } else {
            color.a *= 0.70f;
        }

        if (camera_fill_rect_screen(
                camera,
                rect(
                    position.x,
                    size.y * (float) i + position.y,
                    size.x, size.y),
                color) < 0) {
            return -1;
        }

        if (camera_render_text_screen(
                camera,
                LAYER_CELL_TITLES[i],
                vec(size.x * LAYER_TITLE_SW_RATIO,
                    size.x * LAYER_TITLE_SW_RATIO),
                color_invert(color),
                vec(position.x + size.x * LAYER_TITLE_PW_RATIO,
                    size.y * (float) i + position.y + size.x * LAYER_TITLE_PW_RATIO)) < 0) {
            return -1;
        }
    }

    return 0;
}

int layer_picker_event(LayerPicker *layer_picker,
                       const SDL_Event *event,
                       const Camera *camera,
                       bool *selected)
{
    trace_assert(layer_picker);
    trace_assert(event);
    trace_assert(camera);

    const Rect viewport = camera_view_port_screen(camera);

    const Vec size = {
        .x = viewport.w * LAYER_CELL_WR_RATIO,
        .y = viewport.w * LAYER_CELL_WR_RATIO * LAYER_CELL_HW_RATIO
    };

    const Vec position = {
        .x = 0.0f,
        .y = viewport.h * 0.5f - size.y * LAYER_PICKER_N * 0.5f
    };

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN: {
        for (size_t i = 0; i < LAYER_PICKER_N; ++i) {
            const Rect cell = rect(
                position.x,
                size.y * (float) i + position.y,
                size.x, size.y);
            if (rect_contains_point(cell, vec((float) event->button.x, (float) event->button.y))) {
                *layer_picker = i;
                *selected = true;
                return 0;
            }
        }
    } break;
    }

    return 0;
}
