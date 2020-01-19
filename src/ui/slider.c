#include "system/stacktrace.h"
#include "math/rect.h"
#include "game/camera.h"
#include "./slider.h"

int slider_render(const Slider *slider, const Camera *camera, Rect boundary)
{
    trace_assert(slider);
    trace_assert(camera);

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

    const float ratio = slider->value / slider->max_value;
    const float cursor_width = boundary.w * 0.1f;
    const Rect cursor = rect(
        boundary.x + ratio * (boundary.w - cursor_width),
        boundary.y,
        cursor_width,
        boundary.h);
    const Color cursor_color = rgba(1.0f, 0.0f, 0.0f, 1.0f);
    if (camera_fill_rect_screen(camera, cursor, cursor_color) < 0) {
        return -1;
    }

    return 0;
}

int slider_event(Slider *slider, const SDL_Event *event, Rect boundary, int *selected)
{
    trace_assert(slider);
    trace_assert(event);

    if (!slider->drag) {
        switch (event->type) {
        case SDL_MOUSEBUTTONDOWN: {
            Vec2f position = vec((float) event->button.x, (float) event->button.y);
            if (rect_contains_point(boundary, position)) {
                slider->drag = 1;
                if (selected) {
                    *selected = 1;
                }
            }
        } break;
        }
    } else {
        switch (event->type) {
        case SDL_MOUSEBUTTONUP: {
            slider->drag = 0;
            if (selected) {
                *selected = 1;
            }
        } break;

        case SDL_MOUSEMOTION: {
            const float x = fminf(fmaxf((float) event->button.x - boundary.x, 0.0f), (float) boundary.w);
            const float ratio = x / (float) boundary.w;
            slider->value = ratio * slider->max_value;
            if (selected) {
                *selected = 1;
            }
        } break;
        }
    }

    return 0;
}
