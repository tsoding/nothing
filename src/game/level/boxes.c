#include <assert.h>

#include "game/level/boxes.h"
#include "game/level/player.h"
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

    boxes->bodies = PUSH_LT(lt, malloc(sizeof(rigid_rect_t*) * boxes->count), free);
    if (boxes->bodies == NULL) {
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

    boxes->lt = lt;

    return boxes;
}

void destroy_boxes(boxes_t *boxes)
{
    assert(boxes);
    RETURN_LT0(boxes->lt);
}

solid_ref_t boxes_as_solid(boxes_t *boxes)
{
    solid_ref_t ref = {
        .tag = SOLID_BOXES,
        .ptr = (void*) boxes
    };

    return ref;
}

int boxes_render(boxes_t *boxes, camera_t *camera)
{
    assert(boxes);
    assert(camera);

    for (size_t i = 0; i < boxes->count; ++i) {
        if (rigid_rect_render(boxes->bodies[i], camera) < 0) {
            return -1;
        }
    }

    return 0;
}

int boxes_update(boxes_t *boxes,
                 float delta_time)
{
    assert(boxes);
    assert(delta_time);

    for (size_t i = 0; i < boxes->count; ++i) {
        if (rigid_rect_update(boxes->bodies[i], delta_time) < 0) {
            return -1;
        }
    }

    return 0;
}

void boxes_collide_with_solid(boxes_t *boxes,
                              solid_ref_t solid)
{
    assert(boxes);

    if (boxes == solid.ptr) {
        for (size_t i = 0; i < boxes->count; ++i) {
            for (size_t j = 0; j < boxes->count; ++j) {
                if (i != j) {
                    rigid_rect_collide_with_solid(
                        boxes->bodies[i],
                        rigid_rect_as_solid(boxes->bodies[j]));
                }
            }
        }
    } else {
        for (size_t i = 0; i < boxes->count; ++i) {
            rigid_rect_collide_with_solid(boxes->bodies[i], solid);
        }
    }
}

void boxes_collide_with_lava(boxes_t *boxes,
                             lava_t *lava)
{
    assert(boxes);

    for (size_t i = 0; i < boxes->count; ++i) {
        rigid_rect_collide_with_lava(boxes->bodies[i], lava);
    }
}

void boxes_take_impact_from_player(boxes_t *boxes,
                                   player_t *player)
{
    assert(boxes);
    assert(player);

    for (size_t i = 0; i < boxes->count; ++i) {
        player_impact_rigid_rect(player, boxes->bodies[i]);
    }
}

void boxes_touches_rect_sides(const boxes_t *boxes,
                              rect_t object,
                              int sides[RECT_SIDE_N])
{
    assert(boxes);

    for (size_t i = 0; i < boxes->count; ++i) {
        rect_object_impact(
            object,
            rigid_rect_hitbox(boxes->bodies[i]),
            sides);
    }
}
