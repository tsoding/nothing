#include <SDL.h>

#include "math/vec.h"

#include "./wiggly_text.h"
#include "system/lt.h"
#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "system/str.h"
#include "game/camera.h"

void wiggly_text_render(const WigglyText *wiggly_text,
                        const Camera *camera,
                        Vec2f position)
{
    trace_assert(wiggly_text);
    trace_assert(camera);

    const size_t n = strlen(wiggly_text->text);
    char buf[2] = {0, 0};

    for (size_t i = 0; i < n; ++i) {
        buf[0] = wiggly_text->text[i];

        camera_render_text_screen(
            camera,
            buf,
            wiggly_text->scale,
            wiggly_text->color,
            vec_sum(
                position,
                vec(
                    (float) (i * FONT_CHAR_WIDTH) * wiggly_text->scale.x,
                    sinf(wiggly_text->angle + (float) i / (float) n * 10.0f) * 20.0f)));
    }
}

int wiggly_text_update(WigglyText *wiggly_text, float delta_time)
{
    trace_assert(wiggly_text);
    wiggly_text->angle = fmodf(wiggly_text->angle + 10.0f * delta_time, 2 * PI);
    return 0;
}

Vec2f wiggly_text_size(const WigglyText *wiggly_text)
{
    trace_assert(wiggly_text);

    const Rect boundary = sprite_font_boundary_box(
        vec(0.0f, 0.0f),
        wiggly_text->scale,
        wiggly_text->text);

    return vec(boundary.w, boundary.h);
}
