#include "system/stacktrace.h"
#include "settings.h"
#include "config.h"

Settings create_settings(void)
{
    Settings settings = {
        .volume_slider = {
            .drag = 0,
            .value = SOUND_SAMPLES_DEFAULT_VOLUME,
            .max_value = 100.0f,
        },

        .volume_slider_scale = {
            0.25f, 0.10f
        },

        .background = {
            .base_color = {0.5f, 0.8f, 0.5f, 1.0f}
        },

        .camera_position = {
            0.0f, 0.0f
        }
    };

    return settings;
}

void settings_render(const Settings *settings, const Camera *camera)
{
    trace_assert(settings);
    trace_assert(camera);

    background_render(&settings->background, camera);

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

void settings_update(Settings *settings, Camera *camera, float dt)
{
    trace_assert(settings);
    trace_assert(camera);

    vec_add(&settings->camera_position,
            vec(50.0f * dt, 0.0f));
    camera_center_at(camera, settings->camera_position);
}
