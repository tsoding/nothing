#ifndef RIGID_BODIES_H_
#define RIGID_BODIES_H_

typedef struct RigidBodies RigidBodies;
typedef struct Camera Camera;

typedef unsigned int RigidBodyId;

RigidBodies *create_rigid_bodies(void);
void destroy_rigid_bodies(RigidBodies *rigid_bodies);

int rigid_bodies_update(RigidBodies *rigid_bodies,
                        float delta_time);

int rigid_bodies_render(RigidBodies *rigid_bodies,
                        Camera *camera);

int rigid_bodies_add(RigidBodies *rigid_bodies,
                     Rect rect,
                     Color color,
                     RigidBodyId *id);
int rigid_bodies_remove(RigidBodies *rigid_bodies,
                        RigidBodyId id);

#endif  // RIGID_BODIES_H_
