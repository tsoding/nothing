#include "./credits.h"
#include "game/level/background.h"
#include "game/sprite_font.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/stacktrace.h"
#include "system/str.h"
#include "system/log.h"
#include "ui/wiggly_text.h"
#include "config.h"

#define TITLE_MARGIN_TOP 100.0f

Credits create_credits(void)
{
    Credits result;
    result.background = create_background(hexstr("250741"));
    result.camera_position = vec(0.0f, 0.0f);
    result.wiggly_text = (WigglyText) {
        .text = "Twitch Subs/Contributors",
        .scale = {8.0f, 8.0f},
        .color = COLOR_WHITE,
    };
    return result;
}

int credits_render(const Credits *credits, const Camera *camera)
{
    trace_assert(credits);
    trace_assert(camera);

    const Rect viewport = camera_view_port_screen(camera);

    if (background_render(&credits->background, camera) < 0) {
        return -1;
    }

    const Vec2f title_size = wiggly_text_size(&credits->wiggly_text);

    wiggly_text_render(
        &credits->wiggly_text,
        camera,
        vec(viewport.w * 0.5f - title_size.x * 0.5f, TITLE_MARGIN_TOP));
    // TODO(#1150): Credits page don't display list of subs and contributors
    return 0;
}

int credits_update(Credits *credits, Camera *camera, float dt)
{
    trace_assert(credits);
    trace_assert(camera);

    vec_add(&credits->camera_position,
            vec(0.0f, 20.0f * dt));
    camera_center_at(camera, credits->camera_position);

    if (wiggly_text_update(&credits->wiggly_text, dt) < 0) {
        return -1;
    }

    return 0;
}
