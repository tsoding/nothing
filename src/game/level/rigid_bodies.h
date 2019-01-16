#ifndef RIGID_BODIES_H_
#define RIGID_BODIES_H_

typedef struct RigidBodies RigidBodies;
typedef struct Camera Camera;

typedef size_t RigidBodyId;

RigidBodies *create_rigid_bodies(size_t capacity);
void destroy_rigid_bodies(RigidBodies *rigid_bodies);

int rigid_bodies_update(RigidBodies *rigid_bodies,
                        float delta_time);

int rigid_bodies_render(RigidBodies *rigid_bodies,
                        Camera *camera);
RigidBodyId rigid_bodies_add(RigidBodies *rigid_bodies,
                             Rect rect,
                             Color color);

#endif  // RIGID_BODIES_H_
