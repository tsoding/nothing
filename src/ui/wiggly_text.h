#ifndef WIGGLY_TEXT_H_
#define WIGGLY_TEXT_H_

typedef struct Camera Camera;

typedef struct
{
    const char *text;
    Vec font_scale;
    float angle;
    Vec position;
} WigglyText;

int wiggly_text_render(const WigglyText *wiggly_text,
                      Camera *camera);
int wiggly_text_update(WigglyText *wiggly_text, float delta_time);
Vec wiggly_text_size(const WigglyText *wiggly_text, const Camera *camera);

#endif  // WIGGLY_TEXT_H_
