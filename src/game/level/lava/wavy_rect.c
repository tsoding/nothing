#include <SDL.h>
#include "system/stacktrace.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "math/pi.h"
#include "system/log.h"
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

    Wavy_rect *wavy_rect = PUSH_LT(lt, nth_calloc(1, sizeof(Wavy_rect)), free);
    if (wavy_rect == NULL) {
        RETURN_LT(lt, NULL);
    }

    wavy_rect->rect = rect;
    wavy_rect->color = color;
    wavy_rect->angle = 0.0f;
    wavy_rect->lt = lt;

    return wavy_rect;
}

void destroy_wavy_rect(Wavy_rect *wavy_rect)
{
    trace_assert(wavy_rect);
    RETURN_LT0(wavy_rect->lt);
}

int wavy_rect_render(const Wavy_rect *wavy_rect,
                     const Camera *camera)
{
    trace_assert(wavy_rect);
    trace_assert(camera);

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
    trace_assert(wavy_rect);
    wavy_rect->angle = fmodf(wavy_rect->angle + 2.0f * delta_time, 2 * PI);

    return 0;
}

Rect wavy_rect_hitbox(const Wavy_rect *wavy_rect)
{
    return wavy_rect->rect;
}
