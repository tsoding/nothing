#include <SDL.h>

#include "math/point.h"

#include "./wiggly_text.h"
#include "system/lt.h"
#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "system/str.h"
#include "game/camera.h"

int wiggly_text_render(const WigglyText *wiggly_text,
                      Camera *camera)
{
    trace_assert(wiggly_text);
    trace_assert(camera);

    const size_t n = strlen(wiggly_text->text);
    char buf[2] = {0, 0};

    for (size_t i = 0; i < n; ++i) {
        buf[0] = wiggly_text->text[i];

        if (camera_render_text_screen(
                camera,
                buf,
                wiggly_text->scale,
                rgba(1.0f, 1.0f, 1.0f, 1.0f),
                vec_sum(
                    wiggly_text->position,
                    vec(
                        (float) (i * FONT_CHAR_WIDTH) * wiggly_text->scale.x,
                        sinf(wiggly_text->angle + (float) i / (float) n * 10.0f) * 20.0f))) < 0) {
            return -1;
        }
    }

    return 0;
}

int wiggly_text_update(WigglyText *wiggly_text, float delta_time)
{
    trace_assert(wiggly_text);
    wiggly_text->angle = fmodf(wiggly_text->angle + 10.0f * delta_time, 2 * PI);
    return 0;
}

Vec wiggly_text_size(const WigglyText *wiggly_text, const Camera *camera)
{
    trace_assert(wiggly_text);

    const Rect boundary = camera_text_boundary_box(
        camera,
        vec(0.0f, 0.0f),
        wiggly_text->scale,
        wiggly_text->text);

    return vec(boundary.w, boundary.h);
}

int fading_wiggly_text_render(const FadingWigglyText *fading_wiggle_text,
                              Camera *camera)
{
    trace_assert(fading_wiggle_text);
    trace_assert(camera);
    return 0;
}

int fading_wiggly_text_update(FadingWigglyText *fading_wiggle_text,
                              float detla_time)
{
    trace_assert(fading_wiggle_text);
    (void) detla_time;
    return 0;
}
