#ifndef WIGGLY_TEXT_H_
#define WIGGLY_TEXT_H_

#include "color.h"
#include "system/stacktrace.h"

typedef struct Camera Camera;

typedef struct {
    const char *text;
    Vec scale;
    Color color;
    Vec position;
    float angle;
} WigglyText;

int wiggly_text_render(const WigglyText *wiggly_text,
                       Camera *camera);
int wiggly_text_update(WigglyText *wiggly_text, float delta_time);
Vec wiggly_text_size(const WigglyText *wiggly_text, const Camera *camera);

typedef struct {
    WigglyText wiggly_text;
    float duration;
} FadingWigglyText;

static inline
int fading_wiggly_text_render(const FadingWigglyText *fading_wiggly_text,
                              Camera *camera)
{
    return wiggly_text_render(&fading_wiggly_text->wiggly_text, camera);
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


#endif  // WIGGLY_TEXT_H_
