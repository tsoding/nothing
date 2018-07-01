#include <assert.h>

#include "./rigid_bodies.h"

rigid_bodies_t *create_rigid_bodies(void)
{
    return NULL;
}

void destroy_rigid_bodies(rigid_bodies_t *rigid_bodies)
{
    assert(rigid_bodies);
}

void rigid_bodies_update(rigid_bodies_t *rigid_bodies,
                         float delta_time)
{
    assert(rigid_bodies);
    (void) delta_time;
}

rigid_rect_ref rigid_bodies_create_rect(rigid_bodies_t *rigid_bodies,
                                        rect_t rect)
{
    assert(rigid_bodies);
    (void) rect;

    return 0;
}

void rigid_bodies_destroy_rect(rigid_bodies_t *rigid_bodies,
                               rigid_rect_ref rect_ref)
{
    assert(rigid_bodies);
    (void) rect_ref;
}
