#include <assert.h>
#include <SDL2/SDL.h>

#include "./system/lt.h"
#include "./color.h"
#include "./rigid_rect.h"
#include "./system/error.h"

#define RIGID_RECT_GRAVITY 1500.0f

struct rigid_rect_t {
    lt_t *lt;
    vec_t position;
    vec_t velocity;
    vec_t movement;
    vec_t size;
    color_t color;
    int touches_ground;
};

static const vec_t opposing_rect_side_forces[RECT_SIDE_N] = {
    { .x = 1.0f,  .y =  0.0f  },  /* RECT_SIDE_LEFT = 0, */
    { .x = -1.0f, .y =  0.0f  },  /* RECT_SIDE_RIGHT, */
    { .x = 0.0f,  .y =  1.0f, },  /* RECT_SIDE_TOP, */
    { .x = 0.0f,  .y = -1.0f, }   /* RECT_SIDE_BOTTOM, */
};

static vec_t opposing_force_by_sides(int sides[RECT_SIDE_N])
{
    vec_t opposing_force = {
        .x = 0.0f,
        .y = 0.0f
    };

    for (rect_side_t side = 0; side < RECT_SIDE_N; ++side) {
        if (sides[side]) {
            vec_add(
                &opposing_force,
                opposing_rect_side_forces[side]);
        }
    }

    return opposing_force;
}

rigid_rect_t *create_rigid_rect(rect_t rect, color_t color)
{
    lt_t *lt = create_lt();

    if (lt == NULL) {
        return NULL;
    }

    rigid_rect_t *rigid_rect = PUSH_LT(lt, malloc(sizeof(rigid_rect_t)), free);
    if (rigid_rect == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    rigid_rect->lt = lt;
    rigid_rect->position = vec(rect.x, rect.y);
    rigid_rect->velocity = vec(0.0f, 0.0f);
    rigid_rect->movement = vec(0.0f, 0.0f);
    rigid_rect->size = vec(rect.w, rect.h);
    rigid_rect->color = color;
    rigid_rect->touches_ground = 0;

    return rigid_rect;
}

void destroy_rigid_rect(rigid_rect_t *rigid_rect)
{
    RETURN_LT0(rigid_rect->lt);
}

int rigid_rect_render(const rigid_rect_t *rigid_rect,
                      SDL_Renderer *renderer,
                      const camera_t *camera)
{
    return camera_fill_rect(
        camera,
        renderer,
        rigid_rect_hitbox(rigid_rect),
        rigid_rect->color);

}

int rigid_rect_update(rigid_rect_t * rigid_rect,
                      const platforms_t *platforms,
                      float delta_time)
{
    assert(rigid_rect);
    assert(platforms);

    rigid_rect->touches_ground = 0;

    rigid_rect->velocity.y += RIGID_RECT_GRAVITY * delta_time;
    rigid_rect->position = vec_sum(
        rigid_rect->position,
        vec_scala_mult(
            vec_sum(
                rigid_rect->velocity,
                rigid_rect->movement),
            delta_time));

    int sides[RECT_SIDE_N] = { 0, 0, 0, 0 };

    platforms_rect_object_collide(platforms, rigid_rect_hitbox(rigid_rect), sides);

    if (sides[RECT_SIDE_BOTTOM]) {
        rigid_rect->touches_ground = 1;
    }

    vec_t opposing_force = opposing_force_by_sides(sides);

    for (int i = 0; i < 1000 && vec_length(opposing_force) > 1e-6; ++i) {
        rigid_rect->position = vec_sum(
            rigid_rect->position,
            vec_scala_mult(
                opposing_force,
                1e-2f));

        if (fabs(opposing_force.x) > 1e-6 && (opposing_force.x < 0.0f) != ((rigid_rect->velocity.x + rigid_rect->movement.x) < 0.0f)) {
            rigid_rect->velocity.x = 0.0f;
            rigid_rect->movement.x = 0.0f;
        }

        if (fabs(opposing_force.y) > 1e-6 && (opposing_force.y < 0.0f) != ((rigid_rect->velocity.y + rigid_rect->movement.y) < 0.0f)) {
            rigid_rect->velocity.y = 0.0f;
            rigid_rect->movement.y = 0.0f;
        }

        platforms_rect_object_collide(
            platforms,
            rigid_rect_hitbox(rigid_rect),
            sides);
        opposing_force = opposing_force_by_sides(sides);
    }

    return 0;
}

rect_t rigid_rect_hitbox(const rigid_rect_t *rigid_rect)
{
    return rect_from_vecs(
        rigid_rect->position,
        rigid_rect->size);
}

void rigid_rect_move(rigid_rect_t *rigid_rect,
                           vec_t movement)
{
    rigid_rect->movement = movement;
}

void rigid_rect_jump(rigid_rect_t *rigid_rect,
                     float force)
{
    rigid_rect->velocity.y = -force;
}

int rigid_rect_touches_ground(const rigid_rect_t *rigid_rect)
{
    return rigid_rect->touches_ground;
}
