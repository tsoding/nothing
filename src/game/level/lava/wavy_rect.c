#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "math/pi.h"
#include "system/error.h"
#include "system/line_stream.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "wavy_rect.h"

#define WAVE_PILLAR_WIDTH 10.0f

struct Wavy_rect
{
    Lt *lt;

    Rect rect;
    Color color;
    float angle;
};

Wavy_rect *create_wavy_rect(Rect rect, Color color)
{
    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Wavy_rect *wavy_rect = PUSH_LT(lt, nth_alloc(sizeof(Wavy_rect)), free);
    if (wavy_rect == NULL) {
        throw_error(ERROR_TYPE_SDL2);
        RETURN_LT(lt, NULL);
    }

    wavy_rect->rect = rect;
    wavy_rect->color = color;
    wavy_rect->angle = 0.0f;
    wavy_rect->lt = lt;

    return wavy_rect;
}

Wavy_rect *create_wavy_rect_from_line_stream(LineStream *line_stream)
{
    assert(line_stream);
    char color_name[7];
    Rect rect;

    if (sscanf(
            line_stream_next(line_stream),
            "%f%f%f%f%6s\n",
            &rect.x, &rect.y,
            &rect.w, &rect.h,
            color_name) < 0) {
        throw_error(ERROR_TYPE_LIBC);
        return NULL;
    }

    Color color = color_from_hexstr(color_name);

    return create_wavy_rect(rect, color);
}

void destroy_wavy_rect(Wavy_rect *wavy_rect)
{
    assert(wavy_rect);
    RETURN_LT0(wavy_rect->lt);
}

int wavy_rect_render(const Wavy_rect *wavy_rect,
                     Camera *camera)
{
    assert(wavy_rect);
    assert(camera);

    srand(42);
    for (float wave_scanner = 0;
         wave_scanner < wavy_rect->rect.w;
         wave_scanner += WAVE_PILLAR_WIDTH) {

        const float s = (float) (rand() % 50) * 0.1f;
        if (camera_fill_rect(
                camera,
                rect(
                    wavy_rect->rect.x + wave_scanner,
                    wavy_rect->rect.y + s * sinf(wavy_rect->angle + wave_scanner / WAVE_PILLAR_WIDTH),
                    WAVE_PILLAR_WIDTH * 1.20f,
                    wavy_rect->rect.h),
                wavy_rect->color) < 0) {
            return -1;
        }
    }
    srand((unsigned int) time(NULL));

    return 0;
}

int wavy_rect_update(Wavy_rect *wavy_rect,
                     float delta_time)
{
    assert(wavy_rect);
    wavy_rect->angle = fmodf(wavy_rect->angle + 2.0f * delta_time, 2 * PI);

    return 0;
}

Rect wavy_rect_hitbox(const Wavy_rect *wavy_rect)
{
    return wavy_rect->rect;
}
