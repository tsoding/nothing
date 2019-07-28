#ifndef WIGGLY_TEXT_H_
#define WIGGLY_TEXT_H_

#include "color.h"

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
    float alpha;
} FadingWigglyText;

int fading_wiggly_text_render(const FadingWigglyText *fading_wiggle_text,
                              Camera *camera);
int fading_wiggly_text_update(FadingWigglyText *fading_wiggle_text,
                              float delta_time);

#endif  // WIGGLY_TEXT_H_
