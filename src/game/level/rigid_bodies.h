#ifndef RIGID_BODIES_H_
#define RIGID_BODIES_H_

typedef struct RigidBodies RigidBodies;

RigidBodies *create_rigid_bodies(void);
void destroy_rigid_bodies(RigidBodies *rigid_bodies);

#endif  // RIGID_BODIES_H_
