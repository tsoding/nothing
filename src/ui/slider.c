#include "system/stacktrace.h"
#include "math/rect.h"
#include "game/camera.h"
#include "./slider.h"

int slider_render(const Slider *slider, Camera *camera, Rect boundary)
{
    trace_assert(slider);
    trace_assert(camera);

    if (camera_draw_rect_screen(camera, boundary, COLOR_RED) < 0) {
        return -1;
    }

    const Color core_color = rgba(0.0f, 0.0f, 0.0f, 1.0f);

    const float core_height = boundary.h * 0.33f;
    const Rect core = rect(
        boundary.x,
        boundary.y + boundary.h * 0.5f - core_height * 0.5f,
        boundary.w,
        core_height);

    if (camera_fill_rect_screen(camera, core, core_color) < 0) {
        return -1;
    }

    const Rect cursor = rect(
        boundary.x + boundary.w * 0.5f, boundary.y,
        boundary.w * 0.05f, boundary.h);
    const Color cursor_color = rgba(0.60f, 0.60f, 0.60f, 1.0f);

    if (camera_fill_rect_screen(camera, cursor, cursor_color) < 0) {
        return -1;
    }

    return 0;
}

int slider_event(Slider *slider, const SDL_Event *event, Rect boundary)
{
    trace_assert(slider);
    trace_assert(event);
    (void) boundary;
    return 0;
}
