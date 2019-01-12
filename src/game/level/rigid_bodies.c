#include <stdlib.h>

#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/stacktrace.h"
#include "game/camera.h"

#include "./rigid_bodies.h"

struct RigidBodies
{
    Lt *lt;
};

RigidBodies *create_rigid_bodies(void)
{
    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    RigidBodies *rigid_bodies = PUSH_LT(
        lt,
        nth_alloc(sizeof(RigidBodies)),
        free);
    if (rigid_bodies == NULL) {
        RETURN_LT(lt, NULL);
    }
    rigid_bodies->lt = lt;

    return rigid_bodies;
}

void destroy_rigid_bodies(RigidBodies *rigid_bodies)
{
    trace_assert(rigid_bodies);
    RETURN_LT0(rigid_bodies->lt);
}

int rigid_bodies_update(RigidBodies *rigid_bodies,
                        float delta_time)
{
    trace_assert(rigid_bodies);
    (void) delta_time;
    return 0;
}

int rigid_bodies_render(RigidBodies *rigid_bodies,
                        Camera *camera)
{
    trace_assert(rigid_bodies);
    trace_assert(camera);
    return 0;
}

int rigid_bodies_add(RigidBodies *rigid_bodies,
                     Rect rect,
                     Color color,
                     unsigned int *id)
{
    trace_assert(rigid_bodies);
    trace_assert(id);
    (void) rect;
    (void) color;
    return 0;
}

int rigid_bodies_remove(RigidBodies *rigid_bodies,
                        unsigned int id)
{
    trace_assert(rigid_bodies);
    (void) id;
    return 0;
}
