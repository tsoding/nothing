#include <assert.h>

#include "game/level/boxes.h"
#include "game/level/player/rigid_rect.h"
#include "system/lt.h"

struct boxes_t
{
    lt_t *lt;
    size_t count;
    rigid_rect_t *bodies;
};

boxes_t *create_boxes_from_stream(FILE *stream)
{
    assert(stream);
    return NULL;
}

void destroy_boxes(boxes_t *boxes)
{
    assert(boxes);
}

int boxes_render(boxes_t *boxes, camera_t *camera)
{
    assert(boxes);
    assert(camera);
    return 0;
}

int boxes_update(boxes_t *boxes, float delta_time)
{
    assert(boxes);
    assert(delta_time);
    return 0;
}

int boxes_collide_with_platforms(boxes_t *boxes, platforms_t *platforms)
{
    assert(boxes);
    assert(platforms);
    return 0;
}
