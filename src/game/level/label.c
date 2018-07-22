#include <assert.h>

#include "game/level/label.h"

label_t *create_label(vec_t position,
                      color_t color,
                      const char *text)
{
    assert(text);
    (void) position;
    (void) color;
    return NULL;
}

void destroy_label(label_t *label)
{
    assert(label);
}

int label_render(const label_t *label,
                 camera_t *camera)
{
    assert(label);
    assert(camera);

    return 0;
}

void label_update(label_t *label,
                  float delta_time)
{
    assert(label);
    (void) delta_time;
}
