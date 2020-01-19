#ifndef WIGGLY_TEXT_H_
#define WIGGLY_TEXT_H_

#include <math.h>

#include "color.h"
#include "system/stacktrace.h"
#include "game/camera.h"

typedef struct {
    const char *text;
    Vec2f scale;
    Color color;
    float angle;
} WigglyText;

void wiggly_text_render(const WigglyText *wiggly_text,
                        const Camera *camera,
                        Vec2f position);
int wiggly_text_update(WigglyText *wiggly_text, float delta_time);
Vec2f wiggly_text_size(const WigglyText *wiggly_text);

typedef struct {
    WigglyText wiggly_text;
    float duration;
} FadingWigglyText;

static inline
void fading_wiggly_text_render(const FadingWigglyText *fading_wiggly_text,
                               const Camera *camera,
                               Vec2f position)
{
    wiggly_text_render(
        &fading_wiggly_text->wiggly_text,
        camera,
        position);
}

static inline
int fading_wiggly_text_update(FadingWigglyText *fading_wiggly_text,
                              float delta_time)
{
    trace_assert(fading_wiggly_text);

    const float alpha = fading_wiggly_text->wiggly_text.color.a;
    const float duration = fading_wiggly_text->duration;

    fading_wiggly_text->wiggly_text.color.a =
        fmaxf(alpha * duration - delta_time, 0.0f) / duration;

    return wiggly_text_update(&fading_wiggly_text->wiggly_text, delta_time);
}

static inline
void fading_wiggly_text_reset(FadingWigglyText *fading_wiggly_text)
{
    trace_assert(fading_wiggly_text);
    fading_wiggly_text->wiggly_text.color.a = 1.0f;
}

static inline
Vec2f fading_wiggly_text_size(const FadingWigglyText *fading_wiggly_text)
{
    return wiggly_text_size(&fading_wiggly_text->wiggly_text);
}

#endif  // WIGGLY_TEXT_H_
