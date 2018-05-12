#include <assert.h>

#include "game/level/boxes.h"
#include "game/level/player/rigid_rect.h"
#include "system/lt.h"
#include "system/error.h"

struct boxes_t
{
    lt_t *lt;
    size_t count;
    rigid_rect_t **bodies;
};

boxes_t *create_boxes_from_stream(FILE *stream)
{
    assert(stream);

    lt_t *lt = create_lt();

    if (lt == NULL) {
        return NULL;
    }

    boxes_t *boxes = PUSH_LT(lt, malloc(sizeof(boxes_t)), free);
    if (boxes == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    if (fscanf(stream, "%lu", &boxes->count) == EOF) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    for (size_t i = 0; i < boxes->count; ++i) {
        boxes->bodies[i] = PUSH_LT(
            lt,
            create_rigid_rect_from_stream(stream),
            destroy_rigid_rect);
        if (boxes->bodies[i] == NULL) {
            RETURN_LT(lt, NULL);
        }
    }

    return boxes;
}

void destroy_boxes(boxes_t *boxes)
{
    assert(boxes);
    RETURN_LT0(boxes->lt);
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
