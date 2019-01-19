#ifndef RIGID_BODIES_H_
#define RIGID_BODIES_H_

typedef struct RigidBodies RigidBodies;
typedef struct Camera Camera;
typedef struct Platforms Platforms;

typedef size_t RigidBodyId;

RigidBodies *create_rigid_bodies(size_t capacity);
void destroy_rigid_bodies(RigidBodies *rigid_bodies);

int rigid_bodies_collide_with_itself(RigidBodies *rigid_bodies);
int rigid_bodies_collide_with_platforms(RigidBodies *rigid_bodies,
                                        const Platforms *platforms);

int rigid_bodies_update(RigidBodies *rigid_bodies,
                        float delta_time);

int rigid_bodies_render(RigidBodies *rigid_bodies,
                        Camera *camera);
RigidBodyId rigid_bodies_add(RigidBodies *rigid_bodies,
                             Rect rect,
                             Color color);

Rect rigid_bodies_hitbox(const RigidBodies *rigid_bodies,
                         RigidBodyId id);

void rigid_bodies_move(RigidBodies *rigid_bodies,
                       RigidBodyId id,
                       Vec movement);

int rigid_bodies_touches_ground(const RigidBodies *rigid_bodies,
                                RigidBodyId id);

void rigid_bodies_apply_force(RigidBodies * rigid_bodies,
                              RigidBodyId id,
                              Vec force);

void rigid_bodies_apply_omniforce(RigidBodies *rigid_bodies,
                                  Vec force);

void rigid_bodies_transform_velocity(RigidBodies *rigid_bodies,
                                     RigidBodyId id,
                                     mat3x3 trans_mat);

void rigid_bodies_teleport_to(RigidBodies *rigid_bodies,
                              RigidBodyId id,
                              Vec position);

void rigid_bodies_damper(RigidBodies *rigid_bodies,
                         RigidBodyId id,
                         Vec v);


#endif  // RIGID_BODIES_H_
