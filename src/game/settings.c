#include "system/stacktrace.h"
#include "settings.h"

void settings_render(const Settings *settings, const Camera *camera)
{
    trace_assert(settings);
    trace_assert(camera);

    const Rect viewport = camera_view_port_screen(camera);

    /* CSS volume */
    const Rect position = {
        .w = viewport.w * settings->volume_slider_scale.x,
        .h = viewport.h * settings->volume_slider_scale.y,
        .x = viewport.w - viewport.w * settings->volume_slider_scale.x - 5.0f,
        .y = 5.0f,
    };

    /* HTML volume */
    slider_render(&settings->volume_slider, camera, position);
}

void settings_event(Settings *settings, Camera *camera, const SDL_Event *event)
{
    trace_assert(settings);
    trace_assert(event);

    const Rect viewport = camera_view_port_screen(camera);
    const Rect position = {
        .w = viewport.w * settings->volume_slider_scale.x,
        .h = viewport.h * settings->volume_slider_scale.y,
        .x = viewport.w - viewport.w * settings->volume_slider_scale.x - 5.0f,
        .y = 5.0f,
    };

    if (slider_event(
            &settings->volume_slider,
            event,
            position, NULL) < 0) {
        return;
    }
}

void settings_update(Settings *settings, float dt)
{
    trace_assert(settings);
    (void) dt;
}
