#include <stdlib.h>

#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/stacktrace.h"

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
}
